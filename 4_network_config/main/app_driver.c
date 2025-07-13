/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <iot_button.h>
#include <button_gpio.h>
#include <led_strip.h>
#include <nvs_flash.h>
#include "esp_system.h"
#include "app_priv.h"

/* This is the button that is used for toggling the power */
#define BUTTON_GPIO          CONFIG_EXAMPLE_BOARD_BUTTON_GPIO
#define BUTTON_ACTIVE_LEVEL  0

/* This is the GPIO on which the power will be set */
#define OUTPUT_GPIO    CONFIG_EXAMPLE_OUTPUT_GPIO
static bool g_output_state = false;

/* These values correspoind to H,S,V = 120,100,10 */
#define DEFAULT_RED     0
#define DEFAULT_GREEN   25
#define DEFAULT_BLUE    0

static led_strip_handle_t g_led_strip = NULL;

static void app_indicator_set(bool state)
{
    if (g_led_strip) {
        if (state) {
            led_strip_set_pixel(g_led_strip, 0, DEFAULT_RED, DEFAULT_GREEN, DEFAULT_BLUE);
            led_strip_refresh(g_led_strip);
            printf("LED: ON (Green)\n");
        } else {
            led_strip_clear(g_led_strip);
            printf("LED: OFF\n");
        }
    }
}

static void app_indicator_init(void)
{
    /* LED strip common configuration */
    led_strip_config_t strip_config = {
        .strip_gpio_num = CONFIG_WS2812_LED_GPIO,
        .max_leds = 1,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        }
    };

    /* LED strip backend configuration: RMT */
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, /* 10MHz */
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        }
    };

    /* Create the LED strip */
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &g_led_strip));
    printf("WS2812 LED initialized on GPIO %d\n", CONFIG_WS2812_LED_GPIO);
}

static void push_btn_cb(void *button_handle, void *usr_data)
{
    printf("Button pressed! Toggling power state...\n");
    app_driver_set_state(!g_output_state);
}

static void button_press_3sec_cb(void *button_handle, void *usr_data)
{
    nvs_flash_erase();
    esp_restart();
}

static void configure_push_button(int gpio_num, void (*btn_cb)(void *, void *))
{
    button_config_t btn_cfg = {
        .long_press_time = 1000,
        .short_press_time = 50,
    };

    button_gpio_config_t gpio_cfg = {
        .gpio_num = BUTTON_GPIO,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .enable_power_save = false,
        .disable_pull = false,
    };

    button_handle_t btn_handle;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn_handle);
    if (ret == ESP_OK) {
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, NULL, btn_cb, "RELEASE");
        button_event_args_t long_press_args = {
            .long_press.press_time = 3000,
        };
        iot_button_register_cb(btn_handle, BUTTON_LONG_PRESS_START, &long_press_args, button_press_3sec_cb, NULL);
        printf("Button configured on GPIO %d\n", BUTTON_GPIO);
    } else {
        printf("Failed to configure button: %s\n", esp_err_to_name(ret));
    }
}

static void set_output_state(bool target)
{
    gpio_set_level(OUTPUT_GPIO, target);
    printf("Power Output GPIO %d: %s\n", OUTPUT_GPIO, target ? "ON" : "OFF");
    app_indicator_set(target);
}

void app_driver_init()
{
    printf("Initializing Smart Power Outlet drivers...\n");

    configure_push_button(BUTTON_GPIO, push_btn_cb);

    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << OUTPUT_GPIO);
    /* Configure the GPIO */
    gpio_config(&io_conf);
    printf("Output GPIO %d configured\n", OUTPUT_GPIO);

    /* Configure the onboard LED for indication */
    app_indicator_init();
    app_driver_set_state(!g_output_state);

    printf("Smart Power Outlet ready! Press button to toggle power.\n");
}

int IRAM_ATTR app_driver_set_state(bool state)
{
    if(g_output_state != state) {
        g_output_state = state;
        set_output_state(g_output_state);
        printf("Power state changed to: %s\n", state ? "ON" : "OFF");
    }
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
