
from lib.interfaces.module_interface import ModuleInterface
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice

class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        options['doctype'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "The file type of your document",
            "validator": ischoice,
            "validatorargs": {'choices': ['word', 'excel']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['word', 'excel']},
            "handler": self._getdoctype
        }
        super().__init__(modulepath, options)

    def _getdoctype(self, value, **kwargs):
        if value == 'word':
            return "ThisDocument"
        else:
            return "ThisWorkbook"
