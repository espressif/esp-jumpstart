/* Cloud Communication

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <esp_event_loop.h>

#include <nvs_flash.h>
#include <conn_mgr_prov.h>
#include <conn_mgr_prov_mode_ble.h>
#include <conn_mgr_prov_mode_softap.h>

#include "app_priv.h"

static const char *TAG = "app_main";

/* Signal Wi-Fi events on this event-group */
const int WIFI_CONNECTED_EVENT = BIT0;
static EventGroupHandle_t wifi_event_group;

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    /* Pass event information to conn_mgr_prov so that it can
     * maintain it's internal state depending upon the system event */
    conn_mgr_prov_event_handler(ctx, event);

    /* Global event handler */
    switch (event->event_id) {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "Connected with IP Address:%s", ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
        xEventGroupSetBits(wifi_event_group, WIFI_CONNECTED_EVENT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
        esp_wifi_connect();
        break;
    default:
        break;
    }
    return ESP_OK;
}

/* Custom event handler for catching provisioning manager events */
static esp_err_t prov_event_handler(void *user_data, conn_mgr_prov_cb_event_t event)
{
    esp_err_t ret = ESP_OK;
    switch (event) {
        case CMP_PROV_END:
            /* De-initialize manager once
             * provisioning is finished */
            conn_mgr_prov_deinit();
            break;
        default:
            break;
    }
    return ret;
}

static void wifi_init_sta()
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start() );
}

static void get_device_service_name(char *service_name, size_t max)
{
    uint8_t eth_mac[6];
    const char *ssid_prefix = "PROV_";
    esp_wifi_get_mac(WIFI_IF_STA, eth_mac);
    snprintf(service_name, max, "%s%02X%02X%02X",
             ssid_prefix, eth_mac[3], eth_mac[4], eth_mac[5]);
}

void app_main()
{
    app_driver_init();

    /* Initialize NVS partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        /* NVS partition was truncated
         * and needs to be erased */
        ret = nvs_flash_erase();

        /* Retry nvs_flash_init */
        ret |= nvs_flash_init();
    }
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init NVS");
        return;
    }

    /* Initialize TCP/IP and the event loop */
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );
    wifi_event_group = xEventGroupCreate();

    /* Configuration for the provisioning manager */
    conn_mgr_prov_config_t config = {
        /* What is the Provisioning Scheme that we want ?
         * conn_mgr_prov_scheme_softap or conn_mgr_prov_scheme_ble */
        .scheme = conn_mgr_prov_scheme_ble,

        /* Any default scheme specific event handler that you would
         * like to choose. Since our example application requires
         * neither BT nor BLE, we can choose to release the associated
         * memory once provisioning is complete, or not needed
         * (in case when device is already provisioned). Choosing
         * appropriate scheme specific event handler allows the manager
         * to take care of this automatically. This can be set to
         * CMP_EVENT_HANDLER_NONE when using conn_mgr_prov_scheme_softap*/
        .scheme_event_handler = CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,

        /* Do we want an application specific handler be executed on
         * various provisioning related events */
        .app_event_handler = {
            .event_cb = prov_event_handler,
            .user_data = NULL
        }
    };

    /* Initialize provisioning manager with the
     * configuration parameters set above */
    if (conn_mgr_prov_init(config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize provisioning manager");
        return;
    }

    bool provisioned = false;
    /* Let's find out if the device is provisioned */
    if (conn_mgr_prov_is_provisioned(&provisioned) != ESP_OK) {
        ESP_LOGE(TAG, "Error getting device provisioning state");
        return;
    }

    /* If device is not yet provisioned start provisioning service */
    if (!provisioned) {
        ESP_LOGI(TAG, "Starting provisioning");

        /* What is the Device Service Name that we want
         * This translates to :
         *     - WiFi SSID when scheme is conn_mgr_prov_scheme_softap
         *     - device name when scheme is conn_mgr_prov_scheme_ble
         */
        char service_name[12];
        get_device_service_name(service_name, sizeof(service_name));

        /* What is the security level that we want (0 or 1):
         *      - Security 0 is simply plain text communication.
         *      - Security 1 is secure communication which consists of secure handshake
         *          using X25519 key exchange and proof of possession (pop) and AES-CTR
         *          for encryption/decryption of messages.
         */
        int security = 1;

        /* Do we want a proof-of-possession (ignored if Security 0 is selected):
         *      - this should be a string with length > 0
         *      - NULL if not used
         */
        const char *pop = "abcd1234";

        /* What is the service key (could be NULL)
         * This translates to :
         *     - WiFi password when scheme is conn_mgr_prov_scheme_softap
         *     - simply ignored when scheme is conn_mgr_prov_scheme_ble
         */
        const char *service_key = NULL;

        /* Start provisioning service */
        conn_mgr_prov_start_provisioning(security, pop, service_name, service_key);

        /* Uncomment the following to wait for the provisioning to finish and then release
         * the resources of the manager. Since in this case de-initialization is triggered
         * by the configured prov_event_handler(), we don't need to call the following */
        // conn_mgr_prov_wait();
        // conn_mgr_prov_deinit();
    } else {
        ESP_LOGI(TAG, "Already provisioned, starting station");

        /* Start wifi station */
        wifi_init_sta();

        /* We don't need the manager as device is already provisioned,
         * so let's release it's resources */
        conn_mgr_prov_deinit();
    }

    /* Wait for Wi-Fi connection */
    xEventGroupWaitBits(wifi_event_group, WIFI_CONNECTED_EVENT, false, true, portMAX_DELAY);
    cloud_start();
}
