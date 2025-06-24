Wi-Fi Connection
================

:link_to_translation:`zh_CN:[中文]`

Let’s now get this power outlet on a Wi-Fi network. In this Chapter we
will connect to a hard-coded Wi-Fi network that is embedded within the
device’s firmware executable image. You may refer to the
*3\_wifi\_connection/* directory of esp-jumpstart for looking at this
code.

The Code
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

    /* Initialize TCP/IP */
    #ifdef ESP_NETIF_SUPPORTED
    esp_netif_init();
    #else
    tcpip_adapter_init();
    #endif

    /* Initialize the event loop */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register our event handler for Wi-Fi and IP related events */
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

In the above code:

-  We initialize the TCP/IP stack with version-dependent calls. Modern ESP-IDF (v4.1+) uses *esp\_netif\_init()* while older versions use *tcpip\_adapter\_init()*

-  We initialize the default event loop with *esp\_event\_loop\_create\_default()*

-  We register our event handler for Wi-Fi and IP events using *esp\_event\_handler\_register()*. This replaces the old single event handler approach

-  We create the default Wi-Fi station netif with *esp\_netif\_create\_default\_wifi\_sta()* (for ESP-IDF v4.1+)

-  The Wi-Fi subsystem and its station interface is initialized with the calls to *esp\_wifi\_init()* and *esp\_wifi\_set\_mode()*

-  Finally, the hard-coded SSID and passphrase configuration of the target Wi-Fi network are configured and we start the station using a call to *esp\_wifi\_start()*

Wi-Fi is a protocol that can generate asynchronous events like connectivity lost, connection established, DHCP Address received etc. The event loop collects events from the TCP/IP Stack and the Wi-Fi subsystem. The modern ESP-IDF uses a default event loop that must be created with *esp\_event\_loop\_create\_default()*. Events are delivered to registered handlers for specific event bases and IDs.

The asynchronous event handler that is registered with the event loop
can be implemented as:

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

The event handler currently handles 3 events. When it receives an event
*WIFI\_EVENT\_STA\_START*, it asks the station interface to connect
using the *esp\_wifi\_connect()* call. The same action is taken even
when we receive a Wi-Fi disconnect event (*WIFI\_EVENT\_STA\_DISCONNECTED*).

The event *IP\_EVENT\_STA\_GOT\_IP* is received when a DHCP IP
address is obtained by ESP32. In this particular case, we only print the
IP address on the console. Note that in modern ESP-IDF, the event handler
receives structured event data rather than a single event structure.

Progress so far
---------------

You can now modify the application to enter your Wi-Fi network’s SSID
and the passphrase. When you compile and flash this code on your
development board, the ESP32 should connect to your Wi-Fi network and
print the IP address on the console. The outlet’s functionality of
toggling the GPIO on pressing the push-button is, of course, also
retained.

One problem with this approach is that the Wi-Fi settings are hard-coded
into the firmware image. While this is ok for a hobby project, a product
will require the end-user to dynamically configure this device with
their settings. This is what we will look at in the next chapter.
