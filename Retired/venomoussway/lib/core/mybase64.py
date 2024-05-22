#!/bin/usr/env Python3
# -*- encoding:utf_8 -*-

import lib.core.Base64Table as Base64Table

# Encode in Base64
def encode(string):
    fullBinary = ""

    # Convert each letter of string in binary of 1 octet
    # thank to zfill
    for letter in string:
        fullBinary += bin(ord(letter))[2:].zfill(8)

    # Found on -> https://gist.github.com/bellbind/255287
    # I add .ljust(6, '0') -> to always have 6 bits
    # split fullBinary on 6bits
    # sixBinary is a generator
    sixBinary = (fullBinary[i:i + 6].ljust(6, '0')
                 for i in range(0, len(fullBinary), 6))

    stringEncode = ""
    s2 = ""

    # This is where the conversion from binary (6bits) to
    # character happens
    # s2 is to keep track of the bits numbers
    for six in sixBinary:
        s2 += six
        stringEncode += Base64Table.dictio[six]

    # Depending of the case add one or two -> '='
    if (len(s2) + 6) % 24 == 0:
        stringEncode += "="
    elif (len(s2) + 12) % 24 == 0:
        stringEncode += "=="

    return stringEncode


# Decode in Base64
def decode(string):
    fullBinary = ""

    # Convert each letter of string in binary of 6 bits
    # thank to the switchDiction if letter isn't an '='
    for letter in string:
        if letter != "=":
            fullBinary += Base64Table.switchDictio[letter]

    # split fullBinary on 8bits
    # heightBinary is a generator
    heightBinary = (fullBinary[i:i + 8] for i in range(0, len(fullBinary), 8))

    stringDecode = ""

    # This is where the conversion from binary of 1 octet to
    # character happens
    for height in heightBinary:
        stringDecode += chr(int(height, 2))

    return stringDecode


if __name__ == "__main__":

    '''print("[Base64, Thibault Galbourdin (github.com/Liodeus)]")
    print("1- Encode")
    print("2- Decode \n")

    choice = input("Choice : ")

    if choice == "1":
        message = input("Message to encode : ")
        print(encode(message))
    elif choice == "2":
        message = input("Message to decode: ")
        print(decode(message))
    else:
        print("Error")'''
    test = "asdfasdgasdgasdgasdgasdgsdgaTEST123sadgadgasdgasdgasdgTEST223"
    encoded = encode(test)
    print("Encoded: %s"%(encoded))
    decoded = decode(encoded)
    print("Decoded: %s"%(decoded))
