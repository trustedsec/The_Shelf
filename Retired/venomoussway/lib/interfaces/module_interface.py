import os
import jinja2
import pprint
import traceback


class ModuleInterface:
    def __init__(self, templatePath=None, options=None, configOnly=False):
        if options is None:
            options = {}
        self.options = options
        self.warnings = "" #warnings get stored here so we can report them easier when using web later
        self.errors = ""
        self.lastError = ""
        self.templatePath = templatePath
        self.configOnly = configOnly
        self.template = ""
        if not hasattr(self, "type"):
            self.type = self.__class__.__name__
        self.name = os.path.splitext(os.path.basename(self.templatePath))[0]
        if not configOnly: # primarily used by obfuscator config module
            if templatePath is None or os.access(templatePath, os.R_OK) == False:
                raise Exception(" [!] Error: Cannot access the template for module {}".format(self.__class__.__name__))
            else:
                with open(templatePath, 'r') as fp:
                    self.template = fp.read()
        self.declares = set()  # This should be a list of win32 api declare / TYPE statements if needed

    def set_option(self, optionName, optionValue=None):
        if optionName in self.options:
            if optionValue is None:
                self.options[optionName]['value'] = optionValue
                return 0
            else:
                if self._validate_option(optionName, optionValue):
                    self.options[optionName]['value'] = optionValue
                    return 0
                else:
                    return 1
        else:
            return 2

    def get_option(self, optionName):
        if 'handler' in self.options[optionName] and self.options[optionName]['handler'] is not None:
            args = self.options[optionName]['handlerargs'] if 'handlerargs' in self.options[optionName] else {}
            return self.options[optionName]['handler'](self.options[optionName]['value'], **args)
        return self.options[optionName]['value']

    def _validate_option(self, optionName, optionValue):
        if 'validator' in self.options[optionName] and self.options[optionName]['validator'] is not None:
            args = self.options[optionName]['validatorargs'] if 'validatorargs' in self.options[optionName] else {}
            status = self.options[optionName]['validator'](optionValue, **args)
            return status
        else:
            self.lastError = None
            return True

    def _add_warning(self, msg):
        print(self.warnings)
        print(msg)
        self.warnings += msg + "\n"

    def _add_error(self, msg):
        self.errors += msg + "\n"

    def check_required(self):
        status = True
        for k, v in self.options.items():
            if v['required'] is True and v['value'] is None:
                print("  [!] Error: Option '{}' in {} is required but is NOT set".format(k,self.name))
                status = False
        return status
        
    #override if you need to take actions before validation occurs
    def preprocess(self):
        return
        
    #override if you need to take actions before validation occurs
    def process(self, context={}):
        if self.configOnly:
            print("  [!] Error: Process failed. This module is for configuration ONLY.")
            return ""
        self.warnings = ""
        input_template = self.template
        render_results = ""
        for k in self.options.keys():
            try:
                transformed = self.get_option(k)
                if 'noinsert' not in self.options[k]:
                    context[k] = transformed
                self.options[k]['processed'] = transformed
            except Exception as msg:
                traceback.print_exc()
                print("  [!] Error: Failed to process key {} value {}".format(k, self.get_option(k)))
                return ""
        env = jinja2.Environment(autoescape=False)
        doc = env.from_string(input_template)
        render_results = doc.render(context)
        return render_results
        
    #override if you need to take actions after the template has been generated
    def postprocess(self):
        return

    def cleanup(self):
        self.declares = set() # these need to be reset after a run so we don't mix between runs
        return
        
    def postObfuscator(self, data): # hook to do anything needed after the obfuscator runs
        return data
