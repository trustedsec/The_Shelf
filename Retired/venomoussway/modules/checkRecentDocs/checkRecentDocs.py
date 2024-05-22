
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring

class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers

        super().__init__(modulepath, options)
