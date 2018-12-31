# ESP-Jumpstart

## Overview

Repository contains incremental applications which provides step-by-step guidance
to build `fully functional` IoT product using ESP-IDF software framework on ESP32.
Reference product considered here is `power outlet` or `smart switch`.

## 1hello_world
Getting started point for setting up development environment using ESP-IDF and simple
multi-threaded application showing periodically `hello world` string on UART/console

## 2outlet
Interface LED module to ESP32 DevKitC and control/toggle using GPIO. This will be real
world `load` for `power outlet`

## 3wifi_connection
Lets connect device to WiFi network with static or build time credentials

## 4network_config
Real world on-boarding of device to WiFi network with secure and dynamic configuration
over multiple transports like BLE or SoftAP interface. Ability to reconfigure device
with reset-to-factory mode based on long press of switch

## 5cloud
Lets on-board device to cloud in secure manner, reference cloud is AWS IoT

## 6ota
Device firmware over-the-air upgrades for pushing latest and greatest software

## 7mfg
Separation of configuration and firmware partitions for seamless manufacturing or factory
programming
