ESP-Jumpstart
=============
[[中文]](./README_cn.md)

| ![Cover Page] |
|:--------------------------:|
| **ESP-Jumpstart: Build ESP8266 Products Fast** |


Building production-ready firmware can be hard. It involves multiple questions and decisions about the best ways of doing things. It involves building phone applications, and integrating cloud agents to get all the features done. What if there was a ready reference, a known set of best steps, gathered from previous experience of others, that you could jumpstart with?

ESP-Jumpstart is focused on building ’products’ on ESP8266. It is a quick-way to get started into your product development process. ESP-Jumpstart builds a fully functional, ready to deploy “Smart Power Outlet” in a sequence of incremental tutorial steps. Each step addresses either a user-workflow or a developer workflow. Each step is an application built with ESP8266_RTOS_SDK, ESP8266’s software development framework.

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

You will require the following to get started:

-   An ESP8266 development kit: ESP8266 Devkit-C (Available on [DigiKey](https://www.digikey.in/product-detail/en/espressif-systems/ESP8266-DEVKITC-02D-F/1965-1001-ND/9649768), [Mouser](https://www.mouser.in/ProductDetail/Espressif-Systems/ESP8266-DevKitC-02D-F?qs=qSfuJ%252Bfl%2Fd64058n5BJabA%3D%3D). You could also use any other ESP8266 development boards if you already have one.)
-   A Development setup (<https://github.com/espressif/ESP8266_RTOS_SDK>)
Please ensure to use ESP8266_RTOS_SDK using the following command,
```
git clone https://github.com/espressif/esp-jumpstart.git
git clone https://github.com/espressif/ESP8266_RTOS_SDK.git
cd ESP8266_RTOS_SDK
git checkout -b release/jumpstart 93e3a3f5424e76def8afb3c41e625471490c056b
cd ../esp-jumpstart
git checkout -b platform/esp8266 origin/platform/esp8266
export IDF_PATH=</path/to/ESP8266_RTOS_SDK/>
```


  [Smart Power Outlet]: docs/_static/jumpstart-outlet.png
  [Jumpstart Applicability]: docs/_static/jumpstart-outlet-blocks.png
  [Cover Page]: docs/_static/cover_page.svg


