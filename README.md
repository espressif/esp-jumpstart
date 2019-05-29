> If you have already been using ESP Jumpstart, please have a look at some breaking [Changes](CHANGES.md).

ESP-Jumpstart
=============
[[中文]](./README_cn.md)

| ![Cover Page](https://docs.espressif.com/projects/esp-jumpstart/en/latest/index.html) |
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

You will require the following to get started:

-   An ESP32 development kit: ESP32-DevKitC (Available on [DigiKey](https://www.digikey.com/?_ga=2.24343738.1493252097.1552534720-1988497184.1552534720), [Mouser](https://www.mouser.com/_/?Keyword=ESP32-DevKitC), [Amazon](https://www.amazon.com/s?k=esp32-devkitc&crid=34ZPSBVQ3ZVZK&sprefix=esp32-devt%2Caps%2C738&ref=nb_sb_ss_i_1_9). You could also use any other ESP32 development boards if you already have one.)
-   Follow the step-by-step guide at [ESP-Jumpstart Getting Started](https://docs.espressif.com/projects/esp-jumpstart/en/latest/index.html).

Please ensure to use ESP-IDF v3.2 from either the tagged release or the following command,
```
git clone https://github.com/espressif/esp-jumpstart.git
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout -b release/v3.2 origin/release/v3.2
cd ../esp-jumpstart
export IDF_PATH=</path/to/esp-idf/>
```


  [Smart Power Outlet]: docs/_static/jumpstart-outlet.png
  [Jumpstart Applicability]: docs/_static/jumpstart-outlet-blocks.png
  [Cover Page]: docs/_static/cover_page.svg


