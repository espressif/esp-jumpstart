/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#pragma once
#include "stdbool.h"
#include "esp_err.h"

void app_driver_init(void);
int app_driver_set_state(bool state);
bool app_driver_get_state(void);
int cloud_start(void);
esp_err_t do_firmware_upgrade(const char *url);
