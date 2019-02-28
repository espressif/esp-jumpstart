/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <protocomm.h>
#include <protocomm_httpd.h>

#include "conn_mgr_prov.h"

/**
 * @brief   Scheme that can be used by manager for provisioning
 *          over SoftAP transport with HTTP server
 */
extern const conn_mgr_prov_scheme_t conn_mgr_prov_scheme_softap;
