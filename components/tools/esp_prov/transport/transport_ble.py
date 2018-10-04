# Copyright 2018 Espressif Systems (Shanghai) PTE LTD
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from __future__ import print_function

import utils
from .transport import *

import dbus
import dbus.mainloop.glib
import time

class Transport_BLE(Transport):
    def __init__(self, devname, iface='hci0'):
        self.devname = devname
        self.sess_secu_ep_path = None
        self.wifi_conf_ep_path = None
        self.prov_vers_ep_path = None
        self.device = None
        self.adapter = None
        self.adapter_props = None

        dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)
        bus = dbus.SystemBus()
        manager = dbus.Interface(bus.get_object("org.bluez", "/"), "org.freedesktop.DBus.ObjectManager")
        objects = manager.GetManagedObjects()

        for path, interfaces in objects.items():
            adapter = interfaces.get("org.bluez.Adapter1")
            if adapter != None:
                if path.endswith(iface):
                    self.adapter = dbus.Interface(bus.get_object("org.bluez", path), "org.bluez.Adapter1")
                    self.adapter_props = dbus.Interface(bus.get_object("org.bluez", path), "org.freedesktop.DBus.Properties")
                    break

        if self.adapter == None:
            raise RuntimeError("Bluetooth adapter not found")

        self.adapter_props.Set("org.bluez.Adapter1", "Powered", dbus.Boolean(1))
        self.adapter.StartDiscovery()

        retry = 10
        while (retry > 0):
            try:
                if self.device == None:
                    print("Connecting...")
                    # Wait for device to be discovered
                    time.sleep(5)
                    self._connect_()
                    print("Connected")
                print("Getting Services...")
                # Wait for services to be discovered
                time.sleep(5)
                self._get_services_()
                return
            except Exception as e:
                print(e)
                retry -= 1
                print("Retries left", retry)
                continue
        self.adapter.StopDiscovery()
        raise RuntimeError("Failed to initialise transport")

    def _connect_(self):
        bus = dbus.SystemBus()
        manager = dbus.Interface(bus.get_object("org.bluez", "/"), "org.freedesktop.DBus.ObjectManager")
        objects = manager.GetManagedObjects()
        dev_path = None
        for path, interfaces in objects.items():
            if "org.bluez.Device1" not in interfaces.keys():
                continue
            if interfaces["org.bluez.Device1"].get("Name") == self.devname:
                dev_path = path
                break

        if dev_path == None:
            raise RuntimeError("BLE device not found")

        try:
            self.device = bus.get_object("org.bluez", dev_path)
            self.device.Connect(dbus_interface='org.bluez.Device1')
        except Exception as e:
            print(e)
            self.device = None
            raise RuntimeError("BLE device could not connect")

    def _get_services_(self):
        bus = dbus.SystemBus()
        manager = dbus.Interface(bus.get_object("org.bluez", "/"), "org.freedesktop.DBus.ObjectManager")
        objects = manager.GetManagedObjects()
        srv_path = None
        for path, interfaces in objects.items():
            if "org.bluez.GattService1" not in interfaces.keys():
                continue
            if path.startswith(self.device.object_path):
                service = bus.get_object("org.bluez", path)
                uuid = service.Get('org.bluez.GattService1', 'UUID',
                            dbus_interface='org.freedesktop.DBus.Properties')
                if uuid == '0000ffff-0000-1000-8000-00805f9b34fb':
                    srv_path = path
                    break

        if srv_path == None:
            raise RuntimeError("Provisioning service not found")

        for path, interfaces in objects.items():
            if "org.bluez.GattCharacteristic1" not in interfaces.keys():
                continue
            if path.startswith(srv_path):
                chrc = bus.get_object("org.bluez", path)
                uuid = chrc.Get('org.bluez.GattCharacteristic1', 'UUID',
                            dbus_interface='org.freedesktop.DBus.Properties')
                if uuid == '0000ff51-0000-1000-8000-00805f9b34fb':
                    self.sess_secu_ep = chrc
                elif uuid == '0000ff52-0000-1000-8000-00805f9b34fb':
                    self.wifi_conf_ep = chrc
                elif uuid == '0000ff53-0000-1000-8000-00805f9b34fb':
                    self.prov_vers_ep = chrc
                else:
                    print("Unknown UUID", uuid)

        if self.sess_secu_ep == None:
            raise RuntimeError("Security endpoint not found")
        if self.wifi_conf_ep == None:
            raise RuntimeError("Configuration endpoint not found")
        if self.prov_vers_ep == None:
            raise RuntimeError("Version endpoint not found")

    def __del__(self):
        self.disconnect()

    def disconnect(self):
        if self.device:
            self.device.Disconnect(dbus_interface='org.bluez.Device1')
            self.adapter.RemoveDevice(self.device)
        if self.adapter:
            self.adapter_props.Set("org.bluez.Adapter1", "Powered", dbus.Boolean(0))

    def send_data(self, path, data):
        path.WriteValue(list(data), {}, dbus_interface='org.bluez.GattCharacteristic1')
        return bytearray([int(b) for b in path.ReadValue({}, dbus_interface='org.bluez.GattCharacteristic1')])

    def send_session_data(self, data):
        return self.send_data(self.sess_secu_ep, data)

    def send_config_data(self, data):
        return self.send_data(self.wifi_conf_ep, data)

    def send_version_data(self, data):
        return self.send_data(self.prov_vers_ep, data)
