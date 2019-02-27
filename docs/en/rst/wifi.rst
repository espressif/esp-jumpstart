Wi-Fi Connection
================

Let’s now get this power outlet on a Wi-Fi network. In this Chapter we
will connect to a hard-coded Wi-Fi network that is embedded within the
device’s firmware executable image. You may refer to the
*3\_wifi\_connection/* directory of esp-jumpstart for looking at this
code.

The Code
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

In the above code:

-  We initialize the TCP/IP stack with the *tcpip\_adapter\_init()* call

-  Similarly, the Wi-Fi subsystem and its station interface is
   initialized with the calls to *esp\_wifi\_init()* and
   *esp\_wifi\_set\_mode()*

-  Finally, the hard-coded SSID and passphrase configuration of the
   target Wi-Fi network are configured and we start the station using a
   call to *esp\_wifi\_start()*

Wi-Fi is a protocol that can generate asynchronous events like
connectivity lost, connection established, DHCP Address received etc.
The event loop collects events from the TCP/IP Stack and the Wi-Fi
subsystem. The call to *esp\_event\_loop\_init()* initialises the event
loop. The event loop delivers these events to the callback that is
registered through the first parameter.

The asynchronous event handler that is registered with the event loop
can be implemented as:

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

The event handler current handles 3 events. When it receives an event
*SYSTEM\_EVENT\_STA\_START*, it asks the station interface to connect
using the *esp\_wifi\_connect()* call. The same action is taken even
when we receive a Wi-Fi disconnect event.

The event *SYSTEM\_EVENT\_STA\_GOT\_IP* is received when a DHCP IP
address is obtained by ESP32. In this particular case, we only print the
IP address on the console.

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
