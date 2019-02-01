/* Cloud task

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include "esp_https_ota.h"

static const char *TAG = "cloud";
extern const uint8_t upgrade_server_cert_pem_start[] asm("_binary_github_ca_cert_pem_start");
extern const uint8_t upgrade_server_cert_pem_end[] asm("_binary_github_ca_cert_pem_end");

void do_firmware_upgrade(const char *url)
{
    if (!url) {
        return;
    }
    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (char *)upgrade_server_cert_pem_start,
    };
    esp_err_t ret = esp_https_ota(&config);
    if (ret == ESP_OK) {
        esp_restart();
    } else {
        ESP_LOGE(TAG, "Firmware Upgrades Failed");
    }
}
