#!/usr/bin/env python3

import logging
import sys
import struct
import os
import binascii
import lznt1
import base64
from wincrypto import CryptCreateHash, CryptHashData, CryptDeriveKey, CryptEncrypt, CryptDecrypt
from wincrypto.definitions import CALG_SHA1, CALG_AES_256
import json
import errno
from cmd2 import ansi


# print('\n\n-----------------------------------------------------\n')
# print('__file__={0:<35}\n__name__={1:<20}\n__package__={2:<20}'.format(__file__,__name__,str(__package__)))
# print('\n-----------------------------------------------------\n')
#

# Default values
COMMANDS_DIRECTORY = "command"
AGENTS_DIRECTORY = "agent"
INSTALLS_DIRECTORY = "install"
PAYLOADS_DIRECTORY = "payload"
# Hardcoded values for the server
DOWNLOADS_DIRECTORY = "downloads"
UPLOADS_DIRECTORY = "uploads"
SENTS_DIRECTORY = "sents"
ADMIN_DIRECTORY = "admin"
BASELINE_CONFIG = "_baseline.json"
AGENT_CONFIG = "AgentConfig.json"
MESSAGE_EXTENSION = ".bin"
CONFIG_EXTENSION = ".json"
# Hardcoded values for the client
AGENT_PASSWORD_ENTRY = "password"
MESSAGE_CONFIG_BINARY_ID = "binary_id"
MESSAGE_CONFIG_AGENT_ID = "agent_id"
MESSAGE_CONFIG_MESSAGE_ID = "message_id"
MESSAGE_CONFIG_RESPONSE = "response"
INITIAL_MESSAGE_ID = "2147483648"
INSTALL_CONFIG_BUFSIZE = 4096
INSTALL_DEFAULT_MAGIC_NUMBER = b"{90DEB964-F2FB-4DB8-9BCA-7D5D10D3A0EB}"
INSTALL_DEFAULT_PASSWORD = "default_password"


my_logger = None

def set_logger(module_name, args):
    """Initialize the logger"""
    global my_logger
    # Check if we are set for debug statements
    logging_level = logging.INFO
    if args.debug:
        logging_level = logging.DEBUG
    # Initialize the logger and the log format
    my_logger = logging.getLogger(module_name)
    my_logger.setLevel(logging_level)
    log_format = logging.Formatter("%(asctime)s - %(filename)s[%(lineno)04d] - %(levelname)s - %(message)s", datefmt="%Y-%m-%d %H:%M:%S")
    # Initialize the filehandler
    fh = logging.FileHandler(module_name+'.log', encoding="UTF-8")
    fh.setLevel(logging_level)
    fh.setFormatter(log_format)
    my_logger.addHandler(fh)
    # Initialize the streamhandler
    fs = logging.StreamHandler(sys.stdout)
    fs.setLevel(logging_level)
    fs.setFormatter(log_format)
    my_logger.addHandler(fs)
    return my_logger



def check_required(options: dict) -> (int, dict):
    """Helper method to determine if there are any required options not set.
    :param options: the dictionary to check (could be primary or secondary options)
    :return: int, the number of required options remaining to set
    :return: dict, the new dict with the filled in values
    """
    required_count = 0
    new_options = options.copy()
    for key, value in options.items():
        # If it's a dictionary, then recursively call check_required
        if isinstance(value, dict):
            sub_required_count, new_options = check_required(value)
            required_count = required_count + sub_required_count
            continue
        # Else, not a new dictionary, so check if the value is required
        elif isinstance(value, str):
            value_type = ''
            if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                value_args = value.split()
                value_prompt = value_args[0][0]
                value_type = value_args[0][1:]
                value_required = value_args[1]
                value_description = value_args[3]
                if "required" == value_required:
                    required_count = required_count + 1
                    print("The option '{:s}' is required (try 'options')".format(key))
    return required_count, new_options

def check_optional(options: dict) -> (int, dict):
    """Helper method to determine if there are any optional options and remove them
    :param options: the dictionary to check (could be primary or secondary options)
    :return: int, the number of optional options
    :return: dict, the new dict with the filled in values
    """
    optional_count = 0
    new_options = options.copy()
    for key, value in options.items():
        # If it's a dictionary, then recursively call check_optional
        if isinstance(value, dict):
            sub_optional_count, new_options = check_optional(new_options)
            optional_count = optional_count + sub_optional_count
            continue
        # Else, not a new dictionary, so check if the value is optional
        elif isinstance(value, str):
            value_type = ''
            if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                value_args = value.split()
                value_prompt = value_args[0][0]
                value_type = value_args[0][1:]
                value_required = value_args[1]
                value_description = value_args[3]
                if "optional" == value_required:
                    optional_count = optional_count + 1
                    print("The option '{:s}' is not set, but it is optional (try 'options')".format(key))
                    new_options.pop(key)
    return optional_count, new_options

def check_auto(options: dict) -> (int, dict):
    """Helper method to determine if there are any auto increment options and set them
    :param options: the dictionary to check (could be primary or secondary options)
    :return: int, the number of auto options
    :return: dict, the new dict with the filled in values
    """
    auto_count = 0
    new_options = options.copy()
    for key, value in options.items():
        # If it's a dictionary, then recursively call check_optional
        if isinstance(value, dict):
            sub_auto_count, new_options = check_auto(new_options)
            auto_count = auto_count + sub_auto_count
            continue
        # Else, not a new dictionary, so check if the value is optional
        elif isinstance(value, str):
            value_type = ''
            if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                value_args = value.split()
                value_prompt = value_args[0][0]
                value_type = value_args[0][1:]
                value_required = value_args[1]
                value_description = value_args[3]
                if "auto" == value_required:
                    auto_count = auto_count + 1
                    # print("Auto-incrementing the value for option '{:s}'...".format(key))
                    if not os.path.isfile(key):
                        print("Warning: Auto-increment file for option '{:s}' is not found".format(key))
                        auto_calc_value = 0
                    else:
                        with open(key, "r") as input_fd:
                            auto_calc_string = input_fd.read()
                        auto_calc_value = int(auto_calc_string)
                        auto_calc_value = auto_calc_value + 1
                    # print("Used auto incremented value of {:d} for option '{:s}'".format(auto_calc_value, key))
                    with open(key, "w") as output_fd:
                        output_fd.write(str(auto_calc_value))
                    new_options[key] = auto_calc_value
    return auto_count, new_options

def check_random(options: dict) -> (int, dict):
    """Helper method to determine if there are any random options and set them
    :param options: the dictionary to check (could be primary or secondary options)
    :return: int, the number of random options
    :return: dict, the new dict with the filled in values
    """
    random_count = 0
    new_options = options.copy()
    for key, value in options.items():
        # If it's a dictionary, then recursively call check_optional
        if isinstance(value, dict):
            sub_random_count, new_options = check_auto(new_options)
            random_count = random_count + sub_random_count
            continue
        # Else, not a new dictionary, so check if the value is optional
        elif isinstance(value, str):
            value_type = ''
            if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                value_args = value.split()
                value_prompt = value_args[0][0]
                value_type = value_args[0][1:]
                value_required = value_args[1]
                value_description = value_args[3]
                if "random" == value_required:
                    random_count = random_count + 1
                    random_value = None
                    # print("Generating a random value for option '{:s}'...".format(key))
                    if value_type.startswith('string'):
                        value_type_size = 38
                        value_type_size_str = value_type[len("string"):]
                        if len(value_type_size_str) > 0:
                            # the size of the string buffer is specified
                            value_type_size = int(value_type_size_str)
                        value_type_range = string.ascii_letters + string.punctuation
                        random_value = ''.join(random.choice(value_type_range) for i in range(value_type_size))
                    elif value_type.startswith('int'):
                        value_type_range_str = value_type[len("int"):]
                        value_type_range = value_type_range_str.split("-")
                        random_value = random.randint(value_type_range[0], value_type_range[1])
                    else:
                        print("Do not know how to create a random '{:s}'".format(value_type))
                        return None
                    # print("Generated random value of {}".format(random_value))
                    new_options[key] = random_value
    return random_count, new_options


def get_agent_password_from_agent_path(path: str) -> str:
    """Helper method to determine the agent password from the configuration json in the specified path.
           :param path: the current path
           :return: str, the password from the current path or None if it could not determine a password
           """
    current_password = None
    full_path = os.path.abspath(path)
    current_config_filename = os.path.join(full_path, ADMIN_DIRECTORY, AGENT_CONFIG)
    if not os.path.isfile(current_config_filename):
        raise FileNotFoundError(current_config_filename)
    with open(current_config_filename, "r") as input_fd:
        current_config_json = input_fd.read()
    current_config_dict = json.loads(current_config_json)
    if AGENT_PASSWORD_ENTRY not in current_config_dict:
        raise KeyError(AGENT_PASSWORD_ENTRY)
    current_password = current_config_dict[AGENT_PASSWORD_ENTRY]
    return current_password



def cJSON_CreateBytes( bytesbuffer ):
    json_bytesbuffer = '64=='
    b64encoded_bytesbuffer = base64.standard_b64encode(bytesbuffer)
    json_bytesbuffer = json_bytesbuffer + b64encoded_bytesbuffer.decode()
    return json_bytesbuffer

def cJSON_GetBytesValue( encoded_string ):
    b64decoded_bytesbuffer = ''
    if '64==' == encoded_string[:4]:
        b64encoded_string = encoded_string[4:]
        b64decoded_bytesbuffer = base64.standard_b64decode(b64encoded_string)
    return b64decoded_bytesbuffer


def Compress_Buffer( input_buffer ):
    if my_logger:
        my_logger.debug('Compress_Buffer::len(input_buffer):      %d', len(input_buffer))
        my_logger.debug('Compress_Buffer::input_buffer:           %s', binascii.hexlify(input_buffer)[:20])
    compressed_buffer = lznt1.compress(input_buffer)
    if my_logger:
        my_logger.debug('Compress_Buffer::len(compressed_buffer): %d', len(compressed_buffer))
        my_logger.debug('Compress_Buffer::compressed_buffer:      %s', binascii.hexlify(compressed_buffer)[:20])
    compressed_buffer_size = struct.pack('<I', len(compressed_buffer))
    output_buffer = compressed_buffer_size + compressed_buffer
    return bytearray(output_buffer)

def Decompress_Buffer( input_buffer ):
    if my_logger:
        my_logger.debug('Decompress_Buffer::len(input_buffer):        %d', len(input_buffer))
        my_logger.debug('Decompress_Buffer::input_buffer:             %s', binascii.hexlify(input_buffer)[:20])
    compressed_buffer_size = input_buffer[:4]
    compressed_buffer = input_buffer[4:]
    decompressed_buffer = lznt1.decompress(compressed_buffer)
    if my_logger:
        my_logger.debug('Decompress_Buffer::len(decompressed_buffer): %d', len(decompressed_buffer))
        my_logger.debug('Decompress_Buffer::decompressed_buffer:      %s', binascii.hexlify(decompressed_buffer)[:20])
    return decompressed_buffer


def Encyrpt_Buffer( input_buffer, encryption_password ):
    if my_logger:
        my_logger.debug('Encyrpt_Buffer::len(input_buffer):     %d', len(input_buffer))
        my_logger.debug('Encyrpt_Buffer::input_buffer:          %s', binascii.hexlify(input_buffer)[:20])
    sha1_hasher = CryptCreateHash(CALG_SHA1)
    CryptHashData(sha1_hasher, encryption_password)
    aes_key = CryptDeriveKey(sha1_hasher, CALG_AES_256)
    encrypted_buffer = CryptEncrypt(aes_key, input_buffer)
    if my_logger:
        my_logger.debug('Encyrpt_Buffer::len(encrypted_buffer): %d', len(encrypted_buffer))
        my_logger.debug('Encyrpt_Buffer::encrypted_buffer:      %s', binascii.hexlify(encrypted_buffer)[:20])
    return encrypted_buffer

def Decyrpt_Buffer( input_buffer, encryption_password ):
    if my_logger:
        my_logger.debug('Decyrpt_Buffer::len(input_buffer):     %d', len(input_buffer))
        my_logger.debug('Decyrpt_Buffer::input_buffer:          %s', binascii.hexlify(input_buffer)[:20])
    sha1_hasher = CryptCreateHash(CALG_SHA1)
    CryptHashData(sha1_hasher, encryption_password)
    aes_key = CryptDeriveKey(sha1_hasher, CALG_AES_256)
    decrypted_buffer = CryptDecrypt(aes_key, input_buffer)
    if my_logger:
        my_logger.debug('Decyrpt_Buffer::len(decrypted_buffer): %d', len(decrypted_buffer))
        my_logger.debug('Decyrpt_Buffer::decrypted_buffer:      %s', binascii.hexlify(decrypted_buffer)[:20])
    return decrypted_buffer


def Encode_Buffer( input_buffer ):
    if my_logger:
        my_logger.debug('Encode_Buffer::len(input_buffer):   %d', len(input_buffer))
        my_logger.debug('Encode_Buffer::input_buffer:        %s', binascii.hexlify(input_buffer)[:20])
    encoded_buffer = base64.standard_b64encode(input_buffer)
    if my_logger:
        my_logger.debug('Encode_Buffer::len(encoded_buffer): %d', len(encoded_buffer))
        my_logger.debug('Encode_Buffer::encoded_buffer:      %s', binascii.hexlify(encoded_buffer)[:20])
    return encoded_buffer

def Decode_Buffer( input_buffer ):
    if my_logger:
        my_logger.debug('Decode_Buffer::len(input_buffer):   %d', len(input_buffer))
        my_logger.debug('Decode_Buffer::input_buffer:        %s', binascii.hexlify(input_buffer)[:20])
    decoded_buffer = base64.standard_b64decode(input_buffer)
    if my_logger:
        my_logger.debug('Decode_Buffer::len(decoded_buffer): %d', len(decoded_buffer))
        my_logger.debug('Decode_Buffer::decoded_buffer:      %s', binascii.hexlify(decoded_buffer)[:20])
    return decoded_buffer


def CompressEncryptEncode( plaintext_buffer, c2_password ):
    # Compress the command_message_string
    compressed_buffer = Compress_Buffer(plaintext_buffer)
    # Encrypt the compressed_buffer
    encrypted_buffer = Encyrpt_Buffer(compressed_buffer, c2_password)
    # Encode the encrypted_buffer
    encoded_command_message_string = Encode_Buffer(encrypted_buffer)
    return encoded_command_message_string

def DecodeDecryptDecompress( encoded_command_message_string, c2_password ):
    # Decode the encoded_command_message_string
    decoded_buffer = Decode_Buffer(encoded_command_message_string)
    # Decrypt the decoded_buffer
    decrypted_buffer = Decyrpt_Buffer(decoded_buffer,c2_password)
    # Deompress the decrypted_buffer
    decompressed_buffer = Decompress_Buffer(decrypted_buffer)
    return decompressed_buffer

def UpdateBinary(input_file_name, output_file_name, input_buffer, find_buffer_sequence, find_buffer_size):
    """Update a data_blob inside a binary given a magic bytes sequence to find"""

    if my_logger:
        my_logger.debug('UpdateBinary::input_file_name:         %s', input_file_name)
        my_logger.debug('UpdateBinary::output_file_name:        %s', output_file_name)
        my_logger.debug('UpdateBinary::input_buffer:            %s', binascii.hexlify(input_buffer)[:20])
        my_logger.debug('UpdateBinary::len(input_buffer):       %d', len(input_buffer))
        my_logger.debug('UpdateBinary::find_buffer_sequence:    %s', binascii.hexlify(find_buffer_sequence)[:20])
        my_logger.debug('UpdateBinary::find_buffer_size:        %d', find_buffer_size)

    # Initialize variables
    input_file_data = ''
    output_file_data = bytearray()

    # Perform sanity checks
    if len(input_buffer) > find_buffer_size:
        if my_logger:
            my_logger.error('UpdateBinary::Input buffer is too large (%d > %d)', len(input_buffer), find_buffer_size)
        raise BufferError()
    if not os.path.isfile(input_file_name):
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), input_file_name)

    # Read in the input_file_data
    with open(input_file_name, 'rb') as inputfd:
        input_file_data = inputfd.read()
    if my_logger:
        my_logger.debug('UpdateBinary::len(input_file_data):    %d', len(input_file_data))

    # Find the find_buffer_sequence in the input_file_data
    find_buffer_offset = input_file_data.find(find_buffer_sequence)
    if -1 == find_buffer_offset:
        if my_logger:
            my_logger.error('UpdateBinary::find_buffer_sequence not found')
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), input_file_name)
    if my_logger:
        my_logger.debug('UpdateBinary::find_buffer_offset:      %d', find_buffer_offset)

    # Replace the find_buffer_sequence with the input_buffer
    output_file_data = output_file_data + input_file_data[:find_buffer_offset]
    output_file_data = output_file_data + input_buffer
    output_file_data = output_file_data + input_file_data[find_buffer_offset + len(input_buffer):]
    if my_logger:
        my_logger.debug('UpdateBinary::output_file_data:        %s', binascii.hexlify(output_file_data)[:20])
        my_logger.debug('UpdateBinary::len(output_file_data):   %d', len(output_file_data))

    # Write the updated output_file_data to output_file_name
    with open(output_file_name, 'wb') as outputfd:
        outputfd.write(output_file_data)
    if my_logger:
        my_logger.info('UpdateBinary::Successfully updated %s as %s', input_file_name, output_file_name)

    return output_file_name, len(output_file_data)





# def prompt_json(template_filename):
#     """Reads in a JSON template file and fills in the template by prompting the user"""
#
#     # Initialize the variables
#     template_json_dict = dict()
#
#     # Load the command config JSON
#     with open(template_filename, 'r') as input_fd:
#         template_file_buffer = input_fd.read()
#     template_json_dict = json.loads(template_file_buffer)
#
#     # Check if the configuration references any filenames
#     for key, value in template_json_dict.items():
#         if isinstance(value, basestring):
#             # Check if we should prompt for the value
#             if value.startswith('?') or value.startswith('!'):
#                 prompt_type = value[1:]
#                 prompt_input = raw_input("%s (%s): " % (key,prompt_type))
#                 if prompt_type == 'int':
#                     value_input = int(prompt_input)
#                 elif prompt_type == 'string':
#                     value_input = str(prompt_input)
#                 elif prompt_type == 'file':
#                     file_input = str(prompt_input)
#                     file_contents = ''
#                     if not os.path.isfile(file_input):
#                         raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), file_input)
#                     with open(file_input, 'rb') as file_input_fd:
#                         file_contents = file_input_fd.read()
#                     value_input = file_contents
#                 else:
#                     raise TypeError('Unknown prompt_type')
#                 # Check if we should encode the value as a byte buffer
#                 if value.startswith('!'):
#                     value_input = cJSON_CreateBytes(value_input)
#             # Check if we should auto-calculate the value
#             elif value.startswith('#'):
#                 prompt_type = value[1:]
#                 file_input = str(prompt_type)
#                 if not os.path.isfile(file_input):
#                     raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), file_input)
#                 with open(file_input, 'rb') as file_input_fd:
#                     file_contents = file_input_fd.read()
#                 value_input = int(file_contents)
#                 value_input = value_input + 1
#                 with open(file_input, 'wb') as file_input_fd:
#                     file_input_fd.write(str(value_input))
#             # Else just use the value specified
#             else:
#                 value_input = value
#             template_json_dict[key] = value_input
#         # End if the value in the command config file is a basestring
#     # End for each key, value pair in the command config file
#     return template_json_dict
def prompt_json(json_object):
    """Reads in a JSON template file and fills in the template by prompting the user"""

    # Check if current object is a list
    if isinstance(json_object, list):
        list_index = 0
        for json_list_object in json_object:
            json_object[list_index] = prompt_json(json_list_object)
            list_index = list_index + 1
        return json_object

    if not isinstance(json_object, dict):
        raise TypeError("json_object is not a dict")

    set_dict = json_object.copy()

    # Check if the configuration references any filenames
    for set_key, original_value in json_object.items():
        set_value = ''
        if isinstance(original_value, str):
            value_args = original_value.split()
            value_prompt = value_args[0][0]
            value_type = value_args[0][1:]
            value_required = value_args[1]
            value_description = ' '.join(value_args[3:])
            # Check if we should prompt for the value
            if value_prompt == '?' or value_prompt == '!':
                print("option:" + ansi.style("{:s}".format(set_key), fg='blue'), end='')
                print(" type:" + ansi.style("{:s}".format(value_type), fg='blue'), end='')
                if value_required == "required":
                    print(" (" + ansi.style("{:s}".format(value_required), fg='red') + ")", end='')
                else:
                    print(" (" + ansi.style("{:s}".format(value_required), fg='yellow') + ")", end='')
                print(" - " + ansi.style("{:s}".format(value_description), fg='green'))
                while set_value == "":
                    set_value = input("{:s} : ".format(set_key))
                    if set_value == "" and value_required == "optional":
                        break
                    elif set_value == "":
                        print(ansi.style("Option {:s} is required".format(set_key), fg='red'))
                if set_value == "":
                    set_dict.pop(set_key)
                    continue
                if "int" == value_type:
                    set_dict[set_key] = int(set_value)
                elif value_type.startswith("string"):
                    value_type_size_str = value_type[len("string"):]
                    if len(value_type_size_str) > 0:
                        # the size of the string buffer is specified
                        value_type_size = int(value_type_size_str)
                        if len(set_value) > value_type_size:
                            raise BufferError("Input too large for string buffer")
                    set_dict[set_key] = str(set_value)
                elif "file" == value_type:
                    file_input = str(set_value)
                    file_contents = ''
                    if not os.path.isfile(file_input):
                        raise FileNotFoundError("{}".format(set_value))
                    with open(file_input, 'rb') as file_input_fd:
                        file_contents = file_input_fd.read()
                    if value_prompt == '!':
                        file_contents = cJSON_CreateBytes(file_contents)
                    set_dict[set_key] = file_contents
                elif "list_string" == value_type:
                    if isinstance(set_dict[set_key], list):
                        key_list = set_dict[set_key]
                    else:
                        key_list = []
                    key_list.append(str(set_value))
                    set_dict[set_key] = key_list
                elif "list_int" == value_type:
                    if isinstance(set_dict[set_key], list):
                        key_list = set_dict[set_key]
                    else:
                        key_list = []
                    key_list.append(int(set_value))
                    set_dict[set_key] = key_list
                else:
                    raise TypeError("Unknown value_type")
            # Check if we should auto-calculate the value
            elif value_prompt == '#':
                # Auto increment, so ignore
                continue
            # Else we have a hardcoded str
            else:
                # Ignore
                continue
        # Else not a str, so most likely a hardcoded int
        else:
            # Ignore
            continue
    # End for each key, value pair in the json object
    return set_dict



def print_json(json_object,indent = 0):
    # if my_logger:
    #     my_logger.debug('print_json::json_object:  %s', json_object)
    #     my_logger.debug('print_json::indent:      %d', indent)


    if isinstance(json_object, list):
        list_index = 0
        # print("json_object is list of size:",len(json_object))
        for json_list_object in json_object:
            print("  " * indent, end ="")
            print("[{}] : ".format(list_index))
            print_json(json_list_object, indent+1)
            list_index += 1
        return

    if not isinstance(json_object, dict):
        raise TypeError("Not a json_object")

    if len(json_object.items()) == 0:
        #print('None')
        return

    # Loop through all the items in the JSON json_object

    for key, value in json_object.items():
        # First check if it'ss a Base64 string that needs to be decoded
        if isinstance(value,str):
            if value.startswith('64=='):
                value = cJSON_GetBytesValue(value)
                try:
                    value_json = json.loads(value)
                except:
                    value_json = value
                value = value_json
        # Next check if it's a list and if it isn't make it a list,
        #  so that we can treat all items the same
        if isinstance(value, list):
            values = value
        else:
            values = [value]
        values_index = -1
        # Loop through all items in the list
        for value in values:
            # Increment list counter
            values_index = values_index + 1
            # Create the indent for the current item
            print('  ' * indent, end='')
            # If it's a json_object, then recursively call print_json
            if isinstance(value, dict):
                if len(values) > 1:
                    print("{0!s}[{1:d}] : ".format(key,values_index))
                else:
                    print("{0!s} : ".format(key))
                print_json(value, indent + 1)
                continue
            # Else, not a new json_object, so print the value
            value_string = ''
            # Check if int
            if isinstance(value, int):
                value_string = str(value)
            # Check if str
            elif isinstance(value, str):
                value_type = ''
                if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                    # If it starts with one of these values, then the key value pair has not been set
                    value_string = value[1:]
                    value_type = value[0]
                else:
                    value_string = value
            else:
                value_string = str(value)
            # Truncate the value if it is too large
            if len(value_string) > 120:
                # But truncate the value if it is too big
                value_string = value_string[:120] + " ({:d} bytes)".format(len(value))
            print("{0!s} : {1!s}".format(key, value_string))