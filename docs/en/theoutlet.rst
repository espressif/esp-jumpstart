The Driver
==========

:link_to_translation:`zh_CN:[中文]`

In this Chapter we will create a basic power outlet using the driver
APIs of the ESP32. The power outlet will do the following:

-  Provide a button that the user can press

-  Toggle an output GPIO on every button press

For the scope of this chapter, we won't worry about 'connectivity' of
this power outlet. That will follow in subsequent chapters. Here we will
only focus on implementing the outlet functionality. You may refer to
the *2\_drivers/* directory of esp-jumpstart for looking at this code.

The code for the driver has been neatly isolated in the file
*app\_driver.c*. This way, later whenever you have to modify this
application to adapt to your product, you could simply change the
contents of this file to talk to your peripheral.

Component Dependencies
----------------------

ESP-Jumpstart uses modern component management through the ESP Component
Registry. The required components are automatically downloaded when you
build the examples. The driver example uses the following components:

-  **espressif/button**: For button handling with debouncing and event callbacks
-  **espressif/led_strip**: For WS2812 LED strip control (if enabled)

These components are specified in the *idf_component.yml* file and are
automatically managed by the ESP Component Manager.

The Push Button
---------------

Let's first create a push-button. The Devkit-C development board has a
button called 'boot' which is connected to GPIO 0. We will configure
this button to be used to toggle the outlet's state.

.. _sec_push\_button:

The Code
~~~~~~~~

The code for enabling this is shown as below:

.. code:: c

    #include <iot_button.h>
    #include <button_gpio.h>

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
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, NULL, push_btn_cb, "RELEASE");
    }

We use the *button* component from the ESP Component Registry for
implementing the button functionality. The modern button API uses two
separate configuration structures:

- *button_config_t* for general button settings like press timing thresholds
- *button_gpio_config_t* for GPIO-specific settings like pin number and active level

First, we configure the button timing with *button_config_t*, setting the
long press time to 1000ms and short press time to 50ms. Then we configure
the GPIO-specific settings with *button_gpio_config_t*, specifying the GPIO
number and active level.

We create the button device using *iot_button_new_gpio_device()* and then
register an event callback for the button. Whenever the button is *released*
(BUTTON_PRESS_UP), the ***push\_btn\_cb*** function will be called. This
function is called in the button component's internal task context.

The *push\_btn\_cb* code then is simply as shown below:

.. code:: c

    static void push_btn_cb(void *button_handle, void *usr_data)
    {
        app_driver_set_state(!g_output_state);
    }

The callback function signature receives the button handle and user data
as parameters. The modern button component handles debouncing internally
through the timing configuration we set earlier (short_press_time = 50ms),
so we don't need to implement debouncing logic in the callback.

The callback simply calls the function *app\_driver\_set\_state()* which
is responsible for toggling the output on or off.

The Output
----------

Now we will configure a GPIO to act as the output of the power outlet.
We will assert this GPIO on or off which would ideally trigger a relay
to switch the output on or off.

.. _sec_relay:

The Code
~~~~~~~~

First off we initialize the GPIO with the correct
configuration as shown below:

.. code:: c

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 1;
    io_conf.pin_bit_mask = ((uint64_t)1 << JUMPSTART_BOARD_OUTPUT_GPIO);

    /* Configure the GPIO */
    gpio_config(&io_conf);

In this example, we have chosen GPIO 27 to act as the output. We
initialize the *gpio\_config\_t* structure with the settings to set this
as a GPIO output with internal pull-up enabled.

.. code:: c

    /* Assert GPIO */
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_GPIO, target);

Finally, the state of the GPIO is set using the *gpio\_set\_level()*
call.

Progress so far
---------------

With this, now we have a power outlet functionality enabled. Once you
build and flash this firmware, every time the user presses the
push-button the output from the ESP32 toggles on and off. As of now,
this is not a connected outlet though.

As our next step, let's add Wi-Fi connectivity to this firmware.
