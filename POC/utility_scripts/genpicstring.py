#!/usr/bin/env python
import sys


def usage():
    print('Usage: genstring.py <string to encode>')
    sys.exit(-1)


def main():
    if (len(sys.argv) != 2):
        usage()
    strToEncode = sys.argv[1]
    strsize = len(strToEncode)
    output = "char rename[] = { "
    i = 0
    for c in strToEncode:
        if c == '\'':
            c = '\\\''
        output += "'{}', ".format(c)
        i += 1
        if i % 16 == 0:
            output += '\n                  '
    replace = output.rfind(',')
    output = output[:replace] + ", 0 };"
    output += " //size {}".format(strsize+1) #+1 for terminating null
    print(output)






if __name__ == "__main__":
    main()

