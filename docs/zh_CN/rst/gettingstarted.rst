入门指南
===============

:link_to_translation:`en:[English]`

在本章中，我们将介绍 ESP32 的开发环境，并帮助您了解 ESP32 可用的开发工具和代码仓库。

开发过程概述
--------------------

使用 ESP32 开发产品时，常用的开发环境如下图所示：

.. figure:: ../../_static/dev_setup.png
   :alt: Typical Developer Setup

   ESP32 产品开发过程

上图电脑，即开发主机可以是 Linux、Windows 或 MacOS 操作系统。ESP32 开发板通过 USB 连接到开发主机，开发主机上有 ESP-IDF (乐鑫 SDK)、编译器工具链和项目代码。主机先编译代码生成可执行文件，然后电脑上的工具把生成的文件烧到板子上，然后板子开始执行文件。最后你可以从主机查看日志。

ESP-IDF 介绍
-------------

ESP-IDF 是乐鑫为 ESP32 提供的物联网开发框架。

-  ESP-IDF 包含一系列库及头文件，提供了基于 ESP32 构建软件项目所需的核心组件。

-  ESP-IDF 还提供了开发和量产过程中最常用的工具及功能，例如：构建、烧录、调试和测量等。

设置 ESP-IDF
~~~~~~~~~~~~~~

请参照 `ESP-IDF 入门指南 <https://docs.espressif.com/projects/esp-idf/zh_CN/release-v3.3/get-started/index.html>`_，按照步骤设置 ESP-IDF。注：请完成链接页面的所有步骤。

在进行下面步骤之前，请确认您已经正确设置了开发主机，并按照上面链接中的步骤构建了第一个应用程序。如果上面步骤已经完成，那让我们继续探索 ESP-IDF。

ESP-IDF 详解
~~~~~~~~~~~~~~

ESP-IDF 采用了一种基于组件的架构：

.. figure:: ../../_static/idf_comp.png
   :alt: Component Based Design

   组件设计

ESP-IDF 中的所有软件均以“组件”的形式提供，比如操作系统、网络协议栈、Wi-Fi 驱动程序、以及 HTTP 服务器等中间件等等。

在这种基于“组件”的架构下，你可以轻松使用更多自己研发或第三方提供的组件。

开发人员通常借助 ESP-IDF 构建 *应用程序*，包含业务逻辑、外设驱动程序和 SDK 配置。

.. figure:: ../../_static/app_structure.png
   :alt: Application’s Structure

   应用程序架构

应用程序必须包含一个 *main* 组件，这是保存应用程序逻辑的主要组件。除此之外，应用程序根据自身需求还可以包含其他组件。应用程序的 *Makefile* 文件定义了应用程序的构建指令，此外，应用程序还包含一个可选的 *sdkconfig.defaults* 文件，用于存放应用程序默认的 SDK 配置。 

获取 ESP-Jumpstart 库
---------------------

ESP-Jumpstart 库包含了一系列由 ESP-IDF 构建的 *应用程序*，我们将在本次练习中使用这些应用程序。首先克隆 ESP-Jumpstart 库：

::

    $ git clone --recursive https://github.com/espressif/esp-jumpstart

我们将构建一个可用于量产的固件，因此选择使用 ESP-IDF 稳定版本进行开发。目前 ESP-Jumpstart 使用的是 ESP-IDF v3.3 稳定版本，请切换到这一版本。

::

    $ cd esp-idf
    $ git checkout -b release/v3.3 remotes/origin/release/v3.3
    $ git submodule update --recursive

现在，我们构建 ESP-Jumpstart 中的第一个应用程序 *Hello World*，并将其烧录到开发板上，具体步骤如下，相信您已经熟悉这些步骤：

::

    $ cd esp-jumpstart/1_hello_world
    $ make -j8 menuconfig
    $ export ESPPORT=/dev/cu.SLAB_USBTOUART   # Or the correct device name for your setup
    $ export ESPBAUD=921600
    $ make -j8 flash monitor

上面的步骤将构建整个 SDK 和应用程序。构建成功后，将会把生成的固件烧录到开发板。

烧录成功后，设备将重启。同时，你还可以在控制台看到该固件的输出。

.. _sec_for\_esp8266\_users:

ESP8266 用户
~~~~~~~~~~~~~~~~~

请确保已将 IDF\_PATH 设置为 ESP8266\_RTOS\_SDK 的路径，并使用 esp-jumpstart 的 platform/esp8266 分支。请使用以下命令切换到该分支：  

::

    $ cd esp-jumpstart
    $ git checkout -b platform/esp8266 origin/platform/esp8266

代码
--------

现在，让我们研究一下 Hello World 应用程序的代码，只有如下几行：

.. code:: c

    #include <stdio.h>
    #include "freertos/FreeRTOS.h"
    #include "freertos/task.h"


    void app_main()
    {
        int i = 0;
        while (1) {
            printf("[%d] Hello world!\n", i);
            i++;
            vTaskDelay(5000 / portTICK_PERIOD_MS);
        }
    }

这组代码非常简单，下面是一些要点：

-  app\_main() 函数是应用程序入口点，所有应用程序都从这里开始执行。FreeRTOS 内核在 ESP32 双核上运行之后将调用此函数，FreeRTOS 一旦完成初始化，即将在 ESP32 的其中一个核上新建一个应用程序线程，称为主线程，并在这一线程中调用 app\_main() 函数。应用程序线程的堆栈可以通过 SDK 的配置信息进行配置。

-  printf()、strlen()、time() 等 C 库函数可以直接调用。IDF 使用 newlib C 标准库，newlib 是一个占用空间较低的 C 标准库，支持 stdio、stdlib、字符串操作、数学、时间/时区、文件/目录操作等 C 库中的大多数函数，不支持 signal、locale、wchr 等。在上面示例中，我们使用 printf() 函数将数据输出打印到控制台。 

-  FreeRTOS 是驱动 ESP32 双核的操作系统。`FreeRTOS <https://www.freertos.org>`_ 是一个很小的内核，提供了任务创建、任务间通信（信号量、信息队列、互斥量）、中断和定时器等机制。在上面示例中，我们使用 vTaskDelay 函数让线程休眠 5 秒。有关 FreeRTOS API 的详细信息，请查看 `FreeRTOS 文档 <https://www.freertos.org/a00106.html>`_。

未完待续
---------------

到现在为止，我们已经具备了基本的开发能力，可以进行编译代码、烧录固件、查看固件日志和消息等基本开发操作。

下一步，让我们用 ESP32 构建一个简单的电源插座。
