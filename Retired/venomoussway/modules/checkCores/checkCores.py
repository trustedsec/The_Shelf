
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import makeint
from lib.validators.generic import isint

class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        options['coreCount'] = { #list of strings for expected options
            "value": 3,
            "required": True,
            "description": "Number of cpu Cores required to pass check",
            "validator": isint,
            "handler": makeint
        }
        super().__init__(modulepath, options)
