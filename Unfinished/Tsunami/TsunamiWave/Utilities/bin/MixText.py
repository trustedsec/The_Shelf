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
import string
import random

MODULE_NAME = 'MixText'

MIX_TEXT_START = "_InitMT_"
MIX_TEXT_END = "_FiniMT_"

DEFAULT_MIX_TEXT_PASSWORD = "{8DED3F71-05BD-48F6-B729-7457A0E137B9}"

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

    parser.add_argument("--key", dest="key", help="MixText xor key")
    parser.add_argument("--input", dest="input", help="Input binary filename")
    parser.add_argument("--output", dest="output", help="Output binary filename")
    parser.add_argument("--debug", dest="debug", action='store_true',  help="Output binary filename")
    parser.add_argument("--random", dest="randomkey", action='store_true',  help="Generate a random MixText key")
    parser.set_defaults(debug=False,randomkey=False)
    args = parser.parse_args()

    if not os.path.isfile( args.input ):
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)

    if args.randomkey:
        args.key = ''.join(random.choice(string.printable) for _ in xrange(len(DEFAULT_MIX_TEXT_PASSWORD)))
    elif not args.key:
        args.key = DEFAULT_MIX_TEXT_PASSWORD

    if len(args.key) > len(DEFAULT_MIX_TEXT_PASSWORD):
        raise IndexError( 'key too long', args.key)


    return args


def encrypt_string(input_string,encryption_key):
    output_string = ''
    random_rolling_xor_seed = random.randint(1,255)
    output_char = chr(random_rolling_xor_seed)
    mix_text_buffer = ''
    mix_text_size = 0
    i = 0
    #print "input_string", input_string
    #print "encryption_key", encryption_key
    for input_char in input_string:
        #print "i: %d    in:%s key:%s rolling:%s" % ( i, binascii.hexlify(chr(ord(input_char))), binascii.hexlify(chr(ord(encryption_key[i]))), binascii.hexlify(chr(ord(output_char))))
        output_char = chr(ord(input_char) ^ ord(encryption_key[i]) ^ ord(output_char))
        #print "output_char", binascii.hexlify(chr(ord(output_char)))
        i = (i + 1) % len(encryption_key)
        output_string = output_string + output_char
        mix_text_size = mix_text_size + 1

    mix_text_random_seed = struct.pack('B', random_rolling_xor_seed)
    mix_text_buffer_size = struct.pack('<I', mix_text_size)

    mix_text_buffer = mix_text_buffer + mix_text_buffer_size
    mix_text_buffer = mix_text_buffer + mix_text_random_seed
    mix_text_buffer = mix_text_buffer + output_string

    return mix_text_buffer



def main(args):
    """Pack the configuration information/bytes/struct into the binary"""

    logger = set_logger(args)

    logger.debug('Start of main')
    logger.debug('args.key       %s', args.key)
    logger.debug('args.input     %s', args.input)
    logger.debug('args.output    %s', args.output)






    # Read in the unmixed binary
    logger.info('Reading unmixed binary from %s', args.input)
    input_bytes = ''
    with open(args.input, 'rb') as inputfd:
        input_bytes = inputfd.read()
    logger.debug('Length of input binary:  %d', len(input_bytes))

    output_bytes = input_bytes

    # Find the mixed strings in the input binary
    mix_text_start_index = input_bytes.find(MIX_TEXT_START)
    while -1 != mix_text_start_index:
        #logger.debug('mix_text_start_index: %d', mix_text_start_index)
        mix_text_end_index = input_bytes.find(MIX_TEXT_END, mix_text_start_index)
        if -1 == mix_text_end_index:
            logger.error('mix_text_end_index not found to match mix_text_start_index at %d', mix_text_start_index)
            raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.input)

        #logger.debug('mix_text_end_index: %d', mix_text_end_index)

        # Replace the current mix text string
        original_string = input_bytes[mix_text_start_index+len(MIX_TEXT_START):mix_text_end_index]
        logger.debug('original_string: %s', original_string)

        mixed_string = encrypt_string(original_string,args.key)
        logger.debug('mixed_string: %s', binascii.hexlify(mixed_string))

        output_bytes = ''
        output_bytes = output_bytes + input_bytes[:mix_text_start_index]
        output_bytes = output_bytes + mixed_string
        num_padding = (mix_text_end_index + len(MIX_TEXT_END) - mix_text_start_index - len(mixed_string))
        #logger.debug('num_padding: %d', num_padding)
        output_bytes = output_bytes + ''.join('\0' for _ in xrange( num_padding ) )
        output_bytes = output_bytes + input_bytes[mix_text_end_index + len(MIX_TEXT_END):]

        logger.debug('Length of output binary: %d', len(output_bytes))

        input_bytes = output_bytes

        mix_text_start_index = input_bytes.find(MIX_TEXT_START)


    if args.key != DEFAULT_MIX_TEXT_PASSWORD:
        logger.debug('Replacing the default key with the new key')
        mix_text_key_index = output_bytes.find(DEFAULT_MIX_TEXT_PASSWORD)
        if -1 == mix_text_key_index:
            logger.error('mix_text_key_index not found')
            raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.output)
        logger.debug('mix_text_key_index: %d', mix_text_key_index)
        new_output_bytes = ''
        new_output_bytes = new_output_bytes + output_bytes[:mix_text_key_index]
        new_output_bytes = new_output_bytes + args.key
        num_padding = ( len(DEFAULT_MIX_TEXT_PASSWORD) - len(args.key) )
        new_output_bytes = new_output_bytes + ''.join('\0' for _ in xrange( num_padding ) )
        new_output_bytes = new_output_bytes + output_bytes[mix_text_key_index+len(DEFAULT_MIX_TEXT_PASSWORD):]
        output_bytes = new_output_bytes

    # Write the mixed binary to file
    with open(args.output, 'wb') as outputfd:
        outputfd.write(output_bytes)
    logger.info('Saved %d bytes to the mixed binary: %s', len(output_bytes), args.output)



if __name__ == "__main__":
    args = parse_command_line()
    sys.exit(main(args))