/* Manufacturing

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "app_priv.h"
static const char *TAG = "app_main";

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

    /* Initialise Wi-Fi */
    app_wifi_init();

    /* Start Wi-Fi (Provisioning OR Station, depending on the state */
    app_wifi_start();

    cloud_start();
}
