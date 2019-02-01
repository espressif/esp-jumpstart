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
#include <nvs_flash.h>
#include <nvs.h>

#include "aws_iot_config.h"
#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_client_interface.h"
#include "aws_iot_shadow_interface.h"

#include "app_priv.h"

#define MAX_LENGTH_OF_UPDATE_JSON_BUFFER 200
static const char *TAG = "cloud";
#define UPGRADE_TOPIC_NAME   "outlet/upgrade"
/* 
 * The Json Document in the cloud will be:
 * {
 *   "reported": {
 *      "output": true,
 *      "version": "<version-string>"
 *    },
 *   "desired": {
 *      "output": false
 *   }
 * }
 */

/* Per-Device Unique components:
 * - Certificate
 * - Private Key
 * - Thing Name
 */
static char *serial_no, *cert, *priv_key;
#define MFG_PARTITION_NAME "fctry"

/* Root CA Certificate */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
#define AWS_IOT_MY_MQTT_HOSTNAME   "aln7lww42a72l-ats.iot.us-east-2.amazonaws.com"

static int reported_state = false;
static bool update_desired = false;
static void state_change_callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    if(pContext != NULL) {
	ESP_LOGI(TAG, "Delta - Output state changed to %d", *(bool *) (pContext->pData));
        app_driver_toggle_state();
        update_desired = false;
    }
}

void upgrade_handler(AWS_IoT_Client *pClient, char *pTopicName, uint16_t topicNameLen,
      IoT_Publish_Message_Params *params, void *pClientData)
{
    /* Starting Firmware Upgrades */
    printf("Upgrade Handler got:%.*s on %.*s topic\n", (int) params->payloadLen, (char *)params->payload, topicNameLen, pTopicName);
    do_firmware_upgrade(strndup((char *)params->payload, (int) params->payloadLen));
    /* If upgrade was successful, the above function call will not return */
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

    if(SHADOW_ACK_TIMEOUT == status) {
        ESP_LOGE(TAG, "Update timed out");
    } else if(SHADOW_ACK_REJECTED == status) {
        ESP_LOGE(TAG, "Update rejected");
    } else if(SHADOW_ACK_ACCEPTED == status) {
        ESP_LOGI(TAG, "Update accepted");
    }
}

static IoT_Error_t get_reported_data(char *JsonDocumentBuffer, size_t sizeOfJsonDocumentBuffer, jsonStruct_t *handler)
{
    static bool first_time = true;
    if (first_time) {
        first_time = false;
            jsonStruct_t version_data;
            version_data.cb = NULL;
            version_data.pData = VERSION;
            version_data.pKey = "version";
            version_data.type = SHADOW_JSON_STRING;

        return aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 2, handler, &version_data);
    } else {
        return aws_iot_shadow_add_reported(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 1, handler);
    }
}

static void update_our_status(AWS_IoT_Client *mqttClient_p, jsonStruct_t *handler)
{
    IoT_Error_t rc = FAILURE;
    char JsonDocumentBuffer[MAX_LENGTH_OF_UPDATE_JSON_BUFFER];
    size_t sizeOfJsonDocumentBuffer = sizeof(JsonDocumentBuffer) / sizeof(JsonDocumentBuffer[0]);

    rc = aws_iot_shadow_init_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
    if(SUCCESS == rc) {
        rc = get_reported_data(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, handler);
        if(SUCCESS != rc) {
            goto out;
        }
        if (update_desired) {
            /* If this was updated locally, then we should also update
             * the 'desired' state, else we will get a delta callback
             * again.
             */
            rc = aws_iot_shadow_add_desired(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 1, handler);
            if(SUCCESS != rc) {
                goto out;
            }
        }
        rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
        if(SUCCESS == rc) {
            ESP_LOGI(TAG, "Update Shadow: %s", JsonDocumentBuffer);
            rc = aws_iot_shadow_update(mqttClient_p, serial_no, JsonDocumentBuffer,
                                       update_status_callback, NULL, 4, true);
            shadowUpdateInProgress = true;
        }
    }
 out:
    /* Reset the update_desired state to true. This ensures that for
     * local modifications desired is updated. If we get a remote call
     * to update the state, update_desired will be reset.
     */
    update_desired = true;
}

void aws_iot_task(void *param)
{
    IoT_Error_t rc = FAILURE;
    bool output_state = false;
    AWS_IoT_Client mqttClient;

    ShadowInitParameters_t sp = ShadowInitParametersDefault;
    sp.pHost = AWS_IOT_MY_MQTT_HOSTNAME;
    sp.port = AWS_IOT_MQTT_PORT;
    sp.pClientCRT = (const char *)cert;
    sp.pClientKey = (const char *)priv_key;
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    sp.enableAutoReconnect = false;
    sp.disconnectHandler = NULL;

    ESP_LOGI(TAG, "Shadow Init");
    rc = aws_iot_shadow_init(&mqttClient, &sp);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_init returned error %d, aborting...", rc);
        abort();
    }

    ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = serial_no;
    scp.pMqttClientId = serial_no;
    scp.mqttClientIdLen = (uint16_t) strlen(serial_no);

    ESP_LOGI(TAG, "Shadow Connect");
    rc = aws_iot_shadow_connect(&mqttClient, &scp);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_connect returned error %d, aborting...", rc);
        abort();
    }

    rc = aws_iot_shadow_set_autoreconnect_status(&mqttClient, true);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d, aborting...", rc);
        abort();
    }

    jsonStruct_t state_handler;
    state_handler.cb = state_change_callback;
    state_handler.pData = &output_state;
    state_handler.pKey = "output";
    state_handler.type = SHADOW_JSON_BOOL;
    rc = aws_iot_shadow_register_delta(&mqttClient, &state_handler);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register Delta Error");
    }

    rc = aws_iot_mqtt_subscribe(&mqttClient, UPGRADE_TOPIC_NAME, strlen(UPGRADE_TOPIC_NAME),
                                QOS1, upgrade_handler, NULL);
    if(SUCCESS != rc) {
        ESP_LOGE(TAG, "Upgrade Handler Subscription Error");
    }
    
    output_state = app_driver_get_state();
    update_our_status(&mqttClient, &state_handler);
    while(NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
            rc = aws_iot_shadow_yield(&mqttClient, 200);
        if(NETWORK_ATTEMPTING_RECONNECT == rc || shadowUpdateInProgress) {
            rc = aws_iot_shadow_yield(&mqttClient, 1000);
            // If the client is attempting to reconnect, or already waiting on a shadow update,
            // we will skip the rest of the loop.
            continue;
	}

        output_state = app_driver_get_state();
        if (reported_state == output_state) {
            /* Don't update if the state is the same */
            continue;
        }

        reported_state = output_state;
    	ESP_LOGI(TAG, "Updating our state to cloud");
        update_our_status(&mqttClient, &state_handler);
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}

static int alloc_and_read_from_nvs(nvs_handle handle, const char *key, char **value)
{
    size_t required_size = 0;
    int error;
    if ((error = nvs_get_str(handle, key, NULL, &required_size)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read key %s with error %d size %d\n", key, error, required_size);
        return -1;
    }
    *value = malloc(required_size);
    if (*value) {
        nvs_get_str(handle, key, *value, &required_size);
        ESP_LOGI(TAG, "Read key:%s, value:%s\n", key, *value);
        return 0;
    }
    return -1;
}

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
        ESP_LOGE(TAG, "NVS open failed");
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

    if (xTaskCreate(&aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Couldn't create cloud task\n");
        /* Indicate error to user */
    }
    return ESP_OK;
}
