
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapebackslash
from lib.validators.generic import iswebaddress


class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['HomepageURL'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Location where you are hosting your homepage payload",
            "validator": iswebaddress,
            "handler": escapebackslash,
        }
        super().__init__(templatePath, options)
