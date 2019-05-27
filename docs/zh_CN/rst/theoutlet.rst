驱动程序
=============

:link_to_translation:`en:[English]`

在本章中，我们将使用 ESP32 驱动程序 API 创建一个简单的电源插座应用。该电源插座：

-  包含一个实体按钮

-  按下按钮即可打开/关闭 GPIO 输出端

本章节只关注实现插座自身的功能，后续章节会具体介绍插座的连网功能。请参考 esp-jumpstart 项下的 *2\_drivers/* 目录，查看相关代码。 

此驱动程序的代码已被单独放入 *app\_driver.c* 文件，因此如果后续需要修改此应用程序用于产品，您只需更改此文件内容，即可与外设通信。

实体按钮
---------------

首先，我们需要创建一个实体按钮。在 DevkitC 开发板上设有一个名为 Boot 的按钮，并连接至 GPIO 0。我们将配置此按钮，用于打开/关闭插座。

.. _sec_push\_button:

代码
~~~~~~~~

实现此功能的代码如下：

.. code:: c

    #include <iot_button.h>

    button_handle_t btn_handle=iot_button_create(JUMPSTART_BOARD_BUTTON_GPIO,
                                    JUMPSTART_BOARD_BUTTON_ACTIVE_LEVEL);
    iot_button_set_evt_cb(btn_handle, BUTTON_CB_RELEASE,
                                push_btn_cb, "RELEASE");

我们使用 *iot\_button* 模块来实现按钮功能。首先，创建 iot\_button 对象，指定 GPIO 输出端及其有效电平用于检测按钮动作。DevKitC 开发板的 *BUTTON\_GPIO* 设置为 GPIO 0。  

然后我们为按钮注册事件回调函数，松开按钮时，就会在 esp-timer 线程中调用 *push\_btn\_cb* 函数。请确保为 esp-timer 线程配置的默认堆栈足以满足回调函数需求。

*push\_btn\_cb* 代码如下：

.. code:: c

    static void push_btn_cb(void* arg)
    {
        static uint64_t previous;
        uint64_t current = xTaskGetTickCount();
        if ((current - previous) > DEBOUNCE_TIME) {
            previous = current;
            app_driver_set_state(!g_output_state);
        }
    }

*xTaskGetTickCount()* 属于 FreeRTOS 函数，提供当前节拍事件计数。在回调函数中，需要确保按钮操作短时间内不会生成多个事件，否则会影响终端用户体验。在本示例中，所有 300 毫秒内生成的事件均视为一次有效事件。最后，调用 *app\_driver\_toggle\_state()* 函数，用于打开或关闭输出端。

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

在本示例中，选择 GPIO 27 用作输出端。使用上述设置初始化 *gpio\_config\_t* 结构，将其设置为 GPIO 输出端，内部上拉。

.. code:: c

    /* Assert GPIO */
    gpio_set_level(JUMPSTART_BOARD_OUTPUT_GPIO, target);

最后，使用 *gpio\_set\_level()* 设置 GPIO 状态。

未完待续
---------------

现在，我们已经实现了电源插座本身的插座功能。将此固件构建并烧录至设备后，用户每次按下按钮，ESP32 就会打开或关闭输出端。当然，目前该插座还无法连网。

下一步，我们将为固件增加 Wi-Fi 连接功能。
