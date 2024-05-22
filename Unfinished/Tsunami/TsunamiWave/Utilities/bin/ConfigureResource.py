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

import ctypes
import ctypes.wintypes


# ctypes functions
LoadLibraryEx = ctypes.windll.kernel32.LoadLibraryExW
FreeLibrary = ctypes.windll.kernel32.FreeLibrary
EnumResourceNames = ctypes.windll.kernel32.EnumResourceNamesA
EnumResourceNameCallback = ctypes.WINFUNCTYPE(
	ctypes.wintypes.BOOL,
	ctypes.wintypes.HMODULE, ctypes.wintypes.LONG,
	ctypes.wintypes.LONG, ctypes.wintypes.LONG)
FindResource = ctypes.windll.kernel32.FindResourceA
LoadResource = ctypes.windll.kernel32.LoadResource
FreeResource = ctypes.windll.kernel32.FreeResource
SizeofResource = ctypes.windll.kernel32.SizeofResource
LockResource = ctypes.windll.kernel32.LockResource
UnlockResource = lambda x: None # hehe
CloseHandle = ctypes.windll.kernel32.CloseHandle
LoadString = ctypes.windll.user32.LoadStringA
BeginUpdateResource = ctypes.windll.kernel32.BeginUpdateResourceA
EndUpdateResource = ctypes.windll.kernel32.EndUpdateResourceA
UpdateResource = ctypes.windll.kernel32.UpdateResourceA
GetLastError = ctypes.windll.kernel32.GetLastError

# resource types
RT_CURSOR = 1						# Hardware-dependent cursor resource.
RT_BITMAP = 2						# Bitmap resource.
RT_ICON = 3							# Hardware-dependent icon resource.
RT_MENU = 4							# Menu resource.
RT_DIALOG = 5						# Dialog box.
RT_STRING = 6						# String-table entry.
RT_FONTDIR = 7						# Font directory resource.
RT_FONT = 8							# Font resource.
RT_ACCELERATOR = 9					# Accelerator table.
RT_RCDATA = 10						# Application-defined resource (raw data.)
RT_MESSAGETABLE = 11				# Message-table entry.
RT_VERSION = 16						# Version resource.
RT_DLGINCLUDE = 17					# Allows a resource editing tool to associate a string with an .rc file. Typically, the string is the name of the header file that provides symbolic names. The resource compiler parses the string but otherwise ignores the value. For example,
RT_PLUGPLAY = 19					# Plug and Play resource.
RT_VXD = 20							# VXD.
RT_ANICURSOR = 21					# Animated cursor.
RT_ANIICON = 22						# Animated icon.
RT_HTML = 23						# HTML resource.
RT_MANIFEST = 24					# Side-by-Side Assembly Manifest.
RT_GROUP_CURSOR = RT_CURSOR + 11	# Hardware-independent cursor resource.
RT_GROUP_ICON = RT_ICON + 11		# Hardware-independent icon resource.

# LoadLibrary flags
DONT_RESOLVE_DLL_REFERENCES = 0x1
LOAD_LIBRARY_AS_DATAFILE = 0x2
LOAD_LIBRARY_AS_IMAGE_RESOURCE = 0x20

# locales
LOCAL_EN_US = 1033




MODULE_NAME = 'StorageResource'
#STRUCT_CONFIG_DATA_SIZE_FMT = '<I'
CONFIG_DEFAULT_RESOURCE_TYPE = 10
CONFIG_DEFAULT_RESOURCE_NAME = 1
CONFIG_DEFAULT_PASSWORD = "{E3AEA3F6-D548-4989-9A42-80BAC9321AE1}"



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
    parser.add_argument("--password", dest="password", help="Password for encrypted config buffer")
    parser.add_argument("--rtype", dest="rtype", help="Resource type of the config buffer")
    parser.add_argument("--rname", dest="rname", help="Resource name of the config buffer")
    parser.add_argument("--random", dest="random_password", action='store_true', help="Generate a random encryption password")
    parser.set_defaults(debug=False, password=CONFIG_DEFAULT_PASSWORD, random_password=False, rtype=10, rname=1)
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


class ResourceEditor(object):

    def __init__(self, filename):
        self.filename = filename

    def update_resources(self, resources):
        language = LOCAL_EN_US
        update_handle = BeginUpdateResource(self.filename, False)
        print "BeginUpdateResource returned: %s" % update_handle
        for type, name, data in resources:
            print "resource:", type, name, language, len(data)
            ret = UpdateResource(update_handle, type, name, language, data, len(data))
        print "UpdateResource returned: %s" % ret
        ret = EndUpdateResource(update_handle, False)
        print "EndUpdateResource returned: %s" % ret
        return ret == 1

    def get_resources(self, resource_types):
        """Retrieves the manifest(s) embedded in the current executable"""
        self.module = LoadLibraryEx(self.filename, 0, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE | LOAD_LIBRARY_AS_IMAGE_RESOURCE)
        if self.module == 0:
            raise Exception("Can't read resources from file %s" % self.filename)

        manifests = []

        def callback(hModule, lpType, lpName, lParam):
            hResource = FindResource(hModule, lpName, lpType)
            # print self.get_resource_string(hResource)
            size = SizeofResource(hModule, hResource)
            hData = LoadResource(hModule, hResource)
            try:
                ptr = LockResource(hData)
                try:
                    manifests.append((lpType, lpName, ctypes.string_at(ptr, size)))
                finally:
                    UnlockResource(hData)
            finally:
                FreeResource(hData)
            return True

        for resource_type in resource_types:
            EnumResourceNames(self.module, resource_type, EnumResourceNameCallback(callback), None)
        FreeLibrary(self.module)
        return manifests


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


    ## Pack the size of the encoded configuration structure into DWORD size
    #packed_config_buffer = encoded_configured_data_bytes
    #packed_config_buffer_size = struct.pack(STRUCT_CONFIG_DATA_SIZE_FMT, len(packed_config_buffer))
    #logger.debug( 'packed_config_buffer_size: %s', binascii.hexlify(packed_config_buffer_size) )
    #logger.debug('packed_config_buffer:\n%s', binascii.hexlify(packed_config_buffer))

    #packed_config_struct = packed_config_buffer_size + packed_config_buffer


    # Read in the unconfigured binary
    logger.info('Reading unconfigured binary from %s', args.input)
    input_bytes = None
    with open(args.input, 'rb') as inputfd:
        input_bytes = inputfd.read()
    logger.debug('Length of input binary:  %d', len(input_bytes))

    #
    #
    # # Find the config struct in the input binary using the magic byte sequence
    # config_magic_byte_string = binascii.unhexlify(binascii.hexlify(bytearray(args.magic)))
    # little_endian_magic_byte_string = config_magic_byte_string
    # logger.debug('Finding magic byte sequence: %s', binascii.hexlify(little_endian_magic_byte_string))
    # config_struct_offset = input_bytes.find(little_endian_magic_byte_string)
    # if -1 == config_struct_offset:
    #     logger.error('Magic byte sequence not found: %s', config_magic_byte_string)
    #     raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)
    # logger.debug('Configuration structure index: %d', config_struct_offset)
    #
    #
    # # Replace the empty/unconfigured structure with the configured/packed byte sequence
    # output_bytes = ''
    # output_bytes = output_bytes + input_bytes[:config_struct_offset]
    # output_bytes = output_bytes + packed_config_buffer_size
    # output_bytes = output_bytes + packed_config_buffer
    # output_bytes = output_bytes + input_bytes[config_struct_offset+len(packed_config_buffer)+len(packed_config_buffer_size):]
    # logger.debug('Length of output binary: %d', len(output_bytes))
    #
    #

    output_bytes = input_bytes

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



    # Update the resource section of the output file with the configuration buffer
    resources = []
    #resources.append((args.rtype, args.rname, packed_config_struct))
    resources.append((args.rtype, args.rname, encoded_configured_data_bytes))
    re_to = ResourceEditor(args.output)
    re_to.update_resources(resources)


if __name__ == "__main__":
    args = parse_command_line()
    sys.exit(main(args))
