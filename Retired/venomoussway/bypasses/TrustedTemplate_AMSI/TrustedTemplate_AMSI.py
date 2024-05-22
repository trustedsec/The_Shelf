
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring

class Bypass(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        self.type = "bypass"
        options['vba_payload'] = { 
            "value": None,
            "required": True,
            "description": "VBA function to call once running from the trusted location. This is typically the name of the template you are using, e.g., vba_exec.",
            "validator": None,
            "handler": None
        }
        super().__init__(modulepath, options)
