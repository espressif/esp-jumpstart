/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <protocomm.h>
#include <protocomm_ble.h>

#include "conn_mgr_prov.h"

typedef protocomm_ble_config_t conn_mgr_prov_ble_config_t;

/**
 * @brief   Scheme that can be used by manager for provisioning
 *          over BLE transport with GATT server
 */
extern const conn_mgr_prov_scheme_t conn_mgr_prov_scheme_ble;

/* This scheme specific event handler is to be used when application
 * doesn't require BT and BLE after provisioning has finished */
#define CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM {              \
    .event_cb  = conn_mgr_prov_scheme_ble_event_cb_free_btdm, \
    .user_data = NULL                                         \
}

/* This scheme specific event handler is to be used when application
 * doesn't require BLE to be active after provisioning has finished */
#define CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BLE {              \
    .event_cb  = conn_mgr_prov_scheme_ble_event_cb_free_ble, \
    .user_data = NULL                                        \
}

/* This scheme specific event handler is to be used when application
 * doesn't require BT to be active after provisioning has finished */
#define CMP_SCHEME_BLE_EVENT_HANDLER_FREE_BT {              \
    .event_cb  = conn_mgr_prov_scheme_ble_event_cb_free_bt, \
    .user_data = NULL                                       \
}

esp_err_t conn_mgr_prov_scheme_ble_event_cb_free_btdm(void *user_data, conn_mgr_prov_cb_event_t event);
esp_err_t conn_mgr_prov_scheme_ble_event_cb_free_ble(void *user_data, conn_mgr_prov_cb_event_t event);
esp_err_t conn_mgr_prov_scheme_ble_event_cb_free_bt(void *user_data, conn_mgr_prov_cb_event_t event);
