
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import makelist

class Module(ModuleInterface): #usename
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['badbios'] = { #list of strings for expected options
            "value": 'virtualbox "vmware" kvm',
            "required": True,
            "description": "space separated list of lowercase bios names to check for",
            "validator": None,
            "handler": makelist
        }
        super().__init__(templatePath, options)
