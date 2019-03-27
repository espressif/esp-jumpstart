/* Cloud task

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include "esp_system.h"

#include "aws_custom_utils.h"

#include "aws_iot_log.h"
#include "aws_iot_config.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "app_priv.h"

static const char *TAG = "cloud";

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 300
#define MAX_DESIRED_PARAM 2
#define MAX_REPORTED_PARAM 3
#define MAX_LENGTH_URL 256

/*
 * The Json Document in the cloud will be:
 * {
 *   "reported": {
 *      "ota_url": "",
 *      "output": true,
 *      "fw_version": "<version-string>"
 *    },
 *   "desired": {
 *      "ota_url": "",
 *      "output": false
 *   }
 * }
 */

/* Per-Device Unique components:
 * - Device ID
 * - Certificate
 * - Private Key
 */

extern const uint8_t deviceid_txt_start[] asm("_binary_deviceid_txt_start");
extern const uint8_t deviceid_txt_end[] asm("_binary_deviceid_txt_end");

extern const uint8_t certificate_pem_crt_start[] asm("_binary_device_cert_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_device_cert_end");
extern const uint8_t private_pem_key_start[] asm("_binary_device_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_device_key_end");

/* Root CA Certificate */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_server_cert_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_server_cert_end");

/* AWS IoT Endpoint specific to account and region */
extern const uint8_t endpoint_txt_start[] asm("_binary_endpoint_txt_start");
extern const uint8_t endpoint_txt_end[] asm("_binary_endpoint_txt_end");

static int reported_state = false;
static bool output_changed_locally = false;
static void output_state_change_callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    if (pContext != NULL) {
        bool state = *(bool *) (pContext->pData);
        ESP_LOGI(TAG, "Delta - Output state changed to %s", state ? "true":"false");
        app_driver_set_state(state);
        output_changed_locally = false;
    }
}

static bool ota_update_done = false;
static void ota_url_state_change_callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    if (pContext != NULL) {
        char * ota_url = strndup((char *) pJsonString, (int) JsonStringDataLen);
        ESP_LOGI(TAG, "Delta - Output state changed to %d and data: %s", *(bool *) (pContext->pData), ota_url);
        if (do_firmware_upgrade(ota_url) == ESP_OK) {
            // Firmware upgrade successful
            ota_update_done = true;
        }
        free(ota_url);
    }
}

static bool shadowUpdateInProgress;
static void update_status_callback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
                                   const char *pReceivedJsonDocument, void *pContextData)
{
    IOT_UNUSED(pThingName);
    IOT_UNUSED(action);
    IOT_UNUSED(pReceivedJsonDocument);
    IOT_UNUSED(pContextData);

    shadowUpdateInProgress = false;

    if (SHADOW_ACK_TIMEOUT == status) {
        ESP_LOGE(TAG, "Update timed out");
    } else if (SHADOW_ACK_REJECTED == status) {
        ESP_LOGE(TAG, "Update rejected");
    } else if (SHADOW_ACK_ACCEPTED == status) {
        // shadow doc OTA URL reset successful
        if (ota_update_done) {
            esp_restart();
        }
        ESP_LOGI(TAG, "Update accepted");
    }
}

static IoT_Error_t shadow_update(AWS_IoT_Client *mqttClient,
                                 jsonStruct_t **reported_handles,
                                 size_t reported_count,
                                 jsonStruct_t **desired_handles,
                                 size_t desired_count)
{
    IoT_Error_t rc = FAILURE;
    char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
    size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);
    rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
    if (rc != SUCCESS) {
        return rc;
    }

    if (reported_count > 0) {
        rc = custom_aws_iot_shadow_add_reported(JsonDocumentBuffer,
                                                sizeOfJsonDocumentBuffer,
                                                reported_count,
                                                reported_handles);
        if (rc != SUCCESS) {
            return rc;
        }
    }

    if (desired_count > 0) {
        rc = custom_aws_iot_shadow_add_desired(JsonDocumentBuffer,
                            sizeOfJsonDocumentBuffer,
                            desired_count,
                            desired_handles);
        if (rc != SUCCESS) {
            return rc;
        }
    }

    rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
    if (rc != SUCCESS) {
        return rc;
    }
    ESP_LOGI(TAG, "Update Shadow: %s", JsonDocumentBuffer);
    rc = aws_iot_shadow_update(mqttClient, (const char *)deviceid_txt_start, JsonDocumentBuffer,
                               update_status_callback, NULL, 4, true);
    if (rc != SUCCESS) {
        return rc;
    }            
    shadowUpdateInProgress = true;
    return rc;
}

void aws_iot_task(void *param)
{
    IoT_Error_t rc = FAILURE;
    bool output_state = false;
    AWS_IoT_Client mqttClient;

    ShadowInitParameters_t sp = ShadowInitParametersDefault;
    sp.pHost = (char *)endpoint_txt_start;
    sp.port = AWS_IOT_MQTT_PORT;
    sp.pClientCRT = (const char *)certificate_pem_crt_start;
    sp.pClientKey = (const char *)private_pem_key_start;
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    sp.enableAutoReconnect = false;
    sp.disconnectHandler = NULL;

    ESP_LOGI(TAG, "Shadow Init");
    rc = aws_iot_shadow_init(&mqttClient, &sp);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_init returned error %d", rc);
        goto error;
    }

    ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = (const char *)deviceid_txt_start;
    scp.pMqttClientId = (const char *)deviceid_txt_start;
    scp.mqttClientIdLen = (uint16_t) strlen((const char *)deviceid_txt_start);

    ESP_LOGI(TAG, "Connecting to AWS...");
    do {
        rc = aws_iot_shadow_connect(&mqttClient, &scp);
        if(SUCCESS != rc) {
            ESP_LOGE(TAG, "Error(%d) connecting to %s:%d", rc, sp.pHost, sp.port);
            vTaskDelay(1000 / portTICK_RATE_MS);
        }
    } while (SUCCESS != rc);

    rc = aws_iot_shadow_set_autoreconnect_status(&mqttClient, true);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d", rc);
        goto aws_error;
    }
    output_state = app_driver_get_state();
    jsonStruct_t output_handler;
    output_handler.cb = output_state_change_callback;
    output_handler.pData = &output_state;
    output_handler.pKey = "output";
    output_handler.type = SHADOW_JSON_BOOL;
    rc = aws_iot_shadow_register_delta(&mqttClient, &output_handler);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register State Delta Error %d", rc);
        goto aws_error;
    }

    jsonStruct_t ota_handler;
    char ota_url[MAX_LENGTH_URL];
    strcpy(ota_url, "");
    ota_handler.cb = ota_url_state_change_callback;
    ota_handler.pData = &ota_url;
    ota_handler.pKey = "ota_url";
    ota_handler.dataLength = sizeof(ota_url);
    ota_handler.type = SHADOW_JSON_STRING;
    rc = aws_iot_shadow_register_delta(&mqttClient, &ota_handler);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register OTA Delta Error");
        goto aws_error;
    }

    jsonStruct_t fw_handler;
    fw_handler.pData = FW_VERSION;
    fw_handler.dataLength = sizeof(FW_VERSION);
    fw_handler.pKey = "fw_version";
    fw_handler.type = SHADOW_JSON_STRING;

    jsonStruct_t **desired_handles = malloc(MAX_DESIRED_PARAM * sizeof(jsonStruct_t *));
    if (desired_handles == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        goto aws_error;
    }

    jsonStruct_t **reported_handles = malloc(MAX_REPORTED_PARAM * sizeof(jsonStruct_t *));
    if (reported_handles == NULL) {
        ESP_LOGE(TAG, "Failed to allocate memory");
        free(desired_handles);
        goto aws_error;
    }

    // Report the initial values once
    size_t desired_count = 0, reported_count = 0;
    reported_handles[reported_count++] = &fw_handler;
    reported_handles[reported_count++] = &output_handler;
    rc = shadow_update(&mqttClient, reported_handles, reported_count, desired_handles,  desired_count);
    reported_state = output_state;

    while (NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
        rc = aws_iot_shadow_yield(&mqttClient, 200);
        if (NETWORK_ATTEMPTING_RECONNECT == rc || shadowUpdateInProgress) {
            rc = aws_iot_shadow_yield(&mqttClient, 1000);
            // If the client is attempting to reconnect, or already waiting on a shadow update,
            // we will skip the rest of the loop.
            continue;
        }
        desired_count = 0;
        reported_count = 0;

        if (ota_update_done) {
            // OTA update was successful
            // Reset OTA URL
            strcpy(ota_url, "");
            reported_handles[reported_count++] = &ota_handler;
            desired_handles[desired_count++] = &ota_handler;
        }

        output_state = app_driver_get_state();
        if  (reported_state != output_state) {
            reported_handles[reported_count++] = &output_handler;
            if (output_changed_locally == true) {
                desired_handles[desired_count++] = &output_handler;
            }
            output_changed_locally = true;
            reported_state = output_state;
        }

        if (reported_count > 0 || desired_count > 0) {
            rc = shadow_update(&mqttClient, reported_handles, reported_count, desired_handles,  desired_count);
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "An error occurred in the loop %d", rc);
    }
    if (reported_handles) {
        free(reported_handles);
    }
    if (desired_handles) {
        free(desired_handles);
    }

aws_error:
    ESP_LOGI(TAG, "Disconnecting");
    rc = aws_iot_shadow_disconnect(&mqttClient);

    if (SUCCESS != rc) {
	    ESP_LOGE(TAG, "Disconnect error %d", rc);
    }
error:
    vTaskDelete(NULL);
}

int cloud_start(void)
{
    printf("Starting cloud\n");
    if (xTaskCreate(&aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Couldn't create cloud task\n");
        /* Indicate error to user */
    }
    return ESP_OK;
}
