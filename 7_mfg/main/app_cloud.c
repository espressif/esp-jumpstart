/* Cloud task for Manufacturing example
   This example code is in the Public Domain (or CC0 licensed, at your option.)
*/
#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <mqtt_client.h>
#include <json_parser.h>
#include <esp_crt_bundle.h>
#include "app_priv.h"
#include <esp_app_desc.h>

static const char *TAG = "cloud";

extern const uint8_t endpoint_txt_start[] asm("_binary_endpoint_txt_start");
extern const uint8_t endpoint_txt_end[] asm("_binary_endpoint_txt_end");

#define SHADOW_DELTA_TOPIC_FMT "$aws/things/%s/shadow/update/delta"
#define SHADOW_UPDATE_TOPIC_FMT "$aws/things/%s/shadow/update"
#define MFG_PARTITION_NAME "fctry"

static esp_mqtt_client_handle_t mqtt_client = NULL;
static char thing_name[128];

// These will be loaded from NVS
static char *serial_no = NULL;
static char *cert = NULL;
static char *priv_key = NULL;

void publish_reported_state(bool output)
{
    if (!mqtt_client) {
        return;
    }
    char topic[256];
    char payload[256];
    snprintf(topic, sizeof(topic), SHADOW_UPDATE_TOPIC_FMT, thing_name);
    snprintf(payload, sizeof(payload),
        "{\"state\":{\"reported\":{\"output\":%s},\"desired\":{\"output\":%s}}}",
        output ? "true" : "false", output ? "true" : "false");
    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "Published reported+desired state: %s", payload);
}

void publish_boot_shadow_state(bool output) {
    if (!mqtt_client) return;
    const esp_app_desc_t *app_desc = esp_app_get_description();
    char topic[256];
    char payload[512];
    snprintf(topic, sizeof(topic), SHADOW_UPDATE_TOPIC_FMT, thing_name);
    snprintf(payload, sizeof(payload),
        "{\"state\":{\"reported\":{\"output\":%s,\"ota_url\":null,\"fw_version\":\"%s\"},\"desired\":{\"output\":%s,\"ota_url\":null}}}",
        output ? "true" : "false", app_desc->version, output ? "true" : "false");
    esp_mqtt_client_publish(mqtt_client, topic, payload, 0, 1, 0);
    ESP_LOGI(TAG, "Published boot shadow state: %s", payload);
}

static void handle_delta(const char *data, int len)
{
    jparse_ctx_t jctx;
    bool new_state = false;
    char ota_url[256] = "";
    int ret = json_parse_start(&jctx, data, len);
    if (ret == OS_SUCCESS) {
        if (json_obj_get_object(&jctx, "state") == OS_SUCCESS) {
            // output
            if (json_obj_get_bool(&jctx, "output", &new_state) == OS_SUCCESS) {
                app_driver_set_state(new_state);
                ESP_LOGI(TAG, "Delta: Set output to %s", new_state ? "true" : "false");
            }
            // ota_url
            json_obj_get_string(&jctx, "ota_url", ota_url, sizeof(ota_url));
            if (strlen(ota_url) > 0) {
                ESP_LOGI(TAG, "Delta: OTA URL: %s", ota_url);
                if (do_firmware_upgrade(ota_url) == ESP_OK) {
                    ESP_LOGI(TAG, "OTA success, restarting...");
                    esp_restart();
                }
            }
            json_obj_leave_object(&jctx);
        } else {
            ESP_LOGW(TAG, "Delta JSON does not contain 'state' object");
        }
        json_parse_end(&jctx);
    } else {
        ESP_LOGW(TAG, "Failed to parse delta JSON");
    }
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event)
{
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED: {
            ESP_LOGI(TAG, "MQTT connected");
            char topic[256];
            snprintf(topic, sizeof(topic), SHADOW_DELTA_TOPIC_FMT, thing_name);
            esp_mqtt_client_subscribe(mqtt_client, topic, 1);
            publish_boot_shadow_state(app_driver_get_state());
            break;
        }
        case MQTT_EVENT_DATA: {
            ESP_LOGI(TAG, "MQTT_EVENT_DATA: topic=%.*s", event->topic_len, event->topic);
            char topic[256];
            snprintf(topic, sizeof(topic), SHADOW_DELTA_TOPIC_FMT, thing_name);
            if (event->topic_len == strlen(topic) && strncmp(event->topic, topic, event->topic_len) == 0) {
                handle_delta(event->data, event->data_len);
            }
            break;
        }
        default:
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    mqtt_event_handler_cb((esp_mqtt_event_handle_t)event_data);
}

static int alloc_and_read_from_nvs(nvs_handle handle, const char *key, char **value)
{
    size_t required_size = 0;
    int error;
    if ((error = nvs_get_blob(handle, key, NULL, &required_size)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read key %s with error %d size %d\n", key, error, required_size);
        return -1;
    }
    *value = calloc(1, required_size + 1);  /* The extra byte is for the NULL termination */
    if (*value) {
        nvs_get_blob(handle, key, *value, &required_size);
        ESP_LOGI(TAG, "Read key:%s, value:%s\n", key, *value);
        return 0;
    }
    return -1;
}

static const char *alpn_protos[] = { "x-amzn-mqtt-ca", NULL };

int cloud_start(void)
{
    printf("Starting cloud\n");
    nvs_handle fctry_handle;
    if (nvs_flash_init_partition(MFG_PARTITION_NAME) != ESP_OK) {
        ESP_LOGE(TAG, "NVS Flash init failed");
        return -1;
    }

    if (nvs_open_from_partition(MFG_PARTITION_NAME, "mfg_ns",
                                NVS_READONLY, &fctry_handle) != ESP_OK) {
        ESP_LOGE(TAG, "NVS open failed. Check if the %s partition exists", MFG_PARTITION_NAME);
        return -1;
    }
    if (alloc_and_read_from_nvs(fctry_handle, "serial_no", &serial_no) != 0) {
        return -1;
    }
    if (alloc_and_read_from_nvs(fctry_handle, "cert", &cert) != 0) {
        return -1;
    }
    if (alloc_and_read_from_nvs(fctry_handle, "priv_key", &priv_key) != 0) {
        return -1;
    }
    nvs_close(fctry_handle);

    strncpy(thing_name, serial_no, sizeof(thing_name)-1);
    thing_name[sizeof(thing_name)-1] = '\0';

    /* Get endpoint from endpoint.txt */
    size_t endpoint_len = endpoint_txt_end - endpoint_txt_start;
    char endpoint[256];
    if (endpoint_len >= sizeof(endpoint)) endpoint_len = sizeof(endpoint) - 1;
    memcpy(endpoint, endpoint_txt_start, endpoint_len);
    endpoint[endpoint_len] = '\0';

    const esp_mqtt_client_config_t mqtt_cfg = {
        .broker = {
            .address = {
                .hostname = endpoint,
                .port = 443,
                .transport = MQTT_TRANSPORT_OVER_SSL,
            },
            .verification = {
                .alpn_protos = alpn_protos,
                .crt_bundle_attach = esp_crt_bundle_attach,
            },
        },
        .credentials = {
            .client_id = thing_name,
            .authentication = {
                .certificate = (const char *)cert,
                .key = (const char *)priv_key,
            },
        },
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!mqtt_client) {
        ESP_LOGE(TAG, "esp_mqtt_client_init failed");
        return ESP_FAIL;
    }
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    return 0;
}
