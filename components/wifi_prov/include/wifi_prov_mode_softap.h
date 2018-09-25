/* Unified Provisioning Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <protocomm.h>
#include <protocomm_httpd.h>

#include "wifi_prov.h"

extern wifi_prov_t wifi_prov_mode_softap;

typedef struct {
    protocomm_httpd_config_t httpd_config;
    char ssid[33];
    char password[65];
} wifi_prov_mode_softap_config_t;
