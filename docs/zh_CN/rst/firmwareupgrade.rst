固件升级
=================

:link_to_translation:`en:[English]`

在讨论固件升级之前，让我们先了解一下 flash 分区。

.. _sec_flash\_partitions:

Flash 分区
----------------

ESP-IDF 框架将 flash 划分为多个逻辑分区，用于储存各个组件。具体结构如下：

.. figure:: ../../_static/flash_partitions_intro.png
   :alt: Flash Partitions Structure

   Flash 分区结构

从上图可以看出，flash 地址在 0x9000 之前的结构是固定的，第一部分包括二级 Bootloader，后面紧接着就是分区表，分区表则用来储存 flash 剩余区域的分布信息。通常，至少包含 1 个 NVS 分区和 1 个固件分区。

空中升级 (OTA) 
----------------

固件升级使用活动-非活动分区方案，并预留两个 flash 分区（如下图所示），OTA Data 分区将记录哪个是活动分区。

.. figure:: ../../_static/flash_partitions_upgrade.png
   :alt: OTA Flash Partitions

   OTA Flash 分区

OTA 固件升级过程中，状态变更如图所示：

-  步骤 0：OTA 0 为活动固件，该信息储存在 OTA Data 分区（如图所示）。

-  步骤 1：固件升级开始，识别并擦除非活动分区，新的固件将写入 OTA 1 分区。

-  步骤 2：固件写入完毕，开始进行验证。

-  步骤 3：固件升级成功，OTA Data 分区已更新，并指示 OTA 1 现在是活动分区。下次启动时，固件将从此分区启动。 

.. figure:: ../../_static/upgrade_flow.png
   :alt: Firmware Upgrade Flow

   固件升级步骤

.. _sec_updating\_flash\_partitions:

更新 Flash 分区
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

那么，我们如何让 IDF 创建一个分区表，既包含 OTA Data 分区又包含 2 个储存固件的分区？

我们可以创建一个分区文件来实现，即 CSV 文件（Comma Separated Values，逗号分隔值），此文件会指示 IDF 我们想要的分区是什么，大小应该是多少，以及如何放置等问题。

本示例所用的分区文件如下图所示：

.. code:: text


    # Name,   Type, SubType, Offset,  Size, Flags
    # Note: if you change the phy_init or app partition offset
    # make sure to change the offset in Kconfig.projbuild
    nvs,      data, nvs,     ,        0x6000,
    otadata,  data, ota,     ,        0x2000,
    phy_init, data, phy,     ,        0x1000,
    ota_0,    app,  ota_0,   ,        1600K,
    ota_1,    app,  ota_1,   ,        1600K,

上述分区文件指导 IDF 创建 NVS、OTA Data、OTA 0 及 OTA 1 分区，同时指定分区大小。

创建此分区文件后，我们应指示 IDF 使用该自定义分区，而非默认分区，可以通过更新 SDK 配置来启用自定义分区。本应用程序示例中，此项设置已经在 *6\_ota/sdkconfig.defaults* 文件中激活，因此无需再进行其他激活操作。

但如果希望使用不同的分区文件，或更新主固件的偏移量，请修改此设置。可以通过执行 *make menuconfig* 命令来实现，然后在 *menuconfig* -> *Partition Table* 中配置正确的选项。 

.. _sec_for\_esp8266\_users:

ESP8266 用户
~~~~~~~~~~~~~~~~~

对使用 ESP8266 的用户而言，如果开发板仅有 2 MB flash，则请使用下面的分区表：

.. code:: text

        # Name,   Type, SubType, Offset,  Size, Flags
        # Note: if you change the phy_init or app partition offset, make sure to change the offset in Kconfig.projbuild
        nvs,      data, nvs,     0x9000,   0x4000,
        otadata,  data, ota,     0xd000,   0x2000,
        phy_init, data, phy,     0xf000,   0x1000,
        ota_0,    app,  ota_0,   0x10000,  0xC5000,
        ota_1,    app,  ota_1,   0x110000, 0xC5000,

代码
--------

现在我们来看一下实际执行固件升级的代码：

.. code:: c

        esp_http_client_config_t config = {
            .url = url,
            .cert_pem = (char *)upgrade_server_cert_pem_start,
        };
        esp_err_t ret = esp_https_ota(&config);

-  使用 *esp\_http\_client\_config\_t* 定义 OTA 升级源，包括标记升级地址的 URL，用于验证服务器的 CA 证书（升级从此服务器处获取）。注意，请确保按照 :ref:`sec_security\_first` 章节进行 CA 证书验证，这一步非常重要。 

-  然后执行 *esp\_https\_ota()* API 启动固件升级，固件升级成功（或失败）后，此 API 将返回相应的代码（或错误代码）。

-  默认情况下，我们为固件升级 URL 添加了 GitHub 的 CA 证书，这样您就可以轻松地在 GitHub 上存放升级固件并进行升级。理想情况下，您将安装相应服务器的 CA 证书，并从该服务器下载升级固件。

发送固件升级 URL
-------------------------

现在我们的问题是设备如何接收升级 URL。固件升级指令与前面讨论的远程控制指令不同，固件升级通常由设备生产商根据特定的标准为一批或一组设备进行的升级。

为了简便起见，我们使用相同的远程控制基础架构将固件升级 URL 指令传递给设备。请注意，在批量生产时，您将使用其他的云控制机制发送固件升级 URL。

为了快速进行固件升级，我们在 GitHub 上传了一个固件示例（1\_hello\_world 应用程序），可以按照下述方式快速升级该固件映像：

::

        curl -d '{"state":{"desired":{"ota_url":"https://raw.githubusercontent.com/wiki/espressif/esp-jumpstart/images/hello-world.bin"}}}' \
                --tlsv1.2 --cert cloud_cfg/device.cert \
                --key cloud_cfg/device.key \
                https://a3orti3lw2padm-ats.iot.us-east-1.amazonaws.com:8443/things/<contents-of-deviceid.txt-file>/shadow | python -mjson.tool

固件升级成功后，设备将执行 Hello World 固件。

未完待续
---------------

有了这个固件，我们就实现了智能连网设备的一大关键功能，即固件升级功能。

到现在为止，产品固件马上准备就绪，最后就是维护设备特有数据，我们将在下一章中讨论这一问题。
