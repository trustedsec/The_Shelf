import os
import sys
import random
import string
import struct
import hashlib
import zlib
import time
import datetime
import binascii
from Crypto.Cipher import AES
from Crypto import Random
import lib.core.mycrypto as mycrypto
import lib.core.utils as utils

class BinaryClass:

    def __init__(self, helpers, options=""):
        self.options = options
        self.helpers = helpers
        self.verbose = 0
        self.magicbytes = {"gif":b"\x47\x49\x46\x38\x37\x61", "jpeg": b"\xff\xd8\xff\xdb", 
            "jpeg2":b"\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01", 
            "jpg":b"\xff\xd8\xff\xe0\x00\x10\x4a\x46\x49\x46\x00\x01",
            "pdf": b"\x25\x50\x44\x46\x2d", "der": b"\x30\x82", "mp3": b"\xff\xfb", 
            "mp32": b"\x49\x44\x33", "bmp": b"\x42\x4d", "ico": b"\x69\x63\x6e\x73"}

    '''Generate string of random length, or specified size.'''
    def string_generator(self, size=0, chars=string.ascii_lowercase):
        if size == 0:
            size = random.randint(4, 10)
        return ''.join(random.choice(chars) for _ in range(size))

    def get_template_contents(self, templatefile):
        '''Simple function to simplify modules when using templates'''
        content = None
        try:
            fin = open(templatefile, "rb")
            content = fin.read()
            fin.close()
        except:
            print("Error: Cannot open specified file")
        return content
    
    def getTimeStamp(self, builddate="1/1/2012"):
        currenttime = builddate.split("/")
        month = int(currenttime[0])
        day = int(currenttime[1])
        year = int(currenttime[2])
        timestruct = datetime.datetime(year, month, day)
        outstruct = struct.pack("<I", int(time.mktime(timestruct.timetuple())))
        return outstruct

    def patchBinary(self, infiledata, builddate="1/1/2012"):
        timestamp = self.getTimeStamp(builddate)
        return infiledata[:0x88]+timestamp+infiledata[0x88+0x4:]

    def get_random_string(self, stringlength, filetype=None):
        randombytes = ""
        if filetype != None:
            if filetype not in self.magicbytes.keys():
                print("ERROR: Failed to get magic bytes for type: %s"%(filetype))
                return ""
            randombytes = self.magicbytes[filetype]

        testlength = len(randombytes)
        while (testlength < stringlength):
            randombytes += chr(random.randint(0, 255))
            testlength+=1
        return randombytes
    
    def get_random_bytes(self, stringlength, filetype=None):
        randombytes = b""
        if filetype != None:
            if filetype not in self.magicbytes.keys():
                print("ERROR: Failed to get magic bytes for type: %s"%(filetype))
                return ""
            randombytes = self.magicbytes[filetype]
        testlength = len(randombytes)
        while testlength < stringlength:
            randombytes += bytes([random.randint(0,255)])
            testlength+=1
        return randombytes
        
    def getAESKey(self):
        lower = 20
        higher = 45
        charset = string.ascii_lowercase+string.ascii_uppercase+string.digits+' '
        return self.string_generator(random.randint(lower, higher), charset).encode('utf-8')

    '''Added because it was used in all modules so far, used version
        that works with longer strings as well as the original.'''
    def patchValues(self, replacestring, infileData, value, isurl = 0):
        outdata = infileData
        if self.verbose == 1:
            print("URL Length : %d"%(len(replacestring)*2+1))
        replacementChr = ""
        if isurl == 0:
            replacementChr = struct.pack("B", (len(replacestring)*2)+1)
        else:
            replacementChr = struct.pack(">H", 0x8000 +(len(replacestring)*2)+1)
        temptest = replacementChr + self.convertAsciiToWideChar(replacestring)
        if temptest in infileData:
            if self.verbose == 1:
                print("Replacing string %s"%(replacestring))
            templength = len(temptest)-1
            replacementval = self.convertAsciiToWideChar(value)
            origlen = len(replacementval)
            #Change this so that its using struct.pack to a char array
            if isurl == 0:
                replacementval = struct.pack("B", origlen+1)+replacementval
            else:
                templength = len(temptest)-2
                replacementval = struct.pack(">H", 0x8000 + origlen+1)+replacementval
            if self.verbose == 1:
                print(origlen+1)
            paddedvalue = replacementval + (b"\x00"*(templength-origlen))
            outdata = infileData.replace(temptest, paddedvalue)
        return outdata

    def patchRawValues(self, replacestring, infiledata, value, xor=0, isstring=0):
        """Takes replacementstring, data, value, xorValue, and 0 or 1 if its 
            a string."""
        replacestringValue = replacestring
        valueValue = value
        if len(value) > replacestring:
            print("Error value your replacing is too big")
        valueValue = value+("\x00"*(len(replacestringValue)-len(valueValue)))
        if xor != 0:
            #XOR encode it, then use that to patch.
            replacestringValue = self.xor_data(replacestring, xor, isstring=isstring)
            valueValue = self.xor_data(valueValue, xor, isstring=isstring)
        if replacestringValue not in infiledata:
            if xor != 0:
                print("Encoded string for '%s' not in data"%(replacestring))
            else:    
                print("String '%s' not in data"%(replacestring))
        
        outdata = infiledata.replace(replacestringValue, valueValue)
        return outdata
    
    def aes_pad(self, data):
        if type(data) == str:
            paddingchar = 16 - len(data)%16
            return data + ("%d"%(paddingchar)*paddingchar)
        else:
            paddingchar = 16 - len(data)%16
            return data + bytes([paddingchar])*paddingchar

    def aes_encrypt(self, key, data):
        """aes encrypt function returns encrypted data with IV."""
        iv = Random.new().read(16)
        cipher = AES.new(key, AES.MODE_CBC, iv)
        output = cipher.encrypt(data)
        return iv + output

    def aes_encrypt_sha256(self, key, data):
        """Uses the sha256 hash of key to derive the key."""
        realkey = key
        if type(key) != bytes:
            realkey = key.encode('utf-8')
        keyhash = self.helpers.binary.generate_sha256(key)
        #print("KeyHash: %s"%(binascii.hexlify(keyhash)))
        return self.aes_encrypt(keyhash, data)

    def aes_decrypt(self, key, data):
        """aes decrypt function returns decrypted data IV stripped."""
        iv = data[0:16]
        realdata = data[16:]
        cipher = AES.new(key, AES.MODE_CBC, iv)
        output = cipher.decrypt(realdata)
        return output

    def aes_decrypt_sha256(self, key, data):
        """Uses the sha256 hash of key to derive the key."""
        realkey = key
        if type(key) != bytes:
            realkey = key.encode('utf-8')
        keyhash = self.helpers.binary.generate_sha256(realkey)
        return self.aes_decrypt(keyhash, data)

    def rc4_crypt(self, key, data):
        """rc4 encryption/decryption function
        TODO: Update to use the Crypto.Cipher ARC4 implementation.
        """
        return mycrypto.rc4(key, data)

    def generate_sha256(self, data):
        """Converts data input to char array"""
        #print("TEST: %s"%(data))
        #secretBytes = utils.getChars(data)
        realdata = data
        if type(data) == str:
            realdata = data.encode('utf-8')
        s = hashlib.sha256()
        s.update(realdata)
        return s.digest()
    
    def generate_md5(self, data):
        """Converts data input to char array"""
        #print("TEST: %s"%(data))
        #secretBytes = utils.getChars(data)
        realdata = data
        if type(data) == str:
            realdata = data.encode('utf-8')
        s = hashlib.md5()
        s.update(realdata)
        return s.digest()

    def cleanString(strin):
        outstring = strin.replace("\\n", "\n")
        outstring = outstring.replace("\\t", "\t")
        outstring = outstring.replace("\\\"", "\"")
        return outstring


    def xor_data(self, data, xorval, isstring=0):
        '''Function to obfuscate strings in the source code
            any code changes in this function needs to be change inside of the 
            C code to decode the strings. '''
        strin = data
        #Used for obfuscating my strings for full agent
        if isstring == 1:
            #Cleanup the string, replace \n and \t with each other.
            strin = self.cleanString(data)

        #start parsing out the string and obfuscating it.
        if type(data) == str:
            strings = []
            for linechar in strin:
                strings.append(chr(ord(linechar)^xorval))
            return "".join(strings)
        else:
            outdata = b''
            for item in data:
                outdata += bytes([item^xorval])
            return outdata

    def compress(data, packageWithSize=0):
        '''Compress data with zlib, and if packageWithSize isn't 0, prepend
        the length of the compressed data to the return value.
        '''
        outdata = zlib.compress(data)
        if packageWithSize != 0:
            outdata = struct.pack("<L%ds"%(len(outdata)), len(outdata), outdata)
        return outdata

    '''Used somewhat often in modules.'''
    def convertAsciiToWideChar(self, value):
        retval = b""
        for item in value:
            retval += bytes([item])+b"\x00"
        return retval

