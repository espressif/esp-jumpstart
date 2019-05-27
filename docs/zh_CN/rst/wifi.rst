Wi-Fi 连接
================

:link_to_translation:`en:[English]`

在本章中，我们要把这个电源插座连接到 Wi-Fi 网络。此 Wi-Fi 网络信息已嵌入到设备固件中，源代码可在 *3\_wifi\_connection/* 目录中查看。

代码
--------

.. code:: c

    #include <esp_wifi.h>
    #include <esp_event_loop.h>

    tcpip_adapter_init();
    esp_event_loop_init(event_handler, NULL);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
        },
     };
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    esp_wifi_start();

在上述代码中：

-  我们调用 *tcpip\_adapter\_init()* 来初始化 TCP/IP 堆栈；

-  调用 *esp\_wifi\_init()* 和 *esp\_wifi\_set\_mode()* 来初始化 Wi-Fi 子系统及其 station 接口；

-  最后，使用嵌入的 SSID 和密码配置 Wi-Fi 网络，调用 *esp\_wifi\_start()* 启动 station 接口。 

Wi-Fi 协议栈会发出断开连接、建立连接、获取到 IP 地址等异步事件。事件循环 (event loop) 从 TCP/IP 堆栈和 Wi-Fi 子系统收集事件，调用 *esp\_event\_loop\_init()* 初始化 event loop，event loop 将这些事件传递给通过第一参数注册的回调函数。

异步事件处理程序在 Event Loop 注册，其实现方式如下：

.. code:: c

    esp_err_t event_handler(void *ctx, system_event_t *event)
    {
        switch(event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            ESP_LOGI(TAG, "Connected with IP Address:%s",  
                 ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            break;
        return ESP_OK;
    }

事件处理程序当前处理 3 个事件，当接收到 *SYSTEM\_EVENT\_STA\_START* 事件后，要求 station 接口调用 *esp\_wifi\_connect()* 进行网络连接。收到 Wi-Fi 断开事件，也会要求 station 接口调用 *esp\_wifi\_connect()* 重新进行网络连接。   

ESP32 接收到获取 IP 地址即相当于 *SYSTEM\_EVENT\_STA\_GOT\_IP* 事件发生，在这种情况下，我们只在控制台打印 IP 地址。

未完待续
---------------

现在您可以修改应用程序，输入 Wi-Fi 网络的 SSID 和密码。如果您已将该代码编译并烧录至开发板，ESP32 将连接到您设置的 Wi-Fi 网络，并在控制台上打印 IP 地址。当然，我们还保留了插座按下按钮触发 GPIO 的功能。

但这种方法有个弊端：Wi-Fi 配置被写死到了固件中。虽然这种方法对业余开发项目而言没有问题，但是如果用于量产，终端用户则希望自定义配置设备。这就是我们下一章要讨论的问题。
