/* Cloud task

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <esp_log.h>
#include "esp_https_ota.h"


extern const uint8_t upgrade_server_cert_pem_start[] asm("_binary_github_server_cert_start");
extern const uint8_t upgrade_server_cert_pem_end[] asm("_binary_github_server_cert_end");

esp_err_t do_firmware_upgrade(const char *url)
{
    if (!url) {
        return ESP_FAIL;
    }
    esp_http_client_config_t config = {
        .url = url,
        .cert_pem = (char *)upgrade_server_cert_pem_start,
    };
    return esp_https_ota(&config);
}
