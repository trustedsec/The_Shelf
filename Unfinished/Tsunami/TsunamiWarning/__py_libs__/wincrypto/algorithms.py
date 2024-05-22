from abc import abstractproperty, ABCMeta
import struct
import hashlib

import Crypto.Cipher.PKCS1_v1_5
import Crypto.Cipher.AES
import Crypto.Cipher.ARC4
from Crypto.PublicKey import RSA
from Crypto.Util.number import long_to_bytes, bytes_to_long, inverse

from wincrypto.definitions import RSAPUBKEY, RSAPUBKEY_s, RSAPUBKEY_MAGIC, PUBLICKEYSTRUC_s, bType_PUBLICKEYBLOB, \
    CUR_BLOB_VERSION, CALG_RSA_KEYX, PRIVATEKEYBLOB_MAGIC, PRIVATEKEYBLOB, bType_PRIVATEKEYBLOB, bType_PLAINTEXTKEYBLOB, \
    bType_SIMPLEBLOB, CALG_RC4, CALG_AES_128, CALG_AES_192, CALG_AES_256, CALG_MD5, CALG_SHA1
from wincrypto.util import add_pkcs5_padding, remove_pkcs5_padding


class HCryptKey(object):
    def __init__(self, key):
        self.key = key


class RSA_KEYX(HCryptKey):
    alg_id = CALG_RSA_KEYX

    @classmethod
    def from_pem(cls, pem_key):
        imported_key = Crypto.PublicKey.RSA.importKey(pem_key)
        return cls(imported_key)

    @classmethod
    def import_publickeyblob(cls, data):
        rsapubkey = RSAPUBKEY._make(RSAPUBKEY_s.unpack_from(data))
        assert rsapubkey.magic == RSAPUBKEY_MAGIC
        bitlen8 = rsapubkey.bitlen / 8
        modulus = bytes_to_long(data[12:12 + bitlen8][::-1])
        r = RSA.construct((modulus, long(rsapubkey.pubexp)))
        return cls(r)

    def export_publickeyblob(self):
        n = self.key.key.n
        e = self.key.key.e
        n_bytes = long_to_bytes(n)[::-1]
        result = PUBLICKEYSTRUC_s.pack(bType_PUBLICKEYBLOB, CUR_BLOB_VERSION, CALG_RSA_KEYX)
        result += RSAPUBKEY_s.pack(RSAPUBKEY_MAGIC, len(n_bytes) * 8, e)
        result += n_bytes
        return result

    @classmethod
    def import_privatekeyblob(cls, data):
        rsapubkey = RSAPUBKEY._make(RSAPUBKEY_s.unpack_from(data))
        assert rsapubkey.magic == PRIVATEKEYBLOB_MAGIC
        bitlen8 = rsapubkey.bitlen / 8
        bitlen16 = rsapubkey.bitlen / 16
        privatekeyblob_s = struct.Struct(
            '%ds%ds%ds%ds%ds%ds%ds' % (bitlen8, bitlen16, bitlen16, bitlen16, bitlen16, bitlen16, bitlen8))
        privatekey = PRIVATEKEYBLOB._make(bytes_to_long(x[::-1]) for x in privatekeyblob_s.unpack_from(data[12:]))

        r = RSA.construct((privatekey.modulus, long(rsapubkey.pubexp), privatekey.privateExponent,
                           privatekey.prime1, privatekey.prime2))
        return cls(r)

    def export_privatekeyblob(self):
        key = self.key.key
        n = key.n
        e = key.e
        d = key.d
        p = key.p
        q = key.q

        n_bytes = long_to_bytes(n)[::-1]
        key_len = len(n_bytes) * 8
        result = PUBLICKEYSTRUC_s.pack(bType_PRIVATEKEYBLOB, CUR_BLOB_VERSION, CALG_RSA_KEYX)
        result += RSAPUBKEY_s.pack(PRIVATEKEYBLOB_MAGIC, key_len, e)
        result += n_bytes
        result += long_to_bytes(p, key_len / 16)[::-1]
        result += long_to_bytes(q, key_len / 16)[::-1]
        result += long_to_bytes(d % (p - 1), key_len / 16)[::-1]
        result += long_to_bytes(d % (q - 1), key_len / 16)[::-1]
        result += long_to_bytes(inverse(q, p), key_len / 16)[::-1]
        result += long_to_bytes(d, key_len / 8)[::-1]
        return result

    def decrypt(self, data):
        data = data[::-1]
        c = Crypto.Cipher.PKCS1_v1_5.new(self.key)
        result = c.decrypt(data, None)
        return result


    def encrypt(self, data):
        c = Crypto.Cipher.PKCS1_v1_5.new(self.key)
        result = c.encrypt(data)
        result = result[::-1]
        return result


class symmetric_HCryptKey(HCryptKey):
    __metaclass__ = ABCMeta
    alg_id = abstractproperty()
    key_len = abstractproperty()

    def __init__(self, key):
        if len(key) != self.key_len:
            raise AssertionError('key must be {} bytes long'.format(self.key_len))
        super(symmetric_HCryptKey, self).__init__(key)

    @classmethod
    def import_plaintextkeyblob(cls, data):
        key_len = struct.unpack('<I', data[:4])[0]
        key = data[4:4 + key_len]
        return cls(key)

    def export_plaintextkeyblob(self):
        result = PUBLICKEYSTRUC_s.pack(bType_PLAINTEXTKEYBLOB, 2, self.alg_id)
        result += struct.pack('<I', len(self.key))
        result += self.key
        return result

    @classmethod
    def import_simpleblob(cls, data, hPubKey):
        assert struct.unpack('<I', data[:4])[0] == CALG_RSA_KEYX
        assert hPubKey
        pkcs_1_encrypted_key = data[4:][::-1]
        c = Crypto.Cipher.PKCS1_v1_5.new(hPubKey)
        key = c.decrypt(pkcs_1_encrypted_key, None)
        return cls(key)

    def export_simpleblob(self, rsa_key):
        result = PUBLICKEYSTRUC_s.pack(bType_SIMPLEBLOB, CUR_BLOB_VERSION, self.alg_id)
        result += struct.pack('<I', CALG_RSA_KEYX)
        c = Crypto.Cipher.PKCS1_v1_5.new(rsa_key)
        encrypted_key = c.encrypt(self.key)
        result += encrypted_key[::-1]
        return result


class RC4(symmetric_HCryptKey):
    alg_id = CALG_RC4
    key_len = 16

    def encrypt(self, data):
        return Crypto.Cipher.ARC4.new(self.key).encrypt(data)

    def decrypt(self, data):
        return Crypto.Cipher.ARC4.new(self.key).encrypt(data)


class AES_base(symmetric_HCryptKey):
    alg_id = abstractproperty()

    def encrypt(self, data):
        data = add_pkcs5_padding(data, 16)
        # print("data", data)
        encrypted = Crypto.Cipher.AES.new(self.key, mode=Crypto.Cipher.AES.MODE_CBC, IV='\0' * 16).encrypt(bytes(data))
        return encrypted

    def decrypt(self, data):
        decrypted = Crypto.Cipher.AES.new(self.key, mode=Crypto.Cipher.AES.MODE_CBC, IV='\0' * 16).decrypt(data)
        result = remove_pkcs5_padding(decrypted)
        return result


class AES128(AES_base):
    alg_id = CALG_AES_128
    key_len = 16


class AES192(AES_base):
    alg_id = CALG_AES_192
    key_len = 24


class AES256(AES_base):
    alg_id = CALG_AES_256
    key_len = 32


class HCryptHash():
    __metaclass__ = ABCMeta
    alg_id = abstractproperty()
    hash_class = abstractproperty()

    def __init__(self):
        # noinspection PyCallingNonCallable
        self.hasher = self.hash_class()

    def hash_data(self, data):
        self.hasher.update(data)

    def get_hash_val(self):
        return self.hasher.digest()

    def get_hash_size(self):
        return self.hasher.digest_size


class MD5(HCryptHash):
    alg_id = CALG_MD5
    hash_class = hashlib.md5


class SHA1(HCryptHash):
    alg_id = CALG_SHA1
    hash_class = hashlib.sha1


symmetric_algorithms = [RC4, AES128, AES192, AES256]
asymmetric_algorithms = [RSA_KEYX]
hash_algorithms = [MD5, SHA1]
algorithm_registry = dict((x.alg_id, x) for x in symmetric_algorithms + asymmetric_algorithms + hash_algorithms)