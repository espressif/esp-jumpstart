/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_event_loop.h>
#include <protocomm.h>

typedef struct {
    esp_err_t (*prov_start) (protocomm_t *pc, void *config);
    esp_err_t (*prov_stop)  (protocomm_t *pc);
    void *    (*new_config) (void);
    void      (*delete_config) (void *config);
    esp_err_t (*set_config_service) (void *config, const char *service_name, const char *service_key);
    esp_err_t (*set_config_endpoint) (void *config, const char *endpoint_name, uint16_t uuid);
    int         wifi_mode;
} wifi_prov_t;

/**
 * @brief   Event handler for provisioning app
 *
 * This is called from the main event handler and controls the
 * provisioning application, depeding on WiFi events
 *
 * @param[in] ctx   Event context data
 * @param[in] event Event info
 *
 * @return
 *  - ESP_OK      : Event handled successfully
 *  - ESP_FAIL    : Failed to start server on event AP start
 */
esp_err_t wifi_prov_event_handler(void *ctx, system_event_t *event);

/**
 * @brief   Checks if device is provisioned
 * *
 * @param[out] provisioned  True if provisioned, else false
 *
 * @return
 *  - ESP_OK      : Retrieved provision state successfully
 *  - ESP_FAIL    : Failed to retrieve provision state
 */
esp_err_t wifi_prov_is_provisioned(bool *provisioned);

/**
 * @brief   Start provisioning
 *
 * @param[in] mode      Provisioning mode to use (BLE/SoftAP)
 * @param[in] security  Security mode
 * @param[in] pop       Pointer to proof of possesion (NULL if not present)
 *
 * @return
 *  - ESP_OK      : Provisioning started successfully
 *  - ESP_FAIL    : Failed to start
 */
esp_err_t wifi_prov_start_provisioning(wifi_prov_t mode, int security, const char *pop,
                                       const char *service_name, const char *service_key);
