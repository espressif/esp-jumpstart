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
from .security import *

from cryptography.hazmat.backends import default_backend
from cryptography.hazmat.primitives import hashes
from cryptography.hazmat.primitives.asymmetric.x25519 import X25519PrivateKey, X25519PublicKey
from cryptography.hazmat.primitives.ciphers import Cipher, algorithms, modes

import session_pb2

class security_state:
    REQUEST1 = 0
    RESPONSE1_REQUEST2 = 1
    RESPONSE2 = 2
    FINISHED = 3

class Security1(Security):
    def __init__(self, pop, verbose):
        self.session_state = security_state.REQUEST1
        self.pop = utils.str_to_bytes(pop)
        self.verbose = verbose
        Security.__init__(self, self.security1_session)

    def security1_session(self, response_data):
        if (self.session_state == security_state.REQUEST1):
            self.session_state = security_state.RESPONSE1_REQUEST2
            return self.setup0_request()
        if (self.session_state == security_state.RESPONSE1_REQUEST2):
            self.session_state = security_state.RESPONSE2
            self.setup0_response(response_data)
            return self.setup1_request()
        if (self.session_state == security_state.RESPONSE2):
            self.session_state = security_state.FINISHED
            self.setup1_response(response_data)
            return None
        else:
            print("Unexpected state")
            return None

    def __generate_key(self):
        self.client_private_key = X25519PrivateKey.generate()
        self.client_public_key  = self.client_private_key.public_key()

    def _print_verbose(self, data):
        if (self.verbose):
            print("++++ " + data + " ++++")

    def setup0_request(self):
        setup_req = session_pb2.SessionData()
        setup_req.sec_ver = session_pb2.SecScheme1
        self.__generate_key()
        setup_req.sec1.sc0.client_pubkey = self.client_public_key.public_bytes()
        self._print_verbose("Client Public Key:\t" + utils.bytes_to_hexstr(setup_req.sec1.sc0.client_pubkey))
        return setup_req.SerializeToString()

    def setup0_response(self, response_data):
        setup_resp = proto.session_pb2.SessionData()
        setup_resp.ParseFromString(utils.bytearr_to_bytes(response_data))
        self._print_verbose("Security version:\t" + str(setup_resp.sec_ver))
        if setup_resp.sec_ver != session_pb2.SecScheme1:
            print("Incorrect sec scheme")
            exit(1)
        self._print_verbose("Device Public Key:\t" + utils.bytes_to_hexstr(setup_resp.sec1.sr0.device_pubkey))
        self._print_verbose("Device Random:\t" + utils.bytes_to_hexstr(setup_resp.sec1.sr0.device_random))
        sharedK = self.client_private_key.exchange(X25519PublicKey.from_public_bytes(setup_resp.sec1.sr0.device_pubkey))
        self._print_verbose("Shared Key:\t" + utils.bytes_to_hexstr(sharedK))
        if len(self.pop) > 0:
            h = hashes.Hash(hashes.SHA256(), backend=default_backend())
            h.update(self.pop)
            digest = h.finalize()
            sharedK = utils.xor(sharedK, digest)
            self._print_verbose("New Shared Key XORed with PoP:\t" + utils.bytes_to_hexstr(sharedK))
        self._print_verbose("IV " +  hex(int(utils.bytes_to_hexstr(setup_resp.sec1.sr0.device_random), 16)))
        cipher = Cipher(algorithms.AES(sharedK), modes.CTR(setup_resp.sec1.sr0.device_random), backend=default_backend())
        self.cipher = cipher.encryptor()
        self.client_verify = self.cipher.update(setup_resp.sec1.sr0.device_pubkey)
        self._print_verbose("Client Verify:\t" + utils.bytes_to_hexstr(self.client_verify))

    def setup1_request(self):
        setup_req = proto.session_pb2.SessionData()
        setup_req.sec_ver = session_pb2.SecScheme1
        setup_req.sec1.msg = proto.sec1_pb2.Session_Command1
        setup_req.sec1.sc1.client_verify_data = self.client_verify
        return setup_req.SerializeToString()

    def setup1_response(self, response_data):
        setup_resp = proto.session_pb2.SessionData()
        setup_resp.ParseFromString(utils.bytearr_to_bytes(response_data))
        if setup_resp.sec_ver == session_pb2.SecScheme1:
            self._print_verbose("Device verify:\t" + utils.bytes_to_hexstr(setup_resp.sec1.sr1.device_verify_data))
            enc_client_pubkey = self.cipher.update(setup_resp.sec1.sr1.device_verify_data)
            self._print_verbose("Enc client pubkey:\t " + utils.bytes_to_hexstr(enc_client_pubkey))
        else:
            print("Unsupported security protocol")
            return -1

    def encrypt_data(self, data):
        return self.cipher.update(data)

    def decrypt_data(self, data):
        return self.cipher.update(utils.bytearr_to_bytes(data))
