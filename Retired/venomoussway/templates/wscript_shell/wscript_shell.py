
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapequotes
from lib.validators.generic import ischoice
from lib.tab_completers.generic import tab_choice

class Template(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['runmode'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "standard or guid to specify how we create the wscript.shell object",
            "validator": ischoice,
            "validatorargs": {'choices': ['standard', 'guid']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['standard', 'guid']},
            "handler": self._getmode
        }
        options['command'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Full command with arguments of what you would like to run",
            "validator": None,
            "handler": escapequotes
        }
        super().__init__(templatePath, options)

    def _getmode(self, value, **kwargs):
        if value == 'standard':
            return "wscript.shell"
        else:
            return "new:72C24DD5-D70A-438B-8A42-98424B88AFB8"


