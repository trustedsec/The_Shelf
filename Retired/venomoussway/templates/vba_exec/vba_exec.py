
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapequotes

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
        super().__init__(templatePath, options)
