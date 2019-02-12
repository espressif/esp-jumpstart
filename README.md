# ESP-Jumpstart

This Project is aimed at guiding developers to build products using ESP32 *fast*.
It is designed as a tutorial with features introduced incrementally, leading to a fully-functional
production-ready implementation.

## Getting Started

Please ensure to use ESP-IDF v3.2 from either tagged release or command,
```
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout -b release/v3.2 origin/release/v3.2
export IDF_PATH=</path/to/esp-idf/>
```

## Overview

This project is a tutorial with features incrementally. The step-by-step guidance will help building "Fully Functional" IoT Products using ESP-IDF software
framework on ESP32.
By the end of this, a firmware for a `power outlet` is built with the following functionality:
* Ability to configure user’s home Wi-Fi network configuration through phone applications (iOS/Android)
* Ability to switch on or off a single GPIO output
* Ability to use a push-button to physically toggle this output
* Ability to remotely control this output through a cloud
* Ability to perform over-the-air (OTA) firmware upgrade
* Ability to perform Reset to Factory settings on long-press of the push-button

## 1_hello_world
Getting started point by setting up the development environment using ESP-IDF and a simple
application periodically showing `hello world` string on UART/console

## 2_drivers
Configure a GPIO to act as our output, and then configure one GPIO to act as the input

## 3_wifi_connection
Connect the device to a Wi-Fi network with static or compile-time Wi-Fi credentials.

## 4_network_config
Real world on-boarding of the device to Wi-Fi network with secure and dynamic configuration
over multiple transports like BLE or SoftAP interface. Ability to reconfigure device
with reset-to-factory mode based on long press of a button.

## 5_cloud
Securely connect the device to the "Internet" by introducing cloud connectivity.
AWS IoT has been used as a reference.

## 6_ota
Device firmware over-the-air upgrades for pushing latest and greatest software.

## 7_mfg
Separation of configuration and firmware partitions for seamless manufacturing or factory
programming.
