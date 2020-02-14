SoftAP 配网和 BLE 配网
============================

:link_to_translation:`en:[English]`

在上个示例中，我们把 Wi-Fi 信息（SSID 和 PASSWORD）直接嵌入到了固件中，但这显然不适用于终端产品。

因此在这一章节，我们将构建一个适用于终端用户的固件，允许用户在设备运行时，将其 Wi-Fi 信息配置到设备中。由于用户的网络信息将永久储存在设备中，所以我们另外提供了 *恢复出厂设置* 功能，可擦除设备中储存的用户配置信息。如需查看相关代码，请前往 esp-jumpstart 项下的 *4\_network\_config/* 目录。

概述
--------

如下图所示，在配网阶段，终端用户通常使用智能手机将 Wi-Fi 信息安全地配置到你的设备中。设备获取 Wi-Fi 信息后，就会连接到终端用户的家庭 Wi-Fi 网络中。

.. figure:: ../../_static/network_config.png
   :alt: Network Configuration Process

   配网过程

设备可通过多种途径接收 Wi-Fi 信息。ESP-Jumpstart 支持以下方式：

-  SoftAP

-  低功耗蓝牙 (BLE)

这两种方式各有千秋，一些开发人员会选择这种，有些开发人员则会选择那种，这主要取决于自己的侧重点。

SoftAP 配网
~~~~~~~~~~~~~~

在 SoftAP 模式下，插座会充当临时的 Wi-Fi 接入点。之后，用户可将智能手机连接到这个临时 Wi-Fi 网络中。连接创建完成后，即可将用户的家庭 Wi-Fi 信息传送到插座。当今市场上的多数连网设备均使用这一模式，在这种配网过程中，用户需要：

-  将手机的 Wi-Fi 网络切换到插座创建的临时 Wi-Fi 网络；

-  使用你所提供的手机 app；

-  输入家庭 Wi-Fi 信息，并将该信息通过 SoftAP 连接传输到插座。 

这种模式一开始就要求客户手动切换手机连接的 Wi-Fi 网络，这可能会让部分用户感到困惑，因此用户体验并不友好。而且，我们通常也很难直接将这个“切换网络”的过程写入代码，交由 app 自动完成（全部 iOS 和部分 Android 版本均并不允许手机 app 进行上述操作）。不过，这种模式的优势在于：第一，非常可靠（SoftAP 只作为 Wi-Fi 接入点，这已经是很成熟的技术了）；第二在于简洁（无需在设备固件中增加额外代码）。

BLE 配网
~~~~~~~~~~

在 BLE 配网模式下，插座会首先进行 BLE 广播，而后附近的手机会收到该广播，并询问手机用户是否与该插座进行 BLE 连接。如选择创建 BLE 连接，手机即可将网络信息传输到插座。在这种配网过程中，用户无需切换 Wi-Fi，而且 iOS 和
Android 系统都支持手机 app 扫描并连接到周围的 BLE 设备。这样就可以大大提升终端用户的体验。

但是，使用 BLE 进行配网时，有一个缺点：此过程需要加入蓝牙相关代码。这就意味着你的固件大小会增加，因而会对 flash 提出更高要求。
此外，在这种配网模式下，BLE 在配网结束前还会一直占用内存。

演示
------

在详细介绍网络配置流程之前，让我们先了解一下终端用户会如何使用我们提供的 APP 进行网络配置。请前往 esp-jumpstart 项下 *4\_network\_config/* 目录，查看详情：

-  进入 *4\_network\_config* 目录；

-  构建、烧录并载入上面的程序；

-  固件默认以 BLE 配网模式启动；

-  点击 `应用下载页面 <https://github.com/espressif/esp-idf-provisioning-android/releases>`_，下载配套的手机 app，进行网络配置。请安装名为 **ble-sec1** 的最新版 app；

-  启动该 app，并按照 app 提供的操作向导进行操作；

-  如果一切顺利，设备将连接到家庭 Wi-Fi 网络；

-  如果此时重启设备，设备将不会进入网络配置模式，而是去连接已配置的 Wi-Fi 网络。这就是我们想要的终端产品体验。

.. _sec_for\_esp8266\_users:

ESP8266 用户
~~~~~~~~~~~~~~~~~

ESP8266 不具备蓝牙功能，因此现阶段仅支持 SoftAP 配网模式。ESP8266 用户请使用 **wifi-sec1** app 进行配置。


.. _sec_unified\_prov:

统一配置
--------------------

乐鑫提供一种 **统一配置** 模块，协助进行网络配置。当从可执行固件调用此模块时，模块将负责管理所有状态转换（如启动/停止 softAP/BLE 接口，安全交换网络信息，储存网络信息以备后续之用等）。统一配置模块具备以下优势：

-  协议可扩展：协议灵活，开发人员可在配置阶段发送自定义配置，数据含义由应用程序决定。

-  传输灵活：此协议作为一种传输协议，既支持 Wi-Fi （SoftAP + HTTP
   服务器），又支持 BLE。该统一配置框架还可以轻松增加对其他传输形式的支持（该传输形式必须支持“命令-响应”行为）。

-  安全方案灵活：在配网阶段，为确保数据传输安全，每个用例对安全方案的需求可能不一样。有些应用需要使用 WPA2 加密的 SoftAP 传输方式或 "just-works" 加密的 BLE 传输模式；而有些应用为了确保数据传输安全，会选择应用级别安全机制。统一配置框架允许应用程序选择自己合适的安全模式。

-  紧凑的数据表示：本协议使用谷歌 Protocol
   Buffers 作为数据表示方式，用于会话设置和 Wi-Fi 配网。谷歌 Protocol
   Buffers 数据表示紧凑，并能够以原生格式解析多种编程语言中的数据。注意，这种数据表示并不专门用于应用程序特有的数据，开发人员可以选择自己的数据表示方式。

配网底层结构包含以下组件：

-  **统一配置规范**：用于将 Wi-Fi 信息安全地传输到设备，与传输模式（SoftAP 或 BLE 模式）无关。请参考 `统一配置相关文档 <https://docs.espressif.com/projects/esp-idf/en/release-v4.0/api-reference/provisioning/provisioning.html>`_，查看详细信息。

-  **IDF 组件**：在设备固件中实现此规范的软件模块，可通过 ESP-IDF 获取。

-  **手机库**：在 iOS 和 Android 系统中的参考实现，可以直接集成到现有的手机应用程序中。

-  **手机应用程序参考**：乐鑫提供功能齐全的 `Android <https://github.com/espressif/esp-idf-provisioning-android>`_ 和 `iOS <https://github.com/espressif/esp-idf-provisioning-ios>`_ 手机应用程序，可在开发时用于测试或排除你的手机品牌影响。

代码
~~~~~~~~

通过固件调用统一配置的代码如下所示：

.. code:: c


    if (conn_mgr_prov_is_provisioned(&provisioned) != ESP_OK) {
        return;
    }

    if (provisioned != true) {
        /* Starting unified provisioning */
        conn_mgr_prov_start_provisioning(prov_type,
                   security, pop, service_name, service_key);
    } else {
        /* Start the station */
        wifi_init_sta();
    }

*conn\_mgr\_prov* 组件是在统一配置接口上的一层封装，请注意：

-  *conn\_mgr\_prov\_is\_provisionined()* API 用于检查 Wi-Fi 网络信息是否已经配置。网络信息通常储存在名为 *NVS* 的 flash 分区内，本章节后续部分会详细介绍 NVS（Non-volatile storage 非易失性存储器 ）。 

-  如果没有可用的 Wi-Fi 网络信息，固件将使用 *conn\_mgr\_prov\_start\_provisioning()* API 启动统一配置。此 API 可以处理以下任务：

   #. 按照配置启动 SoftAP 或 BLE 传输；

   #. 使用 Wi-Fi 或 BLE 标准进行必要的广播；

   #. 安全接收手机应用程序传输过来的任意网络信息；

   #. 将上述网络信息储存在 NVS 中，以备后续之用；

   #. 最后，还可以对统一配置所需的所有组件（SoftAP、BLE、HTTP 等）撤销初始化，确保配网结束后，统一配置模块几乎不占用内存。

-  如果在 NVS 中发现 Wi-Fi 配网信息，即可使用 *wifi\_init\_sta()* API 直接启动 Wi-Fi station 接口。

上述步骤确保了在没有发现任何配网信息后，固件即可启用统一配置模块，如果有可用的配网信息，则会启动 Wi-Fi station 接口。

统一配置模块还需知道 Wi-Fi 接口的状态转换情况。因此，需要事件处理程序 (event handler) 发出调用请求，来处理这一问题：

.. code:: c

    esp_err_t event_handler(void *ctx, system_event_t *event)
    {
         conn_mgr_prov_event_handler(ctx, event);
       
         switch(event->event_id) {
         case SYSTEM_EVENT_STA_START:
    ...
    ...
    ...

配置选项
^^^^^^^^^^^^^^^^^^^^

在上述代码中，我们用到了下面的 API 来调用统一配置接口：

.. code:: c

        /* Starting unified provisioning */
        conn_mgr_prov_start_provisioning(prov_type,
                   security, pop, service_name, service_key);

该 API 用到的参数，即该 API 的配置选项如下：

#. **安全性 (Security)**：统一配置模块当前支持两种用于传输网络信息的安全模式：*security0* 模式和
   *security1* 模式。Security0 交换网络信息时，未采用任何安全措施，主要用于开发目的。Security1 使用椭圆曲线 *curve25519* 对密钥交换进行加密，然后使用 *AES-CTR* 对信道上交换的数据进行加密。

#. **传输机制 (Transport)**：开发人员可自主选择传输机制，用于网络配置。可供选择的传输机制有：SoftAP 和 BLE。

   -  此模块编写方式特殊，可根据开发人员的选择，仅将相关软件库添加到最终可执行映像。

   -  统一配置模块同时还将管理配网所需的状态转换和其他服务。

#. **所有权证明 (Proof of Possession)**：当用户拿来一个新的智能设备时，该设备将启动其配网功能（BLE 或 SoftAP）进行网络配置。如何才能确保只有设备所有者才能对该设备进行配置？而不是周围邻居也能轻易进行配置呢？此配置选项就是为了解决这个问题的。有关此选项的详细信息，请阅读以下小节。

#. **服务名称 (Service Name)**：用户在启动配网 app 后，将看到周围未配置设备的列表。这里的 **服务名称** 就是客户将在列表中看到的你的设备的名称。因此，你在为设备命名时应注意取一个便于用户理解区别的名字，比如 “某某恒温器”。通常而言，您的 **服务名称** 中应能够部分体现所提供服务（通过唯一或随机元素），方便用户在配网时轻松找到需要配置的设备。当采用 SoftAP 配网时，**服务名称** 显示为临时 Wi-Fi 接入点的 SSID；当采用 BLE 配网时，服务名称则显示为 BLE 设备的名称。 

#. **服务秘钥 (Service Key)**：服务秘钥为可选参数，可以用作密码，防止未经授权的用户访问传输数据。该参数一般仅在 SoftAP 配网模式下使用，可用于为临时 Wi-Fi 接入点提供密码保护。当采用 BLE 配网时，BLE 会使用 "just-works" 配对方式，此选项可忽略。

所有权证明
^^^^^^^^^^^^^^^^^^^

当用户拿到一个新的智能设备，并启动设备进行配网时（无论是 SoftAP 配网或 BLE 配网），如何确保仅有设备所有者才能对该设备进行配置，而不是随便一个附近的邻居呢？

为了实现这个目的，有些产品会要求用户完成某些设置，以证明自己是此设备的所有者，比如要求用户对设备进行一些物理操作，比如按下某个按键；或输入设备包装盒或显示屏（如果配备显示屏的情况下）上的某些特有随机秘钥等。

在生产过程中，每个设备均将具备一个唯一的随机秘钥。接着，该秘钥将被提供给统一配网模块，用于所有权证明过程。之后，用户在使用手机 app 对设备进行配置时，app 会将该所有权证明传送给设备，然后由统一配网模块验证所有权证明是否匹配，进而确认是否进行配网操作。

补充信息
~~~~~~~~~~~~~~~~~~

请参考 `统一配置相关文档 <https://docs.espressif.com/projects/esp-idf/en/release-v4.0/api-reference/provisioning/provisioning.html>`_，查看详细信息。

.. _sec_nvs\_info:

NVS：永久储存键值对
-------------------------------

在上文有关“统一配置模块”的介绍中，我们曾介绍说数据传输时的 Wi-Fi 网络信息是储存在 NVS 中的。NVS 是一种软件组件，用于永久储存键值对。由于 NVS 存储是永久性的，因此即便设备重启或断电，这些信息也不会丢失。NVS 在 flash 中有一个专门的分区来储存这些信息。 

NVS 经过专门设计，不但可以防止设备断电带来的数据损坏影响，而且还可以通过将写入的内容分布到整个 NVS 分中以处理 flash 磨损的问题。

开发人员还可以使用 NVS 储存任何你希望与应用程序固件一起维护的数据，比如产品的用户配置信息。NVS 支持存储多种数据类型，比如整型、以 NULL 结尾的字符串和二进制大对象（BLOB）等。此外，NVS 的操作简便，仅通过以下两个 API 即可完成读写操作。 

.. code:: c

      /* Store the value of key 'my_key' to NVS */
      nvs_set_u32(nvs_handle, "my_key", chosen_value);

      /* Read the value of key 'my_key' from NVS */
      nvs_get_u32(nvs_handle, "my_key", &chosen_value);

补充信息
~~~~~~~~~~~~~~~~~~

请参考 `NVS 相关文档 <https://docs.espressif.com/projects/esp-idf/en/release-v4.0/api-reference/storage/nvs_flash.html>`_，查看详细信息.

恢复出厂设置
----------------

*恢复出厂设置* 是产品另一个常见功能。如上述所述，只要将用户配置储存到 NVS 后，后续只需擦除 NVS 分区内的信息即可将设备恢复为出厂设置。通常而言，长按设备上的某个按钮即可恢复出厂设置。配置按钮功能也很简单，通过 *iot\_button\_()* 函数即可实现。 

.. _sec_reset\_to\_factory:

代码
~~~~~~~~

在 *4\_network\_config/* 应用程序中，我们同样通过长按按钮动作来恢复出厂设置。

.. code:: c

    /* Register 3 second press callback */  
    iot_button_add_on_press_cb(btn_handle, 3, button_press_3sec_cb, NULL);

具体实现过程为：一旦与 *btn\_handle* 关联的按钮被按下超过 3 秒，就会回调 *button\_press\_3sec\_cb()* 函数。请注意，我们在 :ref:`sec_push\_button` 章节中对 *btn\_handle* 进行了初始化。

回调函数示例如下：

.. code:: c

    static void button_press_3sec_cb(void *arg)
    {
        nvs_flash_erase();
        esp_restart();
    }

这段代码的作用是擦除 NVS 的所有内容，然后触发设备重启。由于 NVS 内容已被清除，设备下次启动时将回到未配置状态。

这里，如果你已经通过 *4\_network\_config/* 加载并配置了你的设备，则可以尝试长按（3 秒以上）相关按钮，亲自查看恢复出厂设置的整个过程。

未完待续
---------------

截止目前，我们已经拥有了这样一款允许用户通过手机 app 连入家庭 Wi-Fi 网络的智能插座。一旦完成配置，该智能插座将总是尝试连接这个 Wi-Fi 网络。当然了，我们也可以通过长按按钮擦除现有配网信息，恢复出厂设置。

然而，到目前为止，插座自身功能与连网功能还是分开的。下一步，我们会将这两个功能结合起来，实现远程控制与监控插座状态，即打开/关闭。


