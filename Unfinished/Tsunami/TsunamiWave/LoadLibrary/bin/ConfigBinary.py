import logging
import sys
import argparse
import struct
import os
import errno
import binascii
import lznt1
import base64
import hashlib
from wincrypto import CryptCreateHash, CryptHashData, CryptDeriveKey, CryptEncrypt
from wincrypto.definitions import CALG_SHA1, CALG_AES_256
import cstruct
import string
import random
import glob
import json

MODULE_NAME = 'ConfigBinary'
CONFIG_BUFSIZE = 4096
CONFIG_DEFAULT_MAGIC_NUMBER = "{90DEB964-F2FB-4DB8-9BCA-7D5D10D3A0EB}"
CONFIG_DEFAULT_PASSWORD = "{E3AEA3F6-D548-4989-9A42-80BAC9321AE0}"



def set_logger(args):
    """Initialize the logger"""

    # Check if we are set for debug statements
    logging_level = logging.INFO
    if args.debug:
        logging_level = logging.DEBUG

    # Initialize the logger and the log format
    logger = logging.getLogger(MODULE_NAME)
    logger.setLevel(logging_level)
    log_format = logging.Formatter("%(asctime)s - %(filename)s[%(lineno)04d] - %(levelname)s - %(message)s")

    # Initialize the filehandler
    fh = logging.FileHandler(MODULE_NAME+'.log', encoding="UTF-8")
    fh.setLevel(logging_level)
    fh.setFormatter(log_format)
    logger.addHandler(fh)

    # Initialize the streamhandler
    fs = logging.StreamHandler(sys.stdout)
    fs.setLevel(logging_level)
    fs.setFormatter(log_format)
    logger.addHandler(fs)

    return logger




def parse_command_line():
    """Parse the command-line arguments"""

    parser = argparse.ArgumentParser(description=MODULE_NAME)

    parser.add_argument("--config", dest="config", required=False, help="The configuration settings (JSON)")
    parser.add_argument("--input", dest="input", required=True, help="Input binary filename")
    parser.add_argument("--output", dest="output", required=True, help="Output binary filename")
    parser.add_argument("--debug", dest="debug", action='store_true',  help="Display debug print statements")
    parser.add_argument("--password", dest="password", help="Password for encrypted binary buffer")
    parser.add_argument("--magic", dest="magic", help="Magic number used to find the binary buffer")
    parser.add_argument("--random", dest="random_password", action='store_true', help="Generate a random encryption password")
    parser.set_defaults(debug=False, password=CONFIG_DEFAULT_PASSWORD, random_password=False, magic=CONFIG_DEFAULT_MAGIC_NUMBER)
    args = parser.parse_args()

    if args.random_password:
        args.password = ''.join(random.choice(string.printable) for _ in xrange(len(CONFIG_DEFAULT_PASSWORD)))

    if len(args.password) > len(CONFIG_DEFAULT_PASSWORD):
        raise IndexError( 'password too long', args.password)

    if args.config:
        if not os.path.isfile( args.config ):
            raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.config)

    if not os.path.isfile( args.input ):
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)

    return args




def main(args):
    """Pack the configuration information/bytes/struct into the binary"""

    logger = set_logger(args)

    logger.debug('Start of main')
    if args.config:
        logger.debug('args.config:    %s', args.config)
    logger.debug('args.input:     %s', args.input)
    logger.debug('args.output:    %s', args.output)

	# Read in the specified configuration json
    if args.config:
        with open(args.config, 'r') as configfd:
            configuration = configfd.read()

	# If a specific json was not specified, use all json's from current directory
    else:
        config_dict = dict()
        for config_file in glob.glob("*.json"):
            logger.debug('parsing config: %s', config_file)
            with open(config_file, 'r') as configfd:
                config_json_str = configfd.read()
                config_json_dict = json.loads(config_json_str)
                config_dict.update(config_json_dict)
        configuration = json.dumps(config_dict)

	# Display overall configuraton
    logger.info('Using configuration: %s', configuration)


    # Pack the configuration data into a structure
    configured_data_bytes = configuration
    logger.debug('len(configured_data_bytes): %d', len(configured_data_bytes))
    logger.debug('configured_data_bytes:\n%s', binascii.hexlify(configured_data_bytes))


    # Compress the configuration structure
    compressed_configured_data_bytes = lznt1.compress(configured_data_bytes)
    logger.debug('compressed_configured_data_bytes_size: %d', len(compressed_configured_data_bytes))
    logger.debug('compressed_configured_data_bytes:\n%s', binascii.hexlify(compressed_configured_data_bytes))
    packed_compressed_buffer_size = struct.pack('<I', len(compressed_configured_data_bytes))
    compressed_configured_data_bytes = packed_compressed_buffer_size + compressed_configured_data_bytes


    # Encrypt the compressed configuration structure
    logger.debug('decrypted_data_bytes_size: %d', len(compressed_configured_data_bytes))
    logger.debug('decrypted_data_bytes:\n%s', binascii.hexlify(compressed_configured_data_bytes))
    sha1_hasher = CryptCreateHash(CALG_SHA1)
    CryptHashData(sha1_hasher, args.password)
    aes_key = CryptDeriveKey(sha1_hasher, CALG_AES_256)
    encrypted_data_bytes = CryptEncrypt(aes_key, compressed_configured_data_bytes)
    logger.debug('encrypted_data_bytes_size: %d', len(encrypted_data_bytes))
    logger.debug('encrypted_data_bytes:\n%s', binascii.hexlify(encrypted_data_bytes))


    # Encode the encrypted configuration structure
    encoded_configured_data_bytes = base64.standard_b64encode(encrypted_data_bytes)
    logger.debug('encoded_configured_data_bytes_size: %d', len(encoded_configured_data_bytes))
    logger.debug('encoded_configured_data_bytes:\n%s', binascii.hexlify(encoded_configured_data_bytes))


    # Check the size of the packed buffers
    if len(encoded_configured_data_bytes) > CONFIG_BUFSIZE:
        logger.error('packed config buffer is larger than allocated buffer in binary %d > %d', len(encoded_configured_data_bytes), CONFIG_BUFSIZE)
        raise BufferError()


    # Read in the unconfigured binary
    logger.info('Reading unconfigured binary from %s', args.input)
    input_bytes = None
    with open(args.input, 'rb') as inputfd:
        input_bytes = inputfd.read()
    logger.debug('Length of input binary:  %d', len(input_bytes))


    # Find the config struct in the input binary using the magic byte sequence
    config_magic_byte_string = binascii.unhexlify(binascii.hexlify(bytearray(args.magic)))
    little_endian_magic_byte_string = config_magic_byte_string
    logger.debug('Finding magic byte sequence: %s', binascii.hexlify(little_endian_magic_byte_string))
    config_struct_offset = input_bytes.find(little_endian_magic_byte_string)
    if -1 == config_struct_offset:
        logger.error('Magic byte sequence not found: %s', config_magic_byte_string)
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)
    logger.debug('Configuration structure index: %d', config_struct_offset)


    # Replace the empty/unconfigured structure with the configured/packed byte sequence
    output_bytes = ''
    output_bytes = output_bytes + input_bytes[:config_struct_offset]
    output_bytes = output_bytes + encoded_configured_data_bytes
    output_bytes = output_bytes + input_bytes[config_struct_offset + len(encoded_configured_data_bytes):]
    logger.debug('Length of output binary: %d', len(output_bytes))


    # Replace default password if necessary
    if args.password != CONFIG_DEFAULT_PASSWORD:
        logger.debug('Replacing the default key with the new key')
        binary_password_index = output_bytes.find(CONFIG_DEFAULT_PASSWORD)
        if -1 == binary_password_index:
            logger.error('binary_password_index not found')
            raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.output)
        logger.debug('binary_password_index: %d', binary_password_index)
        new_output_bytes = ''
        new_output_bytes = new_output_bytes + output_bytes[:binary_password_index]
        new_output_bytes = new_output_bytes + args.password
        num_padding = ( len(CONFIG_DEFAULT_PASSWORD) - len(args.password) )
        new_output_bytes = new_output_bytes + ''.join('\0' for _ in xrange( num_padding ) )
        new_output_bytes = new_output_bytes + output_bytes[binary_password_index+len(CONFIG_DEFAULT_PASSWORD):]
        output_bytes = new_output_bytes


    # Write the configured binary to file
    with open(args.output, 'wb') as outputfd:
        outputfd.write(output_bytes)
    logger.info('Saved configured binary to %s', args.output)


if __name__ == "__main__":
    args = parse_command_line()
    sys.exit(main(args))