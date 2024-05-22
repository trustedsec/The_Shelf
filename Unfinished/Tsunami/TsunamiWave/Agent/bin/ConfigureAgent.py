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
import tsunami

MODULE_NAME = 'ConfigureAgent'
DEFAULT_PASSWORD = "{E3AEA3F6-D548-4989-9A42-80BAC9321AE0}"


def parse_command_line():
    """Parse the command-line arguments"""
    # Create and add arguments to the parser
    parser = argparse.ArgumentParser(description=MODULE_NAME)
    parser.add_argument("--config", dest="config", required=True, help="The configuration settings (JSON)")
    parser.add_argument("--input", dest="input", required=True, help="Input binary filename")
    parser.add_argument("--output", dest="output", required=True, help="Output binary filename")
    parser.add_argument("--debug", dest="debug", action='store_true', help="Display debug print statements")
    parser.add_argument("--password", dest="password", help="Password for encrypted binary buffer")
    parser.add_argument("--random", dest="random_password", action='store_true',
                        help="Generate a random encryption password")
    parser.set_defaults(debug=False, password=DEFAULT_PASSWORD, random_password=False)
    # Parse the arguments
    args = parser.parse_args()
    # Sanity check on arguments
    if args.random_password:
        args.password = ''.join(random.choice(string.printable) for _ in iter(range(len(DEFAULT_PASSWORD))))
    if len(args.password) > len(DEFAULT_PASSWORD):
        raise IndexError('password too long', args.password)
    if args.config:
        if not os.path.isfile(args.config):
            raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.config)
    if not os.path.isfile(args.input):
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)
    return args


def main(args):
    """Pack the configuration information/bytes/struct into the binary"""

    logger = tsunami.set_logger(MODULE_NAME,args)
    logger.debug('Start of main')

    # Initialize variables
    configuration_buffer = ''
    configuration_input = ''
    configuration_dict = dict()

    # Load the configuration JSON
    with open(args.config, 'r') as configfd:
        configuration_input = configfd.read()
    configuration_dict = json.loads(configuration_input)

    # Get some of the settings from the config like binary_id and agent_id
    agent_id = configuration_dict['agent_id']
    binary_id = configuration_dict['binary_id']
    logger.debug('agent_id:            %d', agent_id)
    logger.debug('binary_id:           %d', binary_id)

    # Create the file destination
    input_filename = args.input
    logger.debug('input_filename:      %s', input_filename)
    output_filename = args.output
    logger.debug('output_filename:     %s', output_filename)
    config_output_filename = 'AgentConfig.json'
    logger.debug('config_output_filename: %s', config_output_filename)

    # Check if we need to update the password
    config_password = args.password
    default_password = DEFAULT_PASSWORD
    logger.debug('default_password: %s', default_password)
    if config_password != default_password:
        logger.debug('config_password: %s', config_password)
        configuration_dict['password'] = config_password

    # Check for any initial messages to parse
    if "initial_messages" in configuration_dict:
        for initial_message in configuration_dict["initial_messages"]:
            initial_command = initial_message["command"]
            initial_command_request = initial_message[initial_command]
            initial_command_request_string = json.dumps(initial_command_request)
            initial_command_request_encoded = tsunami.cJSON_CreateBytes(initial_command_request_string.encode())
            initial_message[initial_command] = initial_command_request_encoded

    # Check for any periodic messages to parse
    if "periodic_messages" in configuration_dict:
        for periodic_message in configuration_dict["periodic_messages"]:
            periodic_command = periodic_message["command"]
            periodic_command_request = periodic_message[periodic_command]
            periodic_command_request_string = json.dumps(periodic_command_request)
            periodic_command_request_encoded = tsunami.cJSON_CreateBytes(periodic_command_request_string.encode())
            periodic_message[periodic_command] = periodic_command_request_encoded


    print("Configuration:")
    tsunami.print_json(configuration_dict, 2)

    # Create the final configuration string from JSON
    configuration_buffer = json.dumps(configuration_dict)
    logger.debug('configuration_buffer: %s', configuration_buffer)
    with open(config_output_filename, 'w') as output_fd:
        output_fd.write(configuration_buffer)

    # Compress, Encrypt, and Encode the configuration_buffer
    logger.info("Compressing, encrypting, encoding config buffer...")
    encoded_configuration_buffer = tsunami.CompressEncryptEncode(configuration_buffer.encode(), config_password.encode())

    # Update the binary with the encoded_configuration_buffer
    logger.info("Updating binary with config buffer...")
    tsunami.UpdateBinary(input_filename, output_filename, encoded_configuration_buffer, tsunami.INSTALL_DEFAULT_MAGIC_NUMBER, tsunami.INSTALL_CONFIG_BUFSIZE)

    # Update the binary with the args.password
    logger.info("Updating binary with new password...")
    tsunami.UpdateBinary(output_filename, output_filename, config_password.encode(), default_password.encode(), len(default_password))

    logger.info("Saved encoded/encrypted/compressed install to %s", output_filename)

if __name__ == "__main__":
    args = parse_command_line()
    sys.exit(main(args))
