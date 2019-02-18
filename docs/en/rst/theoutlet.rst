The Driver
==========

In this Chapter we will create a basic power outlet using the driver
APIs of the ESP32. The power outlet will do the following:

-  Provide a button that the user can press

-  Toggle an output GPIO on every button press

For the scope of this chapter, we won’t worry about ’connectivity’ of
this power outlet. That will follow in subsequent chapters. Here we will
only focus in implementing the outlet functionality. You may refer to
the *2\_drivers/* directory of esp-jumpstart for looking at this code.

The code for the driver has been neatly isolated in the file
*app\_driver.c*. This way, later whenever you have to modify this
application to adapt to your product, you could simply change the
contents of this file to talk to your peripheral.

The Push Button
---------------

Let’s first create a push-button. The Devkit-C development board has a
button called ’boot’ which is connected to GPIO 0. We will configure
this button to be used to toggle the outlet’s state.

The Code
~~~~~~~~

[sec:push\_button] The code for enabling this is shown as below:

.. code:: c

    #include <iot_button.h>

    button_handle_t btn_handle=iot_button_create(BUTTON_GPIO,
                                    BUTTON_ACTIVE_LEVEL);
    iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE,
                                push_btn_cb, "RELEASE");

We use the *iot\_button* module for implementing the button. First off
we create the iot\_button object. We specify the GPIO number and the
active level of the GPIO to detect the button press. In the case of
DevKit-C the *BUTTON\_GPIO* is set to GPIO 0.

Then we register an event callback for the button, whenever the button
is *released* the ***push\_btn\_cb*** function will be called. This
function is called in the esp-timer thread’s context. So do make sure
that the default stack configured for the esp-timer thread is sufficient
for your callback function.

The *push\_btn\_cb* code then is simply as shown below:

.. code:: c

    static void push_btn_cb(void* arg)
    {
        static uint64_t previous;
        uint64_t current = xTaskGetTickCount();
        if ((current - previous) > DEBOUNCE_TIME) {
            previous = current;
            app_driver_toggle_state();
        }
    }

The *xTaskGetTickCount()* is a FreeRTOS function that provides the
current tick counts. In the callback function, we make sure that the
button press doesn’t accidentally generate multiple events in a short
duration of time. This is generally not what the end-user wants. (In the
current case, we absorb all events generated within a 300 millisecond
span, and call it a single event.) Finally, we call the function
*app\_driver\_toggle\_state()* which is responsible for toggling the
output on or off.

The Output
----------

Now we will configure a GPIO to act as the output of the power outlet.
We will assert this GPIO on or off which would ideally trigger a relay
to switch the output on or off.

The Code
~~~~~~~~

[sec:relay] First off we initialize the GPIO with the correct
configuration as shown below:

.. code:: c

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 1;
    io_conf.pin_bit_mask = ((uint64_t)1 << OUTPUT_GPIO);

    /* Configure the GPIO */
    gpio_config(&io_conf);

In this example, we have chosen GPIO 27 to act as the output. We
initialize the *gpio\_config\_t* structure with the settings to set this
as a GPIO output with internal pull-up enabled.

.. code:: c

    /* Assert GPIO */
    gpio_set_level(OUTPUT_GPIO, target);

Finally, the state of the GPIO is set using the *gpio\_set\_level()*
call.

Progress so far
---------------

With this, now we have a power outlet functionality enabled. Once you
build and flash this firmware, every time the user presses the
push-button the output from the ESP32 toggles on and off. As of now,
this is not a connected outlet though.

As our next step, let’s add Wi-Fi connectivity to this firmware.
