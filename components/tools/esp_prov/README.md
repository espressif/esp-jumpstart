# ESP Provisioining Tool

A python based utility for testing the provisioning examples over a host.

Example usage :
* python esp_prov.py --ssid \<AP SSID\> --passphrase \<AP Password\> --sec_ver \<Security version 0 or 1\> --pop \<proof of possesion string, if any\> --prov_mode \<mode of provisioning - softap or ble\> \[--ble_devname \<BLE device name\>\] \[--avs_config\]

For more options, run : python esp_prov.py --help


# Installation

This requires the following python libraries to run (included in requirements.txt):
* future
* protobuf
* netifaces
* dbus-python (requires libdbus-1-dev and libglib2.0-dev)
* cryptography (requires libffi-dev and libssl-dev)

Steps to install:
1. libffi:
    a. For Debian/Ubuntu:
        apt-get install libffi-dev libssl-dev libdbus-1-dev libglib2.0-dev
    b. For archlinux:
        pacman -S libffi openssl dbus dbusglib
2. pip install -r $IDF_PATH/tools/esp_prov/requirements.txt

Note : The packages listed in requirements.txt are limited only to the ones needed AFTER fully satisfying the requirements of ESP-IDF.
