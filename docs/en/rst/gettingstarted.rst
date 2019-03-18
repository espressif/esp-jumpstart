Getting Started
===============

In this chapter, our aim would be to get our development setup
functional, and also to get an understanding for the development tools
and repositories available around ESP8266.

Development Overview
--------------------

The following diagram depicts the typical developer setup for
development with ESP8266.

.. figure:: ../../_static/dev_setup.png
   :alt: Typical Developer Setup

   Typical Developer Setup

The PC, or the Development Host can be any of Linux, Windows or Mac. The
ESP8266 based development board is connected to the Development Host
over a USB cable. The Development Host has the ESP8266\_RTOS\_SDK
(Espressif’s SDK), the compiler toolchain and the code for your project.
The development host builds this code and generates the executable
firmware image. The tools on the Development Host then download the
generated firmware image on to the development board. As the firmware
executes on the development board, the logs from the firmware can be
monitored from the Development Host.

ESP8266\_RTOS\_SDK
------------------

ESP8266\_RTOS\_SDK is Espressif’s IoT Development Framework for ESP8266.

-  ESP8266\_RTOS\_SDK is a collection of libraries and header files that
   provides the core software components that are required to build any
   software projects on ESP8266.

-  ESP8266\_RTOS\_SDK also provides tools and utilities that are
   required for typical developer and production usecases, like build,
   flash, debug and measure.

Setting up ESP8266\_RTOS\_SDK
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Please follow the steps in the README.md for setting up
ESP8266\_RTOS\_SDK: https://github.com/espressif/ESP8266_RTOS_SDK.
Please complete all the steps on this page.

Before proceeding, please ensure that you have setup your development
host, and have built the first application as indicated in this page.
Now that you have done that, let’s look at some additional details about
ESP8266\_RTOS\_SDK.

ESP8266\_RTOS\_SDK Details
~~~~~~~~~~~~~~~~~~~~~~~~~~

The ESP8266\_RTOS\_SDK has a component based design.

.. figure:: ../../_static/idf_comp.png
   :alt: Component Based Design

   Component Based Design

All the software in the ESP8266\_RTOS\_SDK is available as components.
The Operating System, the network stack, Wi-Fi drivers, middleware
modules like the HTTP Server are all components within
ESP8266\_RTOS\_SDK.

This design allows you to use your own or third-party components that
are built for ESP8266\_RTOS\_SDK.

A developer typically builds *applications* against the
ESP8266\_RTOS\_SDK. The applications contain the business logic, any
drivers for externally interfaced peripherals and the SDK configuration.

.. figure:: ../../_static/app_structure.png
   :alt: Application’s Structure

   Application’s Structure

An application must contain one *main* component. This is the primary
component that holds the application logic. The application may
additionally include other components as may be desired. The
application’s *Makefile* defines the build instructions for the
application. Additionally, an optional *sdkconfig.defaults* may be
placed that picks up the default SDK configuration that should be
selected for this application.

Getting ESP-Jumpstart
---------------------

The ESP-Jumpstart repository contains a sequence of *applications* that
we will use for this exercise. These applications build with the
ESP8266\_RTOS\_SDK that you have setup before. Let’s get started by
cloning the ESP-Jumpstart git repository
https://github.com/espressif/esp-jumpstart.

::

    $ git clone --recursive https://github.com/espressif/esp-jumpstart

Currently, ESP-Jumpstart uses the commit
93e3a3f5424e76def8afb3c41e625471490c056b of ESP8266\_RTOS\_SDK. Let us
first switch to that version of ESP8266\_RTOS\_SDK.

::

    $ cd ESP8266\_RTOS\_SDK
    $ git checkout -b release/jumpstart 93e3a3f5424e76def8afb3c41e625471490c056b

Now we build our first, *Hello World*, application from ESP-Jumpstart
and flash it on to our development board. You should be already familiar
with most of the steps below.

::

    $ cd esp-jumpstart/1_hello_world
    $ make -j8 menuconfig
    $ export ESPPORT=/dev/cu.SLAB_USBTOUART   # Or the correct device name for your setup
    $ export ESPBAUD=921600
    $ make -j8 flash monitor

This will then build the entire SDK and the application. Once the build
is successful, it will write the generated firmware to the device.

Once the flashing is successful, the device will reset and you will see
the console output from this firmware.

The Code
--------

Now let’s look at the code of the Hello World Application. It is only a
few lines of code as shown below:

.. code:: c

    #include <stdio.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"


    void app_main()
    {
        int i = 0;
        while (1) {
            printf("[%d] Hello world!\n", i);
            i++;
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

The code is fairly simple. A few takeaways:

-  The app\_main() function is the application entry point. All
   applications begin execution at this point. This function gets called
   after the FreeRTOS kernel is already executing on the ESP8266. Once
   FreeRTOS is initialised, it forks an application thread, called the
   main thread. The app\_main() function is called in this thread’s
   context. The stack of the application thread can be configured
   through the SDK configuration.

-  C library functions like printf(), strlen(), time() can be directly
   called. The ESP8266\_RTOS\_SDK uses the newlib C library, which is a
   low-footprint implementation of the C library. Most of the category
   of functions of the C library like stdio, stdlib, string operations,
   math, time/timezones, file/directory operations are supported.
   Support for signals, locales, wchrs is not available. In our example
   above, we use the printf() function for printing to the console.

-  FreeRTOS is the operating system powering the core. FreeRTOS
   (https://www.freertos.org) is a tiny kernel that provides mechanisms
   for task creation, inter-task communication (semaphores, message
   queues, mutexes), interrupts and timers. In our example above, we use
   the vTaskDelay function for putting the thread to sleep for 5
   seconds. Details of the FreeRTOS APIs are available at:
   https://www.freertos.org/a00106.html

Progress so far
---------------

Now we have the basic development setup and process in place. We can
build the code into executable firmware images. We can flash these
images to a connected development board, and we can monitor the console
to look at debug logs and messages generated by the firmware.

Let’s now build a simple power outlet with ESP8266.
