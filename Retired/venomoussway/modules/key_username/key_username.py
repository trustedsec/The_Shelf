
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice
from io import StringIO


class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        options['username'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "username required to run",
            "validator": None,
            "handler": None
        }
        options['checkmethod'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Method used to get the username",
            "validator": ischoice,
            "validatorargs": {'choices': ['application', 'win32api', 'environment']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['application', 'win32api', 'environment']},
            "handler": self._getcheckmethod
        }
        super().__init__(modulepath, options)

    def _getcheckmethod(self, value, **kwargs):
        checkmethod_code = ''
        if value == 'win32api':
            self.declares.add('Private Declare PtrSafe Sub apiGetUserName Lib "advapi32.dll" Alias "GetUserNameA" (ByVal lpBuffer As String, nSize As LongPtr)')
        checkmethod_code += 'Function CheckUsernameValue()\n'
        if value == 'application':
            checkmethod_code += '\tCheckUsernameValue = Application.UserName\n'
        elif value == 'environment':
            checkmethod_code += '\tCheckUsernameValue = VBA.Interaction.Environ("Username")\n'
        else:
            checkmethod_code += '''
            \tDim Returned_Val_Length As Variant
            \tDim Returned_UserName As String
            \tReturned_UserName = String$(254, 0)
            \tReturned_Val_Length = 255
            \tapiGetUserName Returned_UserName, Returned_Val_Length
            \tCheckUsernameValue = Mid(Trim(Returned_UserName), 1, Returned_Val_Length - 1)
'''

        checkmethod_code += 'End Function'
        return checkmethod_code

