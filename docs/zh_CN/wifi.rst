Wi-Fi 连接
================

:link_to_translation:`en:[English]`

在本章中，我们要把这个电源插座连接到 Wi-Fi 网络。此 Wi-Fi 网络信息已嵌入到设备固件中，源代码可在 *3\_wifi\_connection/* 目录中查看。

代码
--------

.. code:: c

    #include <esp_wifi.h>
    #include <esp_event.h>
    #include <esp_idf_version.h>

    #if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(4, 1, 0)
    #define ESP_NETIF_SUPPORTED
    #endif

    #ifdef ESP_NETIF_SUPPORTED
    #include <esp_netif.h>
    #else
    #include <tcpip_adapter.h>
    #endif

    /* 初始化 TCP/IP */
    #ifdef ESP_NETIF_SUPPORTED
    esp_netif_init();
    #else
    tcpip_adapter_init();
    #endif

    /* 初始化事件循环 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* 注册 Wi-Fi 和 IP 相关事件的事件处理程序 */
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));

    #ifdef ESP_NETIF_SUPPORTED
    esp_netif_create_default_wifi_sta();
    #endif

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

-  我们使用版本相关的调用来初始化 TCP/IP 堆栈。现代的 ESP-IDF（v4.1+）使用 *esp\_netif\_init()*，而旧版本使用 *tcpip\_adapter\_init()*；

-  我们使用 *esp\_event\_loop\_create\_default()* 初始化默认事件循环；

-  我们使用 *esp\_event\_handler\_register()* 注册 Wi-Fi 和 IP 事件的事件处理程序。这替代了旧的单一事件处理程序方法；

-  我们使用 *esp\_netif\_create\_default\_wifi\_sta()* 创建默认的 Wi-Fi station netif（适用于 ESP-IDF v4.1+）；

-  调用 *esp\_wifi\_init()* 和 *esp\_wifi\_set\_mode()* 来初始化 Wi-Fi 子系统及其 station 接口；

-  最后，使用嵌入的 SSID 和密码配置 Wi-Fi 网络，调用 *esp\_wifi\_start()* 启动 station 接口。

Wi-Fi 协议栈会发出断开连接、建立连接、获取到 IP 地址等异步事件。事件循环 (event loop) 从 TCP/IP 堆栈和 Wi-Fi 子系统收集事件。现代的 ESP-IDF 使用默认事件循环，必须通过 *esp\_event\_loop\_create\_default()* 创建。事件被传递给为特定事件基础和 ID 注册的处理程序。

异步事件处理程序在 Event Loop 注册，其实现方式如下：

.. code:: c

    static void event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
    {
        if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
            ESP_LOGI(TAG, "Connected with IP Address:" IPSTR, IP2STR(&event->ip_info.ip));
        } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
            ESP_LOGI(TAG, "Disconnected. Connecting to the AP again...");
            esp_wifi_connect();
        }
    }

事件处理程序当前处理 3 个事件，当接收到 *WIFI\_EVENT\_STA\_START* 事件后，要求 station 接口调用 *esp\_wifi\_connect()* 进行网络连接。收到 Wi-Fi 断开事件（*WIFI\_EVENT\_STA\_DISCONNECTED*），也会要求 station 接口调用 *esp\_wifi\_connect()* 重新进行网络连接。

ESP32 接收到获取 IP 地址即相当于 *IP\_EVENT\_STA\_GOT\_IP* 事件发生，在这种情况下，我们只在控制台打印 IP 地址。请注意，在现代的 ESP-IDF 中，事件处理程序接收结构化的事件数据而不是单一的事件结构。

未完待续
---------------

现在您可以修改应用程序，输入 Wi-Fi 网络的 SSID 和密码。如果您已将该代码编译并烧录至开发板，ESP32 将连接到您设置的 Wi-Fi 网络，并在控制台上打印 IP 地址。当然，我们还保留了插座按下按钮触发 GPIO 的功能。

但这种方法有个弊端：Wi-Fi 配置被写死到了固件中。虽然这种方法对业余开发项目而言没有问题，但是如果用于量产，终端用户则希望自定义配置设备。这就是我们下一章要讨论的问题。
