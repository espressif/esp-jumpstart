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

import sys

def bytes_to_hexstr(bs):
    if (sys.version_info > (3, 0)):
        return bs.hex()
    else:
        return bs.encode('hex')

def hexstr_to_bytes(hx):
    if (sys.version_info > (3, 0)):
        return bytes.fromhex(hx)
    else:
        return hx

def str_to_bytes(string):
    if (sys.version_info > (3, 0)):
        return bytes(string, 'utf8')
    else:
        return bytes(string)

def bytearr_to_bytes(bs):
    if (sys.version_info > (3, 0)):
        return bytes([b for b in bs])
    else:
        return bytes(bs)

def bytes_to_str(bs):
    if (sys.version_info > (3, 0)):
        return bs.decode('utf8')
    else:
        return ''.join(chr(b) for b in bs)

def compat_bytes(bs):
    if (sys.version_info > (3, 0)):
        return ''.join(chr(b) for b in list(bs))
    else:
        return bs

def xor(a, b):
    a = compat_bytes(a)
    b = compat_bytes(b)
    ret = ''
    for i in range(max(len(a), len(b))):
        num = hex(ord(a[i%len(a)]) ^ ord(b[i%(len(b))]))[2:]
        if len(num) == 0:
            num = '00'
        if len(num) == 1:
            num = '0'+ num
        ret = ret + num
    if (sys.version_info > (3, 0)):
        return bytes.fromhex(ret)
    else:
        return ret.decode('hex')
