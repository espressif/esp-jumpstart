量产
=============

:link_to_translation:`en:[English]`

构建物联网产品时，我们通常需要在每个设备中储存一些唯一性信息。

例如，我们在前面步骤中提到了云平台使用证书进行身份验证，我们还把需要用到的设备证书嵌入了固件中，这些步骤都用到了这种唯一性信息（证书）。开发单个设备时，可以使用上述方法，将这种唯一性信息直接嵌入到固件中，但如果需要开发大量设备呢？在本章节中，我们将讨论这一问题。

在 :ref:`sec_nvs\_info` 章节中，我们讨论了 NVS 分区，用于将键值对永久储存在 flash 中。这样即使设备重启，这部分信息也不会丢失。我们还在 :ref:`sec_reset\_to\_factory` 章节中提到了擦除 NVS 分区内容即可将设备恢复出厂设置。

量产时，我们也可以使用类似的 NVS 分区储存每个设备唯一的键值对，但不希望设备恢复出厂设置时擦除这部分信息。要达到这一目的，我们可以创建另一个 NVS 分区，用于储存工厂嵌入的唯一性信息。由于此分区是在工厂预嵌入的，因此我们将此 NVS 分区用作只读分区，仅用来读取设备配置的唯一性信息。

因此，我们就使用这种方法来存储工厂的唯一性信息。

多个 NVS 分区
-----------------------

在讨论固件升级时，我们在 :ref:`sec_flash\_partitions` 章节中研究了 *flash 分区*，还在 :ref:`sec_updating\_flash\_partitions` 章节中研究了如何修改 flash 分区。在本示例中，我们将新添加一个名为 *fctry* 的 NVS 分区，用于存储唯一的工厂配置信息。

您可以在 *7\_mfg/partitions.csv* 文件中查看相关信息。

代码
--------

有了这个 *fctry* NVS 分区，我们就可以使用标准 NVS API 进行访问。唯一的问题是，我们在执行 NVS 操作时，需要指示 NVS 使用该分区。这可以通过初始化 NVS 句柄来完成，如下所示：

.. code:: c

    #define MFG_PARTITION_NAME "fctry"
    /* Error checks removed for brevity */
    nvs_handle fctry_handle;
    nvs_flash_init_partition(MFG_PARTITION_NAME);
    nvs_open_from_partition(MFG_PARTITION_NAME, "mfg_ns",
                   NVS_READWRITE, &fctry_handle);

现在，您可以使用 *fctry\_handle* NVS 句柄执行 NVS 操作，从该 *fctry* NVS 分区读取数据。例如:

.. code:: c

    nvs_get_str(fctry_handle, "serial_no", buf, &buflen);

现在，我们就可以从烧录至设备的 *fctry* NVS 分区中读取证书信息，而不再需要使用代码将证书嵌入到固件中。

.. _sec_gen\_factory\_data:

生成工厂数据
---------------------------

现在我们可以从固件方面讨论这个问题了。但在此之前，我们仍然需要指定工厂数据生成的机制，这些工厂数据将写入 *fctry* 分区。

.. figure:: ../_static/generate_factory_partition.png
   :alt: Generating Factory Partition

   生成工厂分区

使用 *components/nvs\_flash/nvs\_partition\_generator/nvs\_partition\_gen.py* 程序在开发主机上生成 NVS 镜像，然后我们需要将镜像写入 flash 中的 *fctry* 分区。

此程序接收一个 CSV 文件，并依据 CSV 文件生成一个 NVS 分区镜像。CSV 文件储存键值对信息，这部分信息将被加入到生成的 NVS 分区。工厂将生成海量的此类 NVS 分区镜像，每生产一台设备，就将一个唯一的分区镜像写入设备。

应用程序中还提供了一个名为 *mfg\_config.csv* 的 CSV 示例文件。文件中每一行都包含有在工厂设置的唯一变量值，更新变量值后，您唯一的设置信息就被加入到 CSV 文件。

接下来，您可以使用下面的指令生成 NVS 分区 bin 文件：

.. code:: bash

    $ python $IDF_PATH/components/nvs_flash/nvs_partition_generator/nvs_partition_gen.py generate mfg_config.csv my_mfg.bin 0x6000

my_mfg.bin 文件就是 NVS 分区数据，现在可以嵌入到设备中。您可以使用以下命令将此 NVS 分区 bin 文件写入 flash：

.. code:: bash

    $ $IDF_PATH/components/esptool_py/esptool/esptool.py --port $ESPPORT write_flash 0x340000 my_mfg.bin



.. code:: bash

    $ $IDF_PATH/components/esptool_py/esptool/esptool.py --port $ESPPORT write_flash 0x1D5000 my_mfg.bin

现在，如果启动固件，固件将会像上一章中的固件一样进行工作。但在这种情况下，固件映像的功能与设备内置的唯一配置信息无关。但会根据这些唯一的配置信息表象为不同设备。

这样，您就可以根据需要创建任意数量的唯一性镜像，然后将这些镜像烧录到相应的开发板上。

请参考 `工厂分区文档 <https://medium.com/the-esp-journal/building-products-creating-unique-factory-data-images-3f642832a7a3>`_，查看更多详细信息。

未完待续
---------------

在本章中，我们研究了如何为每个设备创建具备唯一性的工厂镜像，设备不同，镜像内容也不同。

现在，我们就有了一个功能齐全，可以量产的设备固件！
