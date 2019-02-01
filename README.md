# ESP-Jumpstart

This Project is aimed at guiding developers to build simple products using ESP32.
It is designed is a tutorial wherein features will be introduced incrementally, leading to a
production ready implementation.

## Getting Started

Please ensure to use ESP-IDF v3.2 from either tagged release or command,
```
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout -b release/v3.2 origin/release/v3.2
export IDF_PATH=</path/to/esp-idf/>
```

## Overview

As already mentioned, this Project contains tutorials wherein features are introduced incrementally.
The step-by-step guidance will help building "Fully Functional" IoT Products using ESP-IDF software
framework on ESP32.
Reference product considered here is `power outlet` or `smart switch`. The steps are as below

## 1_hello_world
Getting started point for setting up development environment using ESP-IDF and simple
multi-threaded application periodically showing `hello world` string on UART/console

## 2_drivers
Interface LED module to ESP32 DevKitC and control/toggle using a Button/GPIO. This will be the
primary hardware linked product logic.

## 3_wifi_connection
Connect device to Wi-Fi network with static or build time credentials.

## 4_network_config
Real world on-boarding of device to Wi-Fi network with secure and dynamic configuration
over multiple transports like BLE or SoftAP interface. Ability to reconfigure device
with reset-to-factory mode based on long press of a button.

## 5_cloud
Securely connect device to the "Internet" by introducing cloud connectivity.
AWS IoT has been used as a reference.

## 6_ota
Device firmware over-the-air upgrades for pushing latest and greatest software.

## 7_mfg
Separation of configuration and firmware partitions for seamless manufacturing or factory
programming.
