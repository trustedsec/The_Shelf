
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapequotes
from lib.validators.generic import isboolstring, ischoice
from lib.tab_completers.generic import tab_choice

class Template(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['command'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "command to execute",
            "validator": None,
            "handler": escapequotes
        }
        options['UAC_Prompt'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "set to True to elevate with a user show UAC prompt, False to not prompt / elevate",
            "validator": isboolstring,
            "handler": self._uac
        }
        options['runmode'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "standard or guid to specify how we create the Shell.Application object",
            "validator": ischoice,
            "validatorargs": {'choices': ['standard', 'guid']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['standard', 'guid']},
            "handler": self._runmode
        }
        super().__init__(templatePath, options)

    def _uac(self, value, **kwargs):
        if value.lower() == 'true':
            return "runas"
        else:
            return ""

    def _runmode(self, value, **kwargs):
        if value == 'standard':
            return "Shell.Application"
        else:
            return "new:13709620-C279-11CE-A49E-444553540000"
