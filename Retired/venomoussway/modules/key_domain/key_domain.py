from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice


class Module(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        options['domain'] = { #list of strings for expected options
            "value": None,
            "required": True,
            "description": "domain name you want to match (submatches work)",
            "validator": None,
            "handler": None
        }
        options['checkmethod'] = { #list of strings for expected options
            "value": "dnsdomain",
            "required": True,
            "description": "api method to get domainname",
            "validator": ischoice,
            "validatorargs": {'choices': ['userdomain_wscript', 'userdomain', 'dnsdomain']},
            "tab_complete": tab_choice,
            "tab_args": {'choices': ['userdomain_wscript', 'userdomain', 'dnsdomain']},
            "handler": self._getcheckmethod
        }
        super().__init__(modulepath, options)

    def _getcheckmethod(self, value, **kwargs):
        checkmethod_code = ''

        if value == 'userdomain_wscript':
            checkmethod_code += '\tset net = CreateObject("Wscript.Network")\n'
            checkmethod_code += '\tCheckDomainNameValue = LCase(net.UserDomain)\n'
        elif value == 'userdomain':
            checkmethod_code += '\tCheckDomainNameValue = LCase(VBA.Interaction.Environ("UserDomain"))\n'
        elif value == 'dnsdomain':
            checkmethod_code += '\tCheckDomainNameValue = LCase(VBA.Interaction.Environ("UserDNSDomain"))\n'
        else:
            checkmethod_code += '\tCheckDomainNameValue = ""\n'

        return checkmethod_code


