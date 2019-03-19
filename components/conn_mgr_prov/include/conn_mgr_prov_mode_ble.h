/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#ifdef CONFIG_BT_ENABLED
#pragma once

#include <protocomm.h>
#include <protocomm_ble.h>

#include "conn_mgr_prov.h"

extern conn_mgr_prov_t conn_mgr_prov_mode_ble;

typedef protocomm_ble_config_t conn_mgr_prov_mode_ble_config_t;
#endif /* CONFIG_BT_ENABLED */
