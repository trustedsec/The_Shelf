import os
import sys
import base64
import struct



class MyBase64:
    def __init__(self):
        self.mybase64_originalTable = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"
        self.mybase64_myTable = b"\x00\x48\xff\x8b\x01\x24\x0f\x03\x8d\x02\x85\x89\x83\xe8\x4c\xc0\x74\x44\xcc\x41\x15\x20\x66\x45\x33\x05\x84\x40\x75\xfe\x49\xe9\x30\x04\x08\x65\x70\x10\xc8\xc3\x0d\x39\x6c\x4d\xb7\x38\x50\xc7\x5c\x6f\xc4\x72\x73\xeb\x80\xd2\x64\xc9\x28\x6e\x54\xf8\x88\x78"
        self.mybase64_mypadding = b"\xde"
        self.mybase64_originalPadding = b"="
        self.mybase64_inmapping = {}
        self.mybase64_outmapping = {}
        self.init_mappings()
        
    def init_mappings(self):
        for i in range(0, 64):
            self.mybase64_outmapping[self.mybase64_originalTable[i]] = self.mybase64_myTable[i]
            self.mybase64_inmapping[self.mybase64_myTable[i]] = self.mybase64_originalTable[i]
        self.mybase64_outmapping[self.mybase64_originalPadding[0]] = self.mybase64_mypadding[0]
        self.mybase64_inmapping[self.mybase64_mypadding[0]] = self.mybase64_originalPadding[0]

    def b64encode(self, data):
        output = base64.b64encode(data)
        convertedoutput = b""
        for item in output:
            if type(self.mybase64_outmapping[item]) == int:
                convertedoutput += struct.pack("B", self.mybase64_outmapping[item])
            else:
                convertedoutput += self.mybase64_outmapping[item]
        return convertedoutput

    def b64decode(self, data):
        convertedinput = b""
        for item in data:
            if type(self.mybase64_inmapping[item]) == int:
                convertedinput += struct.pack("B", self.mybase64_inmapping[item])
            else:
                convertedinput += self.mybase64_inmapping[item]
        output = base64.b64decode(convertedinput)
        return output

if __name__ == "__main__":
    #fin = open(sys.argv[1], "rb")
    #readin = fin.read()
    #fin.close()
    readin = b"this is a test string, this should be readable."
    m64 = MyBase64()
    encoded = m64.b64encode(readin)
    print(encoded)
    print("Got encoded data")
    decoded = m64.b64decode(encoded)
    print(decoded)
    fout = open("testbin.bin", "wb")
    fout.write(encoded)
    fout.close()

