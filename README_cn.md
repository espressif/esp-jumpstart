ESP-Jumpstart
=============
[[English]](./README.md)

| ![Cover Page] |
|:--------------------------:|
| **ESP-Jumpstart：快速构建基于 ESP8266 的产品** |


有时，固件开发并非易事，特别是用于量产的固件。开发人员不但需要面临各种决策上的问题并权衡多种选择，而且还需开发手机 app，并接入各家云服务商。现在，我们盛情推出 ESP-Jumpstart 示例项目，内含产品开发的完整步骤、最佳做法，并融合其他同僚的经验之谈，助您快速启动基于 ESP8266 的产品开发！

ESP-Jumpstart 项目专注于在 ESP8266 上构建“产品”，展示了基于 ESP8266 的完整产品开发流程（想了解基于 ESP32 的产品开发？请查看 [基于 ESP32 的 ESP-Jumpstart](https://docs.espressif.com/projects/esp-jumpstart/en/latest/index.html)）。该项目分步介绍了一款真实产品的完整开发流程，即一款功能齐全、随时可推广的“智能电源插座”。其中，每个步骤均为用户/开发人员的工作流提供指南/参考，且使用了乐鑫专为 ESP8266 打造的软件开发框架 `ESP8266_RTOS_SDK`。

![Smart Power Outlet]

本项目中的“智能电源插座”硬件拥有一个实体按钮和一个 GPIO 输出端，可实现以下常见功能：

- 允许用户在家庭 Wi-Fi 网络环境中，通过手机 app（苹果/安卓）进行配置；
- 允许通过手机 app，打开/关闭 GPIO 输出端；
- 允许通过实体按钮，打开/关闭 GPIO 输出端；
- 允许通过云端，远程打开/关闭 GPIO 输出端；
- 支持 OTA 固件升级；
- 长按实体按钮，恢复出厂设置。

在实际开发中，您仅需将本项目中的“智能电源插座”替换为您的设备驱动程序（灯泡、洗衣机）即可。

![Jumpstart Applicability]

准备工作：

- ESP8266 开发板：ESP8266-DevKitC（[DigiKey](https://www.digikey.in/product-detail/en/espressif-systems/ESP8266-DEVKITC-02D-F/1965-1001-ND/9649768)、[Mouser](https://www.mouser.in/ProductDetail/Espressif-Systems/ESP8266-DevKitC-02D-F?qs=qSfuJ%252Bfl%2Fd64058n5BJabA%3D%3D)、[亚马逊](https://www.amazon.com/s?k=esp8266-devkitc&ref=nb_sb_noss)、[淘宝](https://item.taobao.com/item.htm?spm=a1z10.5-c.w4002-8715811636.13.670e47e6YTPftg&id=576438766399)、[微店](https://detail.youzan.com/show/goods?alias=2xk8zvojrotrk) 均有售。您也可以使用其他 ESP8266 开发板。）
- 按照 [ESP-Jumpstart 快速入门指南](https://docs.espressif.com/projects/esp-jumpstart/en/latest/index.html) ，搭建开发环境。

您可以直接使用以下命令，获得 `ESP8266_RTOS_SDK`。

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



