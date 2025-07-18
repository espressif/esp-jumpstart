> If you have already been using ESP Jumpstart, please have a look at some breaking [Changes](CHANGES.md).

# ESP-Jumpstart

[[中文]](./README_cn.md)

| [![Cover Page]](https://docs.espressif.com/projects/esp-jumpstart/en/latest/esp32/index.html) |
|:--------------------------:|
| **ESP-Jumpstart: Build ESP32 Products Fast** |

Building production-ready firmware can be hard. It involves multiple questions and decisions about the best ways of doing things. It involves building phone applications, and integrating cloud agents to get all the features done. What if there was a ready reference, a known set of best steps, gathered from previous experience of others, that you could jumpstart with?

ESP-Jumpstart is focused on building ’products’ on ESP32. It is a quick-way to get started into your product development process. ESP-Jumpstart builds a fully functional, ready to deploy “Smart Power Outlet” in a sequence of incremental tutorial steps. Each step addresses either a user-workflow or a developer workflow. Each step is an application built with ESP-IDF, ESP32’s software development framework.

![Smart Power Outlet]

The ESP-Jumpstart’s Smart Power Outlet firmware assumes the device has one input push-button, and one GPIO output. It implements the following commonly required functionality.

-   Allows user’s home Wi-Fi network configuration through phone applications (iOS/Android)
-   Ability to switch on or off a single GPIO output
-   Use a push-button to physically toggle this output
-   Allow remote control of this output through a cloud
-   Implements over-the-air (OTA) firmware upgrade
-   Performs *Reset to Factory* settings on long-press of the push-button

Building your production firmware, is a matter of replacing the power-outlet’s device driver, with your device driver (bulb, washing machine).

![Jumpstart Applicability]

You will require one of the following development kits to get started:

-   **ESP32 Series Development Kit**: ESP-Jumpstart supports multiple ESP32 variants. Choose a development kit based on your target chip:
    -   **ESP32**: [ESP32-DevKitC](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/index.html) - Wi-Fi + Bluetooth Classic + BLE
    -   **ESP32-S2**: [ESP32-S2-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s2/esp32-s2-devkitc-1/index.html) - Wi-Fi only
    -   **ESP32-S3**: [ESP32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/index.html) - Wi-Fi + BLE
    -   **ESP32-C2**: [ESP8684-DevKitC-02](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c2/esp8684-devkitc-02/user_guide.html) - Wi-Fi + BLE
    -   **ESP32-C3**: [ESP32-C3-DevKitC-02](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitc-02/index.html) - Wi-Fi + BLE
    -   **ESP32-C6**: [ESP32-C6-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html) - Wi-Fi + BLE

    All development kits are available through [Espressif's distributors](https://www.espressif.com/en/contact-us/sales-questions). You can also use any other ESP32 series development board if you already have one.
-   Follow the step-by-step guide at [ESP-Jumpstart Getting Started](https://docs.espressif.com/projects/esp-jumpstart/en/latest/esp32/index.html).

Please ensure to use ESP-IDF v5.1 or above from either the tagged release or a command like the following:

```
git clone https://github.com/espressif/esp-jumpstart.git
git clone -b release/v5.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-jumpstart
export IDF_PATH=</path/to/esp-idf/>
```

  [Smart Power Outlet]: docs/_static/jumpstart-outlet.png
  [Jumpstart Applicability]: docs/_static/jumpstart-outlet-blocks.png
  [Cover Page]: docs/_static/cover_page.svg