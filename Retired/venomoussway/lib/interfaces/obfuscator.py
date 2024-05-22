# instantiated when from template add_obfuscator is called used in generator call of template.  If obfusctor has not
# been added then don't obfuscator output have remove obfuscator option.  Otherwise have it's treatment be similar to
# a module, it just gets added to a special slot and not the normal list

from lib.interfaces.module_interface import ModuleInterface
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice
import os


class Obfuscator(ModuleInterface):
    def __init__(self, helpers): #static
        options = {} #static
        self.helpers = helpers #static
        self.strtype = None
        self.strargs = {}
        self.vartype = None
        self.varargs = {}
        self.inttype = None
        self.intargs = {}
        options['stringObfType'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "Specifies how strings are randomized, see confluence docs for detailed descriptions",
            "validator": self._isstrchoice,
            "validatorargs": {'choices': ['randomcuts', 'encodestring', 'encode_calls', 'donotobf']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['randomcuts', 'encodestring', 'encode_calls', 'donotobf']},
            "handler": None
        }
        options['intObfType'] = {
            "value": None,
            "required": True,
            "description": "Specifies how ints are randomized, see confluence docs for detailed descriptions",
            "validator": self._isintchoice,
            "validatorargs": {'choices': ['math', 'donotobf']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['math', 'donotobf']},
            "handler": None
        }
        options['varObfType'] = {
            "value": None,
            "required": True,
            "description": "Specifies how vars are randomized, see confluence docs for detailed descriptions",
            "validator": self._isvarchoice,
            "validatorargs": {'choices': ['businesswords', 'wordlist', 'donotobf']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['businesswords', 'wordlist', 'donotobf']},
            "handler": None
        }
        super().__init__(__file__, options, True) # options here get fed down and become self.options

    def _isvarchoice(self, line, **kwargs):
        args = self.helpers.getarguments(line)
        val = args[0]
        if ischoice(val, **kwargs):
            if val == 'businesswords':
                self.vartype = "businesswords"
            elif val == 'wordlist':
                if len(args) != 2:
                    print(" *** Error: Expecting path to a newline or csv wordlist after wordlist, ex. set varObftype wordlist /tmp/wordlist")
                    return False
                if not os.access(args[1], os.R_OK):
                    print(" *** Error: Unable to read wordlist at {}".format(args[1]))
                    return False
                else:
                    self.varargs = {"wordlistpath": args[1]}
                    self.vartype = "wordlist"
            elif val == "donotobf":
                self.vartype = "none"
            else:
                print(" *** Error: How did you get here? This shouldn't be possible!")
                return False
            return True
        else:
            return False

    def _isintchoice(self, line, **kwargs):
        args = self.helpers.getarguments(line)
        val = args[0]
        if ischoice(val, **kwargs):
            if val == 'math':
                self.inttype = "math"
            elif val == "donotobf":
                self.inttype = "none"
            else:
                print(" *** Error: How did you get here? This shouldn't be possible!")
                return False
            return True
        else:
            return False

    def _isstrchoice(self, line, **kwargs):
        args = self.helpers.getarguments(line)
        val = args[0]
        if ischoice(val, **kwargs):
            if val == 'randomcuts':
                if len(args) != 3:
                    print(" *** Error: Requires min and max length to also be specified, ex set stringObfType randomcuts 5 30")
                    return False
                else:
                    try:
                        self.strargs = {"mincut": int(args[1]), "maxcut": int(args[2])}
                    except ValueError:
                        print(" *** Error: min / max length must be ints")
                        return False
                    self.strtype = "cuts"
            elif val == "encodestring":
                self.strtype = "encode"
            elif val == "encode_calls":
                self.strtype = "encode_funcs"
            elif val == "donotobf":
                self.strtype = "none"
            else:
                print(" *** Error: How did you get here? This shouldn't be possible!")
                return False
            return True
        else:
            return False

    def get_module(self, handleDeclares=False):
        print(" *** This module is only for configuring the obfuscator, it can't generate code on its own")
        return
        
    def write_module(self, outputpath):
        print(" *** This module is only for configuring the obfuscator, it can't generate code on its own")
        return
