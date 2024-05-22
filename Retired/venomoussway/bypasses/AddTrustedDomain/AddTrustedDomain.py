from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapequotes
from lib.validators.generic import iswebaddress
from lib.handlers.generic import escapebackslash

class Bypass(ModuleInterface):
	def __init__(self, modulepath, helpers):
		options = {}
		self.helpers = helpers
		self.type = "bypass"
		options['TrustedUrl'] = {
            "value": None, # http://update.micros0ft.net/updates/KB810231245.msi
            "required": True,
            "description": "URL to add to trusted Zone",
            "validator": iswebaddress,
            #"validator": None,
            "handler": escapebackslash
        }
		super().__init__(modulepath, options)
