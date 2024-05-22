from wincrypto import definitions
from wincrypto.algorithms import algorithm_registry
from wincrypto.definitions import PUBLICKEYSTRUC, PUBLICKEYSTRUC_s, CUR_BLOB_VERSION, bType_PUBLICKEYBLOB, \
    bType_PRIVATEKEYBLOB, bType_PLAINTEXTKEYBLOB, bType_SIMPLEBLOB, KP_KEYLEN, KP_ALGID, CALG_AES_192, CALG_AES_256, \
    CALG_AES_128
from wincrypto.util import derive_key_3des_aes


def CryptImportKey(data, pub_key=None):
    publickeystruc = PUBLICKEYSTRUC._make(PUBLICKEYSTRUC_s.unpack_from(data))
    if publickeystruc.bVersion != CUR_BLOB_VERSION:
        raise NotImplementedError('PUBLICKEYSTRUC.bVersion={} not implemented'.format(publickeystruc.bVersion))

    if publickeystruc.bType == bType_PUBLICKEYBLOB:
        if publickeystruc.aiKeyAlg not in algorithm_registry:
            raise NotImplementedError('ALG_ID {:x} not implemented'.format(publickeystruc.aiKeyAlg))
        return algorithm_registry[publickeystruc.aiKeyAlg].import_publickeyblob(data[8:])

    if publickeystruc.bType == bType_PRIVATEKEYBLOB:
        if publickeystruc.aiKeyAlg not in algorithm_registry:
            raise NotImplementedError('ALG_ID {:x} not implemented'.format(publickeystruc.aiKeyAlg))
        return algorithm_registry[publickeystruc.aiKeyAlg].import_privatekeyblob(data[8:])

    if publickeystruc.bType == bType_PLAINTEXTKEYBLOB:
        if publickeystruc.aiKeyAlg not in algorithm_registry:
            raise NotImplementedError('ALG_ID {:x} not implemented'.format(publickeystruc.aiKeyAlg))
        return algorithm_registry[publickeystruc.aiKeyAlg].import_plaintextkeyblob(data[8:])

    if publickeystruc.bType == bType_SIMPLEBLOB:
        if publickeystruc.aiKeyAlg not in algorithm_registry:
            raise NotImplementedError('ALG_ID {:x} not implemented'.format(publickeystruc.aiKeyAlg))
        return algorithm_registry[publickeystruc.aiKeyAlg].import_simpleblob(data[8:], pub_key)

    raise NotImplementedError('PUBLICKEYSTRUC.bType={} not implemented'.format(publickeystruc.bType))


def CryptDecrypt(crypt_key, encrypted_data):
    return crypt_key.decrypt(encrypted_data)


def CryptEncrypt(crypt_key, plain_data):
    return crypt_key.encrypt(plain_data)


def CryptExportKey(crypt_key, exp_key, blob_type):
    if blob_type == bType_SIMPLEBLOB:
        return crypt_key.export_simpleblob(exp_key)
    elif blob_type == bType_PLAINTEXTKEYBLOB:
        return crypt_key.export_plaintextkeyblob()
    elif blob_type == bType_PUBLICKEYBLOB:
        return crypt_key.export_publickeyblob()
    elif blob_type == bType_PRIVATEKEYBLOB:
        return crypt_key.export_privatekeyblob()
    else:
        raise NotImplementedError('blob_type={} not supported'.format(blob_type))


def CryptGetKeyParam(crypt_key, dwParam):
    if dwParam == KP_KEYLEN:
        return crypt_key.key_len * 8
    elif dwParam == KP_ALGID:
        return crypt_key.alg_id
    else:
        raise NotImplementedError('key param {} not implemented'.format(dwParam))


def CryptCreateHash(algid):
    if algid not in algorithm_registry:
        raise NotImplementedError('ALG_ID {:x} not implemented'.format(algid))
    alg_class = algorithm_registry[algid]
    return alg_class()


def CryptHashData(hash_alg, data):
    hash_alg.hash_data(data)


def CryptGetHashParam(hash_alg, dwParam):
    if dwParam == definitions.HP_ALGID:
        return hash_alg.alg_id
    elif dwParam == definitions.HP_HASHVAL:
        return hash_alg.get_hash_val()
    elif dwParam == definitions.HP_HASHSIZE:
        return hash_alg.get_hash_size()
    else:
        return NotImplementedError('hash param {} not implemented'.format(dwParam))


def CryptDeriveKey(hash_alg, algid):
    if algid not in algorithm_registry:
        raise NotImplementedError('ALG_ID {:x} not implemented'.format(algid))
    hash_val = hash_alg.get_hash_val()
    alg_class = algorithm_registry[algid]
    # key derivation for AES and 3DES for non SHA2 family algorithms
    if algid in [CALG_AES_128, CALG_AES_192, CALG_AES_256]:
        key = derive_key_3des_aes(hash_alg)
    else:
        key = hash_val
    key = key[:alg_class.key_len]
    return alg_class(key)