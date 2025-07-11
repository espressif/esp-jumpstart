驱动程序
=============

:link_to_translation:`en:[English]`

在本章中，我们将使用 ESP32 驱动程序 API 创建一个简单的电源插座应用。该电源插座：

-  包含一个实体按钮

-  按下按钮即可打开/关闭 GPIO 输出端

本章节只关注实现插座自身的功能，后续章节会具体介绍插座的连网功能。请参考 esp-jumpstart 项下的 *2_drivers/* 目录，查看相关代码。

此驱动程序的代码已被单独放入 *app_driver.c* 文件，因此如果后续需要修改此应用程序用于产品，您只需更改此文件内容，即可与外设通信。

**组件依赖项**

此示例使用来自 ESP 组件注册表的现代组件：

- ``espressif/button: "^4.1.3"`` - 用于按钮处理
- ``espressif/led_strip: "^3.0.0"`` - 用于 LED 控制

这些依赖项在 ``main/idf_component.yml`` 文件中定义，构建时会自动下载。

实体按钮
---------------

首先，我们需要创建一个实体按钮。在 DevkitC 开发板上设有一个名为 Boot 的按钮，并连接至 GPIO 0。我们将配置此按钮，用于打开/关闭插座。

.. _sec_push_button:

代码
~~~~~~~~

实现此功能的代码如下：

.. code:: c

    #include <iot_button.h>
    #include <button_gpio.h>

    button_config_t btn_cfg = {
        .long_press_time = 1000,
        .short_press_time = 50,
    };

    button_gpio_config_t gpio_cfg = {
        .gpio_num = BUTTON_GPIO,
        .active_level = BUTTON_ACTIVE_LEVEL,
        .enable_power_save = false,
        .disable_pull = false,
    };

    button_handle_t btn_handle;
    esp_err_t ret = iot_button_new_gpio_device(&btn_cfg, &gpio_cfg, &btn_handle);
    if (ret == ESP_OK) {
        iot_button_register_cb(btn_handle, BUTTON_PRESS_UP, NULL, push_btn_cb, "RELEASE");
    }

我们使用来自 ESP 组件注册表的 *button* 组件来实现按钮功能。现代按钮 API 使用两个单独的配置结构：

- *button_config_t* 用于一般按钮设置，如按压时间阈值
- *button_gpio_config_t* 用于 GPIO 特定设置，如引脚号和有效电平

首先，我们使用 *button_config_t* 配置按钮时序，设置长按时间为 1000ms，短按时间为 50ms。然后使用 *button_gpio_config_t* 配置 GPIO 特定设置，指定 GPIO 号和有效电平。

我们使用 *iot_button_new_gpio_device()* 创建按钮设备，然后注册事件回调函数。当按钮被释放（BUTTON_PRESS_UP）时，就会调用 *push_btn_cb* 函数。

*push_btn_cb* 代码如下：

.. code:: c

    static void push_btn_cb(void *button_handle, void *usr_data)
    {
        app_driver_set_state(!g_output_state);
    }

回调函数签名接收按钮句柄和用户数据作为参数。现代按钮组件通过我们之前设置的时序配置（short_press_time = 50ms）内部处理去抖动，因此我们不需要在回调中实现去抖动逻辑。

回调函数简单地调用 *app_driver_set_state()* 函数，用于打开或关闭输出端。

输出端
----------

现在我们将配置 GPIO 作为电源插座的输出端。在理想情况下，打开或关闭此 GPIO 将触发继电器打开或关闭输出端。

.. _sec_relay:

代码
~~~~~~~~

首先，使用以下配置初始化 GPIO。

.. code:: c

    gpio_config_t io_conf;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_up_en = 1;
    io_conf.pin_bit_mask = ((uint64_t)1 << JUMPSTART_BOARD_OUTPUT_GPIO);

    /* Configure the GPIO */
    gpio_config(&io_conf);

在本示例中，选择 GPIO 19 用作输出端。使用上述设置初始化 *gpio_config_t* 结构，将其设置为 GPIO 输出端，内部上拉。

.. code:: c

    /* Assert GPIO */
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_GPIO, target);

最后，使用 *gpio_set_level()* 设置 GPIO 状态。

未完待续
---------------

现在，我们已经实现了电源插座本身的插座功能。将此固件构建并烧录至设备后，用户每次按下按钮，ESP32 就会打开或关闭输出端。当然，目前该插座还无法连网。

下一步，我们将为固件增加 Wi-Fi 连接功能。
