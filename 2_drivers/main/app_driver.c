/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <iot_button.h>

#include "app_priv.h"
#include JUMPSTART_BOARD
#ifdef CONFIG_IDF_TARGET_ESP32C3
#include "driver/rmt.h"
#include "led_strip.h"
static led_strip_t *g_strip_handle = NULL;
#endif

static bool g_output_state = false;
static void push_btn_cb(void *arg)
{
    app_driver_set_state(!g_output_state);
}

static void configure_push_button(int gpio_num, void (*btn_cb)(void *))
{
    button_handle_t btn_handle = iot_button_create(JUMPSTART_BOARD_BUTTON_GPIO, JUMPSTART_BOARD_BUTTON_ACTIVE_LEVEL);
    if (btn_handle) {
        iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE, btn_cb, "RELEASE");
    }
}

static void set_output_state(bool target)
{
#ifdef CONFIG_IDF_TARGET_ESP32
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_GPIO, target);
#elif CONFIG_IDF_TARGET_ESP32C3
    if (target) {
        g_strip_handle->set_pixel(g_strip_handle, 0, 255, 255, 255);
        g_strip_handle->refresh(g_strip_handle, 100);
    } else {
        g_strip_handle->clear(g_strip_handle, 100);
    }
#endif
}

void app_driver_init()
{
    configure_push_button(JUMPSTART_BOARD_BUTTON_GPIO, push_btn_cb);

#ifdef CONFIG_IDF_TARGET_ESP32
    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << JUMPSTART_BOARD_OUTPUT_GPIO);
    /* Configure the GPIO */
    gpio_config(&io_conf);
#elif CONFIG_IDF_TARGET_ESP32C3
    g_strip_handle = led_strip_init(RMT_CHANNEL_0, JUMPSTART_BOARD_OUTPUT_GPIO, 1);
    app_driver_set_state(!g_output_state);
#endif
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if(g_output_state != state) {
        g_output_state = state;
        set_output_state(g_output_state);
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
