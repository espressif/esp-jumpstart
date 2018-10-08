/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <driver/gpio.h>

#include "app_priv.h"

/* This is the button that is used for toggling the output */
#define BUTTON_GPIO    0
/* This is the GPIO on which the output will be set */
#define OUTPUT_GPIO    14
#define DEBOUNCE_TIME  30

static bool g_output_state;
static void IRAM_ATTR push_btn_isr_handler(void* arg)
{
    static uint64_t previous;
    uint64_t current = xTaskGetTickCount();
    if ((current - previous) > DEBOUNCE_TIME) {
        /* Printing in interrupts isn't a good practice. This is only so we have a visual feedback. */
        ets_printf("Button Pressed\n");
        previous = current;
        app_driver_toggle_state();
    }
}

static void configure_push_button(int gpio_num, void (*isr_handler)(void *))
{
    gpio_config_t io_conf = {
        .intr_type = GPIO_PIN_INTR_POSEDGE,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << gpio_num);
    /* Configure the GPIO */
    gpio_config(&io_conf);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for specific gpio pin
    gpio_isr_handler_add(gpio_num, isr_handler, NULL);
}

static void set_output_state(bool target)
{
    gpio_set_level(OUTPUT_GPIO, target);
}

void app_driver_init()
{
    configure_push_button(BUTTON_GPIO, push_btn_isr_handler);

    /* Configure output */
    gpio_config_t io_conf = {
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
    };
    io_conf.pin_bit_mask = ((uint64_t)1 << OUTPUT_GPIO);
    /* Configure the GPIO */
    gpio_config(&io_conf);
}

int IRAM_ATTR app_driver_toggle_state(void)
{
    g_output_state = ! g_output_state;
    set_output_state(g_output_state);
    return ESP_OK;
}

bool app_driver_get_state(void)
{
    return g_output_state;
}
