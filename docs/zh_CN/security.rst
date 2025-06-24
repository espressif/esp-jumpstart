安全性
=======================

:link_to_translation:`en:[English]`

在上一章节中，我们实现了能够进行量产的设备固件，但还不能完美收官，因为我们还没有考虑到设备的安全性问题。在本章节中，我们将介绍有哪些安全性问题需要考虑。

远程通信安全
-----------------------------

设备与外部实体通信时，必须确保通信安全，建议使用已有的 TLS 协议标准来保护通信安全，而不推荐使用其他新方法。ESP-IDF 支持 *mbedtls*，*mbedtls* 实现了 TLS 协议的全部功能。

ESP-Jumpstart 项目中所有代码均采用了上述安全机制，确保远程通信安全。您也可以采用这种机制来保护设备固件其他类型的远程通信，如果您尚未用到远程通信，请跳过本章节。

CA 证书
~~~~~~~~~~~~~~~

TLS 层使用受信任的 CA 证书验证远程端点/服务器是否正确。

*esp\_tls* API 则接收 CA 证书用于服务器验证。

.. code:: c

            esp_tls_cfg_t cfg = {
                .cacert_pem_buf  = server_root_cert_pem_start,
                .cacert_pem_bytes = server_root_cert_pem_end - server_root_cert_pem_start,
            };

            struct esp_tls *tls = esp_tls_conn_http_new("https://www.example.com", &cfg);

如果没有此参数，则会跳过服务器验证。强烈建议对所有 TLS 通信，指定受信任的 CA 证书用于服务器验证。

获取 CA 证书
~~~~~~~~~~~~~~~~~~~~~~~~~

从上面的代码可以看出，必须将受信任的 CA 证书嵌入固件，才能用于验证服务器。您可以使用以下命令获取受信任的 CA 证书：

.. code:: bash

    $ openssl s_client -showcerts -connect www.example.com:443 < /dev/null

运行此命令将输出证书列表，请将列表中最后一个证书嵌入到设备固件中，具体步骤请查看 :ref:`sec_embedding\_files` 章节。

物理访问安全
--------------------------

ESP32 可以保护设备免受物理篡改，保障设备物理访问安全。本章后续内容将详细介绍这一功能。



Secure Boot
~~~~~~~~~~~

Secure Boot 可以确保 ESP32 从 flash 运行任何软件时，软件受信任且由已知实体签名。如果软件 Bootloader 和应用程序固件中有任何改动，此固件则被视为不受信任，设备将拒绝执行此不受信任的代码。

这一功能是通过构建信任链来实现的，包括从硬件到 Bootloader，再到固件。

.. figure:: ../_static/secure_boot.png
   :alt: Secure Boot

   Secure Boot

具体工作步骤如下：

-  量产时：

   -  将密钥嵌入到 ESP32 eFUSE，密钥嵌入后禁止软件读出或写入；

   -  Bootloader 和固件使用正确的密钥进行签名，并将签名添加到映像中；

   -  将签名版的 Bootloader 和固件映像嵌入 ESP32 flash。

-  ESP32 上电复位时：

   -  BootROM 使用 eFUSE 中嵌入的安全密钥，验证 Bootloader；

   -  Bootloader 验证通过后，BootROM 开始加载并执行 Bootloader；

   -  Bootloader 开始验证固件签名；

   -  固件签名验证通过后，Bootloader 加载并执行固件。

如上所述，启动设备 Secure Boot 之前，我们需要先进行其他一些操作。请参考 `Secure Boot 相关文档 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/security/secure-boot.html>`_，查看更多详细信息。

Flash 加密
~~~~~~~~~~~~~~~

flash 加密可以确保储存在 ESP32 flash 中的应用程序固件保持加密状态，从而允许制造商在设备中传输加密固件。

启用 flash 加密时，所有经内存映射对 flash 进行的读操作，均在运行时进行透明解密。flash 控制器使用储存在 eFUSE 中的 AES 密钥来执行 AES 解密。这个存储在 eFUSE 中的加密密钥与上面的 Secure Boot 密钥是分开存放的，该密钥还可以防止软件读出和写入。因此，只有硬件有权对 flash 内容解密。

.. figure:: ../_static/flash_encryption.png
   :alt: Flash Encryption

   flash 加密

请参考 `flash 加密文档 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/security/flash-encryption.html>`_，查看如何启用 flash 加密功能。

NVS 加密
~~~~~~~~~~~~~~

与应用程序固件相比，NVS 分区具有不同的访问模式，写操作更频繁，且内容依赖用户偏好。适用于应用程序固件的加密技术，并不是 NVS 加密的最佳选择。因此，ESP-IDF 为 NVS 分区提供了专门的加密机制，即行业标准 AES-XTS 加密，推荐使用这种加密方式保护静态数据。

具体工作步骤如下：

-  量产时：

   -  创建一个单独的 flash 分区，专门储存用于 NVS 加密的密钥；

   -  将此分区标记为 flash 加密；

   -  使用 *nvs\_partition\_gen.py* 工具生成随机密钥分区文件；

   -  将生成的分区文件写入新建的分区。

-  在固件中：

   -  调用 *nvs\_flash\_read\_security\_cfg()* API 从上述分区读取加密密钥，并将密钥填充到 *nvs\_sec\_cfg\_t* 中；

   -  使用 *nvs\_flash\_secure\_init()* API 或 *nvs\_flash\_secure\_init\_partition()* API 初始化 NVS flash 分区；

   -  正常执行其他的 NVS 操作。

请参考 `NVS 加密相关文档 <https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-reference/storage/nvs_flash.html#nvs-encryption>`_，查看更多详细信息。
