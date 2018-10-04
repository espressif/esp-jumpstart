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
import proto

def print_verbose(security_ctx, data):
    if (security_ctx.verbose):
         print("++++ " + data + " ++++")

def config_get_status_request(security_ctx):
    cfg1 = proto.wifi_config_pb2.WiFiConfigPayload()
    cfg1.msg = proto.wifi_config_pb2.TypeCmdGetStatus
    cmd_get_status = proto.wifi_config_pb2.CmdGetStatus()
    cfg1.cmd_get_status.MergeFrom(cmd_get_status)
    print_verbose(security_ctx, "Plaintext CmdGetStatus " + utils.bytes_to_hexstr(cfg1.SerializeToString()))
    encrypted_cfg = security_ctx.encrypt_data(cfg1.SerializeToString())
    print_verbose(security_ctx, "Client -> Device (Encrypted CmdGetStatus) " + utils.bytes_to_hexstr(encrypted_cfg))
    return encrypted_cfg

def config_get_status_response(security_ctx, response_data):
    decrypted_message = security_ctx.decrypt_data(response_data)
    cmd_resp1 = proto.wifi_config_pb2.WiFiConfigPayload()
    cmd_resp1.ParseFromString(decrypted_message)
    print_verbose(security_ctx, "Response type " + str(cmd_resp1.msg))
    print_verbose(security_ctx, "Response status " + str(cmd_resp1.resp_get_status.status))
    if cmd_resp1.resp_get_status.sta_state == 0:
        print("++++ WiFi state: " + "connected ++++")
    elif cmd_resp1.resp_get_status.sta_state == 1:
        print("++++ WiFi state: " + "connecting... ++++")
    elif cmd_resp1.resp_get_status.sta_state == 2:
        print("++++ WiFi state: " + "disconnected ++++")
    elif cmd_resp1.resp_get_status.sta_state == 3:
        print("++++ WiFi state: " + "connection failed ++++")
        if cmd_resp1.resp_get_status.fail_reason == 0:
            print("++++ Failure reason: " + "Incorrect Password ++++")
        elif cmd_resp1.resp_get_status.fail_reason == 1:
            print("++++ Failure reason: " + "Incorrect SSID ++++")
    return cmd_resp1.resp_get_status.sta_state

def config_set_config_request(security_ctx, ssid, passphrase):
    cmd = proto.wifi_config_pb2.WiFiConfigPayload()
    cmd.msg = proto.wifi_config_pb2.TypeCmdSetConfig
    cmd.cmd_set_config.ssid = utils.str_to_bytes(ssid)
    cmd.cmd_set_config.passphrase = utils.str_to_bytes(passphrase)
    enc_cmd = security_ctx.encrypt_data(cmd.SerializeToString())
    print_verbose(security_ctx, "Client -> Device (SetConfig cmd) " + utils.bytes_to_hexstr(enc_cmd))
    return enc_cmd

def config_set_config_response(security_ctx, response_data):
    decrypt = security_ctx.decrypt_data(response_data)
    cmd_resp4 = proto.wifi_config_pb2.WiFiConfigPayload()
    cmd_resp4.ParseFromString(decrypt)
    print_verbose(security_ctx, "SetConfig status " + str(cmd_resp4.resp_set_config.status))
    return cmd_resp4.resp_set_config.status

def config_apply_config_request(security_ctx):
    cmd = proto.wifi_config_pb2.WiFiConfigPayload()
    cmd.msg = proto.wifi_config_pb2.TypeCmdApplyConfig
    enc_cmd = security_ctx.encrypt_data(cmd.SerializeToString())
    print_verbose(security_ctx, "Client -> Device (ApplyConfig cmd) " + utils.bytes_to_hexstr(enc_cmd))
    return enc_cmd

def config_apply_config_response(security_ctx, response_data):
    decrypt = security_ctx.decrypt_data(response_data)
    cmd_resp5 = proto.wifi_config_pb2.WiFiConfigPayload()
    cmd_resp5.ParseFromString(decrypt)
    print_verbose(security_ctx, "ApplyConfig status " + str(cmd_resp5.resp_apply_config.status))
    return cmd_resp5.resp_apply_config.status

