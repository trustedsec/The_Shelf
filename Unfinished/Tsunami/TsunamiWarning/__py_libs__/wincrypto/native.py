from ctypes import FormatError
from ctypes import windll, c_void_p, byref, create_string_buffer, c_int
import struct

from wincrypto.definitions import HP_ALGID, HP_HASHSIZE, KP_KEYLEN, KP_ALGID, CRYPT_EXPORTABLE


PROV_RSA_FULL = 1
PROV_RSA_AES = 24


def assert_success(success):
    if not success:
        raise AssertionError(FormatError())


def CryptAcquireContext():
    hprov = c_void_p()
    success = windll.advapi32.CryptAcquireContextA(byref(hprov), 0, 0, PROV_RSA_AES, 0)
    assert_success(success)
    return hprov


def CryptReleaseContext(hprov):
    success = windll.advapi32.CryptReleaseContext(hprov, 0)
    assert_success(success)


def CryptImportKey(hprov, keyblob, hPubKey=0):
    hkey = c_void_p()
    success = windll.advapi32.CryptImportKey(hprov, keyblob, len(keyblob), hPubKey, 0, byref(hkey))
    assert_success(success)
    return hkey


def CryptExportKey(hkey, hexpkey, blobType):
    # determine output buffer length
    bdatalen = c_int(0)
    success = windll.advapi32.CryptExportKey(hkey, hexpkey, blobType, 0, 0, byref(bdatalen))
    assert_success(success)

    # export key
    bdata = create_string_buffer('', bdatalen.value)
    success = windll.advapi32.CryptExportKey(hkey, hexpkey, blobType, 0, bdata, byref(bdatalen))
    assert_success(success)
    return bdata.raw[:bdatalen.value]


def CryptDestroyKey(hkey):
    success = windll.advapi32.CryptDestroyKey(hkey)
    assert_success(success)


def CryptDecrypt(hkey, encrypted_data):
    bdata = create_string_buffer(encrypted_data)
    bdatalen = c_int(len(encrypted_data))
    success = windll.advapi32.CryptDecrypt(hkey, 0, 1, 0, bdata, byref(bdatalen))
    assert_success(success)
    return bdata.raw[:bdatalen.value]


def CryptEncrypt(hkey, plain_data):
    # determine output buffer length
    bdatalen_test = c_int(len(plain_data))
    success = windll.advapi32.CryptEncrypt(hkey, 0, 1, 0, 0, byref(bdatalen_test), len(plain_data))
    assert_success(success)
    out_buf_len = bdatalen_test.value

    # encrypt data
    bdata = create_string_buffer(plain_data, out_buf_len)
    bdatalen = c_int(len(plain_data))
    success = windll.advapi32.CryptEncrypt(hkey, 0, 1, 0, bdata, byref(bdatalen), out_buf_len)
    assert_success(success)
    return bdata.raw[:bdatalen.value]


def CryptGetKeyParam(hkey, dwparam):
    # determine output buffer length
    bdatalen = c_int(0)
    success = windll.advapi32.CryptGetKeyParam(hkey, dwparam, 0, byref(bdatalen), 0)
    assert_success(success)

    # get hash param
    bdata = create_string_buffer('', bdatalen.value)
    success = windll.advapi32.CryptGetKeyParam(hkey, dwparam, bdata, byref(bdatalen), 0)
    assert_success(success)
    result = bdata.raw[:bdatalen.value]
    if dwparam in [KP_KEYLEN, KP_ALGID]:
        result = struct.unpack('I', result)[0]
    return result


def CryptCreateHash(hProv, Algid):
    hCryptHash = c_void_p()
    success = windll.advapi32.CryptCreateHash(hProv, Algid, 0, 0, byref(hCryptHash))
    assert_success(success)
    return hCryptHash


def CryptHashData(hHash, data):
    bdata = create_string_buffer(data)
    dwdatalen = c_int(len(data))
    success = windll.advapi32.CryptHashData(hHash, bdata, dwdatalen, 0)
    assert_success(success)


def CryptGetHashParam(hHash, dwParam):
    # determine output buffer length
    bdatalen = c_int(0)
    success = windll.advapi32.CryptGetHashParam(hHash, dwParam, 0, byref(bdatalen), 0)
    assert_success(success)

    # get hash param
    bdata = create_string_buffer('', bdatalen.value)
    success = windll.advapi32.CryptGetHashParam(hHash, dwParam, bdata, byref(bdatalen), 0)
    assert_success(success)
    result = bdata.raw[:bdatalen.value]
    if dwParam in [HP_ALGID, HP_HASHSIZE]:
        result = struct.unpack('I', result)[0]
    return result


def CryptDestroyHash(hCryptHash):
    success = windll.advapi32.CryptDestroyHash(hCryptHash)
    assert_success(success)


def CryptDeriveKey(hProv, Algid, hBaseData):
    hkey = c_void_p()
    success = windll.advapi32.CryptDeriveKey(hProv, Algid, hBaseData, CRYPT_EXPORTABLE, byref(hkey))
    assert_success(success)
    return hkey