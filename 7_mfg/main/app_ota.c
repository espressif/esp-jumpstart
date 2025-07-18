/* Cloud task

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <sdkconfig.h>
#include <esp_log.h>
#include "esp_https_ota.h"
#include <esp_idf_version.h>
#include <esp_crt_bundle.h>

esp_err_t do_firmware_upgrade(const char *url)
{
    if (!url) {
        return ESP_FAIL;
    }
    esp_http_client_config_t config = {
        .url = url,
        .crt_bundle_attach = esp_crt_bundle_attach,
        .timeout_ms = 5000,
        .keep_alive_enable = true,
    };
#ifdef CONFIG_ESP_RMAKER_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true;
#endif

    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    return esp_https_ota(&ota_config);
}

