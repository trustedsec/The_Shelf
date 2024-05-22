
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring, makebool
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice


class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        options['includeDefault'] = { #list of strings for expected options
            "value": 'false',
            "required": True,
            "description": "Include default Windows EDRs?",
            "validator": ischoice,
            "validatorargs": {'choices': ['false', 'true']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['false', 'true']},
            "handler": makebool
        }
        super().__init__(modulepath, options)

