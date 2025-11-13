#!/usr/bin/env python
import sys
import struct
xor_string = 0x2b3c


def usage():
    print('Usage: genstring.py <string to encode>')
    sys.exit(-1)


def main():
    if (len(sys.argv) != 2):
        usage()
    strToEncode = sys.argv[1]
    strsize = len(strToEncode)
    output = "wchar_t rename[] = { "
    i = 0
    for c in strToEncode:
        #if c == '\'':
            #c = '\\\''
        c = c.encode('utf-16-le')
        value = struct.unpack('H', c)[0]
        output += "{}, ".format(value ^ xor_string)
        i += 1
        if i % 16 == 0:
            output += '\n                  '
    replace = output.rfind(',')
    output = output[:replace] + ", 0};"
    output += " // bytes size {}".format(strsize *2 + 2);
    print(output)






if __name__ == "__main__":
    main()

