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
#define EXAMPLE_THING_NAME "change-me"
extern const uint8_t certificate_pem_crt_start[] asm("_binary_certificate_pem_crt_start");
extern const uint8_t certificate_pem_crt_end[] asm("_binary_certificate_pem_crt_end");
extern const uint8_t private_pem_key_start[] asm("_binary_private_pem_key_start");
extern const uint8_t private_pem_key_end[] asm("_binary_private_pem_key_end");

/* Root CA Certificate */
extern const uint8_t aws_root_ca_pem_start[] asm("_binary_aws_root_ca_pem_start");
extern const uint8_t aws_root_ca_pem_end[] asm("_binary_aws_root_ca_pem_end");
#define AWS_IOT_MY_MQTT_HOSTNAME   "aln7lww42a72l-ats.iot.us-east-2.amazonaws.com"

static int reported_state = false;
static bool update_desired = false;
static void state_change_callback(const char *pJsonString, uint32_t JsonStringDataLen, jsonStruct_t *pContext)
{
    if (pContext != NULL) {
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
    char *ota_url = strndup((char *)params->payload, (int) params->payloadLen);
    do_firmware_upgrade(ota_url);
    /* If upgrade was successful, the above function call will not return */
    free(ota_url);
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
        version_data.pData = APP_VERSION;
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
    if (SUCCESS == rc) {
        rc = get_reported_data(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, handler);
        if (SUCCESS != rc) {
            goto out;
        }
        if (update_desired) {
            /* If this was updated locally, then we should also update
             * the 'desired' state, else we will get a delta callback
             * again.
             */
            rc = aws_iot_shadow_add_desired(JsonDocumentBuffer, sizeOfJsonDocumentBuffer, 1, handler);
            if (SUCCESS != rc) {
                goto out;
            }
        }
        rc = aws_iot_finalize_json_document(JsonDocumentBuffer, sizeOfJsonDocumentBuffer);
        if (SUCCESS == rc) {
            ESP_LOGI(TAG, "Update Shadow: %s", JsonDocumentBuffer);
            rc = aws_iot_shadow_update(mqttClient_p, EXAMPLE_THING_NAME, JsonDocumentBuffer,
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
    sp.pClientCRT = (const char *)certificate_pem_crt_start;
    sp.pClientKey = (const char *)private_pem_key_start;
    sp.pRootCA = (const char *)aws_root_ca_pem_start;
    sp.enableAutoReconnect = false;
    sp.disconnectHandler = NULL;

    ESP_LOGI(TAG, "Shadow Init");
    rc = aws_iot_shadow_init(&mqttClient, &sp);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_init returned error %d, aborting...", rc);
        abort();
    }

    ShadowConnectParameters_t scp = ShadowConnectParametersDefault;
    scp.pMyThingName = EXAMPLE_THING_NAME;
    scp.pMqttClientId = EXAMPLE_THING_NAME;
    scp.mqttClientIdLen = (uint16_t) strlen(EXAMPLE_THING_NAME);

    ESP_LOGI(TAG, "Shadow Connect");
    rc = aws_iot_shadow_connect(&mqttClient, &scp);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "aws_iot_shadow_connect returned error %d, aborting...", rc);
        abort();
    }

    rc = aws_iot_shadow_set_autoreconnect_status(&mqttClient, true);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Unable to set Auto Reconnect to true - %d, aborting...", rc);
        abort();
    }

    jsonStruct_t state_handler;
    state_handler.cb = state_change_callback;
    state_handler.pData = &output_state;
    state_handler.pKey = "output";
    state_handler.type = SHADOW_JSON_BOOL;
    rc = aws_iot_shadow_register_delta(&mqttClient, &state_handler);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Shadow Register Delta Error");
    }

    rc = aws_iot_mqtt_subscribe(&mqttClient, UPGRADE_TOPIC_NAME, strlen(UPGRADE_TOPIC_NAME),
                                QOS1, upgrade_handler, NULL);
    if (SUCCESS != rc) {
        ESP_LOGE(TAG, "Upgrade Handler Subscription Error");
    }

    output_state = app_driver_get_state();
    update_our_status(&mqttClient, &state_handler);
    while (NETWORK_ATTEMPTING_RECONNECT == rc || NETWORK_RECONNECTED == rc || SUCCESS == rc) {
        rc = aws_iot_shadow_yield(&mqttClient, 200);
        if (NETWORK_ATTEMPTING_RECONNECT == rc || shadowUpdateInProgress) {
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

int cloud_start(void)
{
    printf("Starting cloud\n");
    if (xTaskCreate(&aws_iot_task, "aws_iot_task", 9216, NULL, 5, NULL) != pdPASS) {
        ESP_LOGE(TAG, "Couldn't create cloud task\n");
        /* Indicate error to user */
    }
    return ESP_OK;
}
