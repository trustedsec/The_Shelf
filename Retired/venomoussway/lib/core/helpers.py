import os
import fnmatch
import sys
import shlex
import subprocess
import base64
import glob
from importlib import import_module
from lib.core.binary import BinaryClass
from lib.core.macro import MacroClass
import lib.core.mybase64 as mybase64
from lib.core.base64_mapping import MyBase64
from lib.obfuscator.randomizers.vbarandomizer import vbaRandomizer
from lib.core.stagers import stagers

class Helpers:
    def __init__(self):
        self.modules = {}
        self.bypasses = {}
        self.templates = {}
        self.basepath = os.path.dirname(os.path.dirname(os.path.dirname(__file__)))
        self.binary = BinaryClass(self)
        self.macro = MacroClass()
        self.stager = stagers(self)
        self.rccommands = []
        # self.logfile = "./data/payload.log"
        # self.fout = open(self.logfile, "a+")
        self.FAIL = '\033[91m'
        self.BOLD = '\033[1m'
        self.GREEN = '\033[32m'
        self.ENDC = '\033[0m'
        self.formatValues = {"ERROR": self.FAIL, "INFO": self.BOLD, "SUCCESS": self.GREEN}
        self.formatValuesPrepend = {"ERROR": "[!]", "INFO": "[-]", "SUCCESS": "[+]"}


    def getpayloaddir(self):
        return os.path.join(self.basepath, "data/payloads")


    @staticmethod
    def complete_path(path, line, **kwargs):
        if os.path.isdir(path):
            return glob.glob(os.path.join(path, '*'))
        else:
            return glob.glob(path + '*')


    @staticmethod
    def getarguments(cmd):
        cmdparse = shlex.shlex(cmd, posix=True)
        cmdparse.escape = ''
        cmdparse.whitespace_split = True
        return list(iter(cmdparse))


    @staticmethod
    def parseURI(uri):
        ssl = True if uri[:5].lower() == 'https' else False
        if uri.count(':') != 1:
            port = int(uri[uri.rfind(':')+1:uri.find('/', 9)])
            host = uri[uri.find('//')+2:uri.rfind(':')]
        else:
            port = 80 if not ssl else 443
            host = uri[uri.find('//') + 2:uri.find('/', 9)]
        path = uri[uri.find('/', 9):]
        return ssl, port, path, host


    def load_templates(self, rootPath, moduleType):
        """
        Parse out a folder path, and load the modules.
        :param rootPath:
        :param moduleType:
        :return:
        """

        for root, dirs, files in os.walk(rootPath):
            for filename in fnmatch.filter(files, '*.py'):
                filePath = os.path.join(root, filename)

                #  don't load up the template
                if filename == "template.py" or \
                   filename == "__init__.py" or \
                   filename == "jobs-template.py":
                    continue
                basepath = os.path.dirname(filePath)
                basepath = os.path.abspath(basepath)

                if basepath not in sys.path:
                    sys.path.append(basepath)
                #  extract just the module name from the full path
                moduleName = filename[:-3]
                vbafile = os.path.abspath(filePath[:-3] + ".vba")
                if not os.access(vbafile, os.R_OK):
                    vbafile = os.path.abspath(filePath[:-3] + ".bas")
                    if not os.access(vbafile, os.R_OK):
                        raise Exception("Can't find vba code associated with {}".format(filePath))

                #  instantiate the module and save it to the internal cache
                try:
                    if moduleType == 'module':
                        self.modules[moduleName] = (import_module(moduleName), vbafile)
                    elif moduleType == 'bypass':
                        self.bypasses[moduleName] = (import_module(moduleName), vbafile)
                    elif moduleType == 'template':
                        self.templates[moduleName] = (import_module(moduleName), vbafile)
                except Exception as e:
                    print("*** Error: Failed to import module!")
                    print("    Module Name:  {}".format(moduleName))
                    print("    VBA File:     {}".format(vbafile))
                    print("    Import Error: {}".format(str(e)))


    def format_print(self, linetype, data):
        """
        Types:
            ERROR == FAIL
            INFO == BOLD
            SUCCESS == GREEN
        """
        if linetype in self.formatValues:
            print(self.formatValues[linetype]+self.formatValuesPrepend[linetype]+data+self.ENDC)
        else:
            print(data)


    def generate_header(self, data, xorval=0):
        """Convert binary data into a header for C
            data at 'payload_bin' and value at 'payload_bin_len' """
        bindata = data
        headerdata = "unsigned char payload_bin[] = {"
        if xorval != 0:
            bindata = self.binary.xor_data(data, xorval)

        for count, item in enumerate(bindata):
            if count % 12 == 0:
                headerdata += "\n  "
            headerdata += "0x%02x" % (item)
            if count != len(bindata) - 1:
                headerdata += ", "

        headerdata += "\n};\n"
        headerdata += "unsigned int payload_bin_len = %s;" % (len(bindata))
        return headerdata


    def msbuild_solution(self, solution, arguments=""):
        devnull = open(os.devnull, "w")
        testout = os.popen("which msbuild")
        testval = testout.read()
        testout.close()
        if testval != "":
            subprocess.call(['msbuild', solution, arguments], stdout=devnull)
        else:
            subprocess.call(['xbuild', solution, arguments], stdout=devnull)
        devnull.close()


    def generate_csharp_bytes(self, data, xorval=0):
        headerdata = "byte[] buf = new byte[%d] {\n" % (len(data))
        bindata = data
        if xorval != 0:
            bindata = self.binary.xor_data(data, xorval)

        for count, item in enumerate(bindata):
            if count % 12 == 0:
                headerdata += "\n  "
            headerdata += "0x%02x" % (item)
            if count != len(bindata) - 1:
                headerdata += ", "

        headerdata += "};\n"
        return headerdata


    def generate_base64(self, data, xorval=0):
        outdata = ""
        bindata = data
        if xorval != 0:
            bindata = self.binary.xor_data(data, xorval)
        outdata = base64.b64encode(bindata)
        return outdata


    def decode_base64(self, data):
        return base64.b64decode(data)


    def generate_inhouse_base64(self, data, xorval=0):
        outdata = ""
        bindata = data
        if xorval != 0:
            bindata = self.binary.xor_data(data, xorval)
        outdata = mybase64.encode(bindata)
        return outdata


    def decode_inhouse_base64(self, data):
        outdata = mybase64.decode(data)
        return outdata


    def generate_bin_base64(self, data):
        mb64 = MyBase64()
        return mb64.b64encode(data)


    def decode_bin_base64(self, data):
        mb64 = MyBase64()
        return mb64.b64decode(data)


    def encode_dll(self, dllpath, type, key):
        if type not in list(self.binary.magicbytes.keys()):
            print("[!] Warning: Invalid type {}. Reverting to file type gif")
            type = "gif"
        padding = self.binary.get_random_bytes(32, filetype=type)
        stage = self.binary.get_template_contents(dllpath)
        stage = self.binary.aes_pad(stage)
        stagebin = self.binary.aes_encrypt_sha256(key, stage)
        return padding + stagebin

    @staticmethod
    def obfuscatevba(data, config, wordmappings=None):
        if config == None:
            return data
        randomizer = vbaRandomizer(data)
        randomizer.set_numeric_randomizer(config.inttype, **config.intargs)
        randomizer.set_string_randomizer(config.strtype, **config.strargs)
        randomizer.set_variable_randomizer(config.vartype, **config.varargs)
        obfuscated = randomizer.randomize()
        if wordmappings is not None:
            wordmappings.update(randomizer.wordmappings)
        return obfuscated