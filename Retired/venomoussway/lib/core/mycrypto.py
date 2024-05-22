import hashlib
import hmac
import copy
import struct

import Crypto.Random as Random
from Crypto.Cipher import ARC4
from binascii import hexlify
import lib.core.utils as utils
# Plan on putting all crypto code here, and then have the helpers functions 
# call it to keep crypto code in a central location.

"""Used for both encryption and decryption, because its the same algorithm back and forth."""
def rc4(key, data,skip=1024):
    realkey = key
    realdata = data
    if type(realkey) != bytes:
        realkey = key.encode('utf-8')
    if type(realdata) != bytes:
        realdata = data.encode('utf-8')
    cipher = ARC4.new(realkey)
    return cipher.encrypt(realdata)

