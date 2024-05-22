from collections import namedtuple
import struct

PUBLICKEYSTRUC = namedtuple('PUBLICKEYSTRUC', 'bType bVersion aiKeyAlg')  # reserved is skipped when unpacking
PUBLICKEYSTRUC_s = struct.Struct('<bb2xI')

PRIVATEKEYBLOB = namedtuple('PRIVATEKEYBLOB', 'modulus prime1 prime2 exponent1 exponent2 coefficient privateExponent')

RSAPUBKEY = namedtuple('RSAPUBKEY', 'magic bitlen pubexp')
RSAPUBKEY_s = struct.Struct('<4sII')

RSAPUBKEY_MAGIC = 'RSA1'
PRIVATEKEYBLOB_MAGIC = 'RSA2'


# bType
bType_SIMPLEBLOB = 1
bType_PUBLICKEYBLOB = 6
bType_PRIVATEKEYBLOB = 7
bType_PLAINTEXTKEYBLOB = 8

# CALG
CALG_AES_128 = 0x660e
CALG_AES_192 = 0x660f
CALG_AES_256 = 0x6610
CALG_RC4 = 0x6801
CALG_MD5 = 0x8003
CALG_SHA = 0x8004
CALG_SHA1 = 0x8004
CALG_RSA_KEYX = 0xa400

# Hash params
HP_ALGID = 0x0001
HP_HASHVAL = 0x0002
HP_HASHSIZE = 0x0004

# key params
KP_ALGID = 7
KP_BLOCKLEN = 8
KP_KEYLEN = 9

CRYPT_EXPORTABLE = 1

CUR_BLOB_VERSION = 2