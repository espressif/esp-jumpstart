> 如果您已经在使用 ESP Jumpstart，请查看一些重大[变更](CHANGES.md)。

# ESP-Jumpstart

[[English]](./README.md)

| [![Cover Page]](https://docs.espressif.com/projects/esp-jumpstart/zh_CN/latest/esp32/index.html) |
|:--------------------------:|
| **ESP-Jumpstart：快速构建基于 ESP32 的产品** |

构建生产就绪的固件可能很困难。它涉及多个问题和关于最佳实践方式的决策。它涉及构建手机应用程序，并集成云代理来完成所有功能。如果有一个现成的参考，一套已知的最佳步骤，从其他人的先前经验中收集，您可以快速启动呢？

ESP-Jumpstart 专注于在 ESP32 上构建"产品"。这是快速进入产品开发过程的方法。ESP-Jumpstart 通过一系列增量教程步骤构建了一个功能齐全、随时可部署的"智能电源插座"。每个步骤都解决用户工作流或开发人员工作流。每个步骤都是使用 ESP32 软件开发框架 ESP-IDF 构建的应用程序。

![Smart Power Outlet]

ESP-Jumpstart 的智能电源插座固件假设设备有一个输入按钮和一个 GPIO 输出。它实现了以下常用功能：

-   允许用户通过手机应用程序（iOS/Android）配置家庭 Wi-Fi 网络
-   能够打开或关闭单个 GPIO 输出
-   使用按钮物理切换此输出
-   允许通过云端远程控制此输出
-   实现空中（OTA）固件升级
-   长按按钮执行*恢复出厂设置*

构建您的生产固件，就是用您的设备驱动程序（灯泡、洗衣机）替换电源插座的设备驱动程序。

![Jumpstart Applicability]

您需要以下开发套件之一来开始：

-   **ESP32 系列开发套件**：ESP-Jumpstart 支持多种 ESP32 变体。根据您的目标芯片选择开发套件：
    -   **ESP32**：[ESP32-DevKitC](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32/esp32-devkitc/index.html) - Wi-Fi + 蓝牙经典版 + BLE
    -   **ESP32-S2**：[ESP32-S2-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s2/esp32-s2-devkitc-1/index.html) - 仅 Wi-Fi
    -   **ESP32-S3**：[ESP32-S3-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/esp32-s3-devkitc-1/index.html) - Wi-Fi + BLE
    -   **ESP32-C2**：[ESP8684-DevKitC-02](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c2/esp8684-devkitc-02/user_guide.html) - Wi-Fi + BLE
    -   **ESP32-C3**：[ESP32-C3-DevKitC-02](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c3/esp32-c3-devkitc-02/index.html) - Wi-Fi + BLE
    -   **ESP32-C6**：[ESP32-C6-DevKitC-1](https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32c6/esp32-c6-devkitc-1/index.html) - Wi-Fi + BLE

    所有开发套件都可通过[乐鑫分销商](https://www.espressif.com/en/contact-us/sales-questions)获得。如果您已经有其他 ESP32 系列开发板，也可以使用。
-   按照 [ESP-Jumpstart 快速入门](https://docs.espressif.com/projects/esp-jumpstart/zh_CN/latest/esp32/index.html) 的分步指南操作。

请确保使用 ESP-IDF v5.1 或更高版本，可以从标记的发布版本或使用以下命令：

```
git clone https://github.com/espressif/esp-jumpstart.git
git clone -b release/v5.4 --recursive https://github.com/espressif/esp-idf.git
cd esp-jumpstart
export IDF_PATH=</path/to/esp-idf/>
```

[Smart Power Outlet]: docs/_static/jumpstart-outlet.png
[Jumpstart Applicability]: docs/_static/jumpstart-outlet-blocks.png
[Cover Page]: docs/_static/cover_page.svg
