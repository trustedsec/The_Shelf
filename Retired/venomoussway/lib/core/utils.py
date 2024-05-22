import os
import sys
import base64
import struct


testnum = 0x1456789ffff

'''def getChars(inputstring):
    #Returns string of chars for input array
    n = 2
    temp =  [inputstring[i:i+n] for i in range(0, len(inputstring), n)]
    tempchars = ""
    for item in temp:
        #print(item)
        tempchars += chr(int(item, 16))
    return tempchars
'''

def getChars(inputstring):
    '''Returns string of chars for input array'''
    n = 2
    temp =  [inputstring[i:i+n] for i in range(0, len(inputstring), n)]
    tempchars = ""
    for item in temp:
        #print(item)
        tempchars += struct.pack("B", int(item, 16))
    return tempchars

def ConvertToArray(inputint):
    hexconv = hex(inputint).replace("0x", "").replace("L", "")
    test = len(hexconv)%2
    if test != 0:
        #print("Padding with leading 0")
        hexconv = "0"+hexconv
    #print("hexconv = \"%s\""%(hexconv))
    chars = getChars(hexconv)
    #print("Number of chars %d"%(len(chars)))
    return struct.pack(">%ds"%(len(chars)), chars)

def ConvertBackToInt(inputstring):
    #print("In convertbacktoint")
    outputarray = []
    outputstring = ""
    #chars = getChars(inputstring)
    for item in inputstring:
        outputarray.append(ord(item))
        #print(ord(item))
        tempchar = hex(struct.unpack("B", item)[0]).replace("0x", "")
        if len(tempchar) == 1:
            tempchar = "0"+tempchar
        outputstring += tempchar
        #print(outputarray)
    #print(outputstring)
    #print(int(outputstring, 16))
    return int(outputstring, 16)


if __name__ == "__main__":
    print("Starting with number : %s"%(testnum))
    teststring = ConvertToArray(testnum)
    print(base64.b64encode(teststring))
    ConvertBackToInt(teststring)
