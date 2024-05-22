import cmd
import json
import os
import platform
import pprint
import traceback

if platform.system() == 'Darwin':
    import readline
    import rlcompleter

    if 'libedit' in readline.__doc__:
        readline.parse_and_bind("bind ^I rl_complete")
    else:
        readline.parse_and_bind("tab: complete")
    readline.set_completer_delims(' \t\n')
elif platform.system() == 'Windows':
    import pyreadline as readline
else:
    import readline
    readline.set_completer_delims(' \t\n')
    
from lib.core.helpers import Helpers
from lib.interfaces.module_interface import ModuleInterface
from lib.interfaces.obfuscator import Obfuscator



class MainMenu(cmd.Cmd):
    def __init__(self, helpers: Helpers, debug=False):
        self.availableModules = [module for module in helpers.modules.keys()]
        self.availableBypasses = [bypass for bypass in helpers.bypasses.keys()]
        self.availableTemplates = [template for template in helpers.templates.keys()]
        self.modules = {}
        self.bypasses = {}
        self.templates = {}
        self.obfuscator = None
        self.helpers = helpers
        self.DEBUG = debug
        super().__init__()
        while len(self.helpers.rccommands) > 0:
            self.onecmd(self.helpers.rccommands.pop(0))


    def do_list(self, cmd):
        """
        list [bypasses|modules|templates|obfuscators]
        
        List the available bypasses, modules, templates, obfuscators, or all if
        not specified.
        """
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 1 and len(arguments) != 0:
                print("\n  [!] Error: Invalid usage of list")
                print("      Expecting: list [tempalate|module|bypass|obfuscator]\n")
                return
            list_type = ""
            if len(arguments)  > 0:
                list_type = arguments[0]
            if list_type != "modules" and list_type != "bypasses" and list_type != "templates" and list_type != "obfuscators" and \
               list_type != "module" and list_type != "bypass" and list_type != "template" and list_type != "obfuscator"  and \
               list_type != "":
                print("\n  [!] Error: Invalid listing")
                print("      Expecting: list (tempalates|modules|bypasses|obfuscator)\n")
                return
            if list_type == "modules" or list_type == "module" or list_type == "":
                print("\n  available modules:")
                for item in self.availableModules:
                    print("    {}".format(item))
            if list_type == "bypasses" or list_type == "bypass" or list_type == "":
                print("\n  available bypasses:")
                for item in self.availableBypasses:
                    print("    {}".format(item))
            if list_type == "templates" or list_type == "template" or list_type == "":
                print("\n  available templates:")
                for item in self.availableTemplates:
                    print("    {}".format(item))
            if list_type == "obfuscators" or list_type == "obfuscator" or list_type == "":
                print("\n  available obfuscator:")
                print("    obfuscator")
            print("")
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process list command: {}\n".format(msg))

    def complete_list(self, text, line, start_index, end_index):
        arguments = self.helpers.getarguments(line)
        results = []
        types = ['modules', 'bypasses', 'templates', 'obfuscators']
        list_type = ""
        if len(arguments) > 1:
            list_type = arguments[1]
        if list_type not in types:
            for key in types:
                if key.startswith(text):
                     results.append(key)
        return results


    def do_add(self, cmd):
        """
        add <bypass|module|template|obfuscator> [name]
        
        Add a bypass, module, or template with the specified name, or an 
        obfuscator (which does not take a name argument. 
        
        You can add many modules and bypasses, but only one template can be 
        added. Subsequent template adds will replace the current template. 
        
        There is also only one available obfuscator which can be added or not.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 2 and len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of add")
                print("      Expecting: add <tempalate|module|bypass|obfuscator> [name]\n")
                return
            add_type = arguments[0]
            instance = None
            if add_type == 'module':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of add {}".format(add_type))
                    print("      Expecting: add {} <{}_name>\n".format(add_type,add_type))
                    return
                add_name = arguments[1]
                if add_name not in self.availableModules:
                    print("\n  [!] Error: {} '{}' not available.".format(add_type, add_name))
                    print("      The following {} are available:".format(add_type))
                    print("        "+"\n        ".join(self.availableModules)+"\n")
                    return
                if add_name in self.modules:
                    print("\n  [+] Warning: {} '{}' already added.\n".format(add_type, add_name))
                    return
                module, vba = self.helpers.modules[add_name]
                instance = module.Module(vba, self.helpers)
                self.modules[add_name] = instance
                print("\n  Added {} '{}'\n".format(add_type, add_name))
            elif add_type == 'bypass':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of add {}".format(add_type))
                    print("      Expecting: add {} <{}_name>\n".format(add_type,add_type))
                    return
                add_name = arguments[1]
                if add_name not in self.availableBypasses:
                    print("\n  [!] Error: {} '{}' not available.".format(add_type, add_name))
                    print("      The following {} are available:".format(add_type))
                    print("        "+"\n        ".join(self.availableBypasses)+"\n")
                    return
                if add_name in self.bypasses:
                    print("\n  [+] Warning: {} '{}' already added.\n".format(add_type, add_name))
                    return
                bypass, vba = self.helpers.bypasses[add_name]
                instance = bypass.Bypass(vba, self.helpers)
                self.bypasses[add_name] = instance
                print("\n  Added {} '{}'\n".format(add_type, add_name))
            elif add_type == 'template':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of add {}".format(add_type))
                    print("      Expecting: add {} <{}_name>\n".format(add_type,add_type))
                    return
                add_name = arguments[1]
                if add_name not in self.availableTemplates:
                    print("\n  [!] Error: {} '{}' not available.".format(add_type, add_name))
                    print("      The following {} are available:".format(add_type))
                    print("        "+"\n        ".join(self.availableTemplates)+"\n")
                    return
                if add_name in self.templates:
                    print("\n  [+] Warning: {} '{}' already added.\n".format(add_type, add_name))
                    return
                if len(self.templates) > 0:
                    template_name, templateInstance = self.templates.popitem()
                    print("\n  [+] Warning: Replacing {} '{}' with '{}'.\n".format(add_type, template_name, add_name))
                template, vba = self.helpers.templates[add_name]
                instance = template.Template(vba, self.helpers)
                self.templates[add_name] = instance
                print("\n  Added {} '{}'\n".format(add_type, add_name))
            elif add_type == 'obfuscator':
                if self.obfuscator is not None:
                    print("\n  [+] Warning: {} already added.\n".format(add_type))
                    return
                instance = Obfuscator(self.helpers)
                self.obfuscator = instance
                print("\n  Added {}\n".format(add_type))
            else:
                print("\n  [!] Error: Invalid usage of add")
                print("      Expecting: add <tempalate|module|bypass|obfuscator> [name]\n")
                return
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process add command: {}\n".format(msg))

    def complete_add(self, text, line, start_index, end_index):
        arguments = self.helpers.getarguments(line)
        results = []
        add_type = ""
        add_name = ""
        if len(arguments) > 1:
            add_type = arguments[1]
        if len(arguments) > 2:
            add_name = arguments[2]
        if add_type == 'module':
            if add_name in self.availableModules:
                results = []
            else:
                for key in self.availableModules:
                    if key.startswith(text):
                        results.append(key)
        elif add_type == 'bypass':
            if add_name in self.availableBypasses:
                results = []
            else:
                for key in self.availableBypasses:
                    if key.startswith(text):
                        results.append(key)
        elif add_type == 'template':
            if add_name in self.availableTemplates:
                results = []
            else:
                for key in self.availableTemplates:
                    if key.startswith(text):
                        results.append(key)
        elif add_type == 'obfuscator':
            results = []
        else:
            for key in ['module', 'template', 'bypass', 'obfuscator']:
                if key.startswith(text):
                    results.append(key)

        return results


    def do_delete(self, cmd):
        """
        delete <bypass|module|template|obfuscator> [name]
        
        Remove a bypass, module, or template with the specified name, or an 
        obfuscator (which does not take a name argument.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 2 and len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of delete")
                print("      Expecting: delete <tempalate|module|bypass|obfuscator> [name]\n")
                return
            delete_type = arguments[0]
            instance = None
            if delete_type == 'module':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of delete {}".format(delete_type))
                    print("      Expecting: delete {} <{}_name>\n".format(delete_type,delete_type))
                    return
                delete_name = arguments[1]
                if delete_name not in self.modules:
                    print("\n  [!] Error: {} '{}' not added.".format(delete_type, delete_name))
                    print("      The following {} are added:".format(delete_type))
                    print("        "+"\n        ".join(self.modules.keys())+"\n")
                    return
                del self.modules[delete_name]
                print("\n  Deleted {} '{}'\n".format(delete_type, delete_name))
            elif delete_type == 'bypass':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of delete {}".format(delete_type))
                    print("      Expecting: delete {} <{}_name>\n".format(delete_type,delete_type))
                    return
                delete_name = arguments[1]
                if delete_name not in self.bypasses:
                    print("\n  [!] Error: {} '{}' not added.".format(delete_type, delete_name))
                    print("      The following {} are added:".format(delete_type))
                    print("        "+"\n        ".join(self.bypasses.keys())+"\n")
                    return
                del self.bypasses[delete_name]
                print("\n  Deleted {} '{}'\n".format(delete_type, delete_name))
            elif delete_type == 'template':
                if len(arguments) != 2:
                    print("\n  [!] Error: Invalid usage of delete {}".format(delete_type))
                    print("      Expecting: delete {} <{}_name>\n".format(delete_type,delete_type))
                    return
                delete_name = arguments[1]
                if delete_name not in self.templates:
                    print("\n  [!] Error: {} '{}' not added.".format(delete_type, delete_name))
                    print("      The following {} are added:".format(delete_type))
                    print("        "+"\n        ".join(self.templates.keys())+"\n")
                    return
                del self.templates[delete_name]
                print("\n  Deleted {} '{}'\n".format(delete_type, delete_name))
            elif delete_type == 'obfuscator':
                if self.obfuscator is None:
                    print("\n  [!] Error: {} not added.\n".format(delete_type))
                    return
                self.obfuscator = None
                print("\n  Deleted {}\n".format(delete_type))
            else:
                print("\n  [!] Error: Invalid usage of delete")
                print("      Expecting: delete <tempalate|module|bypass|obfuscator> [name]\n")
                return
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process delete command: {}\n".format(msg))

    def complete_delete(self, text, line, start_index, end_index):
        arguments = self.helpers.getarguments(line)
        results = []
        delete_type = ""
        delete_name = ""
        if len(arguments) > 1:
            delete_type = arguments[1]
        if len(arguments) > 2:
            delete_name = arguments[2]
        if delete_type == 'module':
            if delete_name in self.modules:
                results = []
            else:
                for key in self.modules.keys():
                    if key.startswith(text):
                        results.append(key)
        elif delete_type == 'bypass':
            if delete_name in self.bypasses:
                results = []
            else:
                for key in self.bypasses.keys():
                    if key.startswith(text):
                        results.append(key)
        elif delete_type == 'template':
            if delete_name in self.templates:
                results = []
            else:
                for key in self.templates.keys():
                    if key.startswith(text):
                        results.append(key)
        elif delete_type == 'obfuscator':
            results = []
        else:
            for key in ['module', 'template', 'bypass', 'obfuscator']:
                if key.startswith(text):
                    results.append(key)
        return results


    def do_options(self, cmd):
        """
        options [name]
        
        Display the options for all bypasses, modules, template, and 
        obfuscator, or just the options for a bypass, module or template 
        specified by name.
        
        You can also display the options for the obfuscator by using 
        'obfuscator' as the name.
        
        The options description will let you know which values are required and
        they type of values they accept.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 0 and len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of options")
                print("      Expecting: options [name]\n")
                return
            option_name = ""

            if len(arguments)  > 0:
                option_name = arguments[0]
            if option_name != "":
                instance = None
                if option_name in self.modules:
                    instance = self.modules[option_name]
                elif option_name in self.bypasses:
                    instance = self.bypasses[option_name]
                elif option_name in self.templates:
                    instance = self.templates[option_name]
                elif option_name == "obfuscator":
                    instance = self.obfuscator
                else:
                    print("\n  [!] Error: Invalid usage of options")
                    print("      No module '{}' loaded.".format(option_name))
                    print("      Expecting: options (tempalate_name|module_name|bypass_name|obfuscator)\n")
                if instance is not None:
                    print("\n  Options:")
                    print("\n    {}".format(option_name))
                    for k, v in instance.options.items():
                        if 'hidden' in v and v['hidden'] is True:
                            continue
                        print("\n      {}".format(k))
                        print("        Value: {}".format(v['value']))
                        print("        Required: {}".format(v['required']))
                        print("        Description: {}".format(v['description']))
                    print("")
            else:
                print("\n  Options:")
                print("\n    bypasses:")
                if len(self.bypasses.items()) == 0:
                    print("\n      None added")
                for name, bypass in self.bypasses.items():
                    print("\n      {}".format(name))
                    for k, v in bypass.options.items():
                        if 'hidden' in v and v['hidden'] is True:
                            continue
                        print("\n        {}".format(k))
                        print("          Value: {}".format(v['value']))
                        print("          Required: {}".format(v['required']))
                        print("          Description: {}".format(v['description']))
                print("\n    modules:")
                if len(self.modules.items()) == 0:
                    print("\n      None added")
                for name, module in self.modules.items():
                    print("\n      {}".format(name))
                    for k, v in module.options.items():
                        if 'hidden' in v and v['hidden'] is True:
                            continue
                        print("\n        {}".format(k))
                        print("          Value: {}".format(v['value']))
                        print("          Required: {}".format(v['required']))
                        print("          Description: {}".format(v['description']))
                print("\n    templates:")
                if len(self.templates.items()) == 0:
                    print("\n      None added")
                for name, template in self.templates.items():
                    print("\n      {}".format(name))
                    for k, v in template.options.items():
                        if 'hidden' in v and v['hidden'] is True:
                            continue
                        print("\n        {}".format(k))
                        print("          Value: {}".format(v['value']))
                        print("          Required: {}".format(v['required']))
                        print("          Description: {}".format(v['description']))
                print("\n    obfuscator:")
                if self.obfuscator is None:
                    print("\n      None added")
                else:
                    for k, v in self.obfuscator.options.items():
                        if 'hidden' in v and v['hidden'] is True:
                            continue
                        print("\n      {}".format(k))
                        print("        Value: {}".format(v['value']))
                        print("        Required: {}".format(v['required']))
                        print("        Description: {}".format(v['description']))
                print("")
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process options command: {}\n".format(msg))

    def complete_options(self, text, line, start_index, end_index):
        results = []
        option_name = ""
        arguments = self.helpers.getarguments(line)
        if len(arguments) >= 1:
            if len(arguments) > 1:
                option_name = arguments[1]
            if not (option_name in self.modules or option_name in self.bypasses or option_name in self.templates or option_name == "obfuscator"):
                for module_name, module_instance in self.modules.items():
                    if module_name.startswith(text):
                        results.append(module_name)
                for bypass_name, bypass_instance in self.bypasses.items():
                    if bypass_name.startswith(text):
                        results.append(bypass_name)
                for template_name, template_instance in self.templates.items():
                    if template_name.startswith(text):
                        results.append(template_name)
                if self.obfuscator is not None and "obfuscator".startswith(text):
                        results.append("obfuscator")
        return results


    def do_unset(self, cmd):
        """
        unset <name> <option_name>
        
        Reset a specific option for a specifically named bypass, module, 
        template, or obfuscator.
        
        You can reset an option for the obfuscator by using 'obfuscator' as the 
        name.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 2:
                print("\n  [!] Error: Invalid usage of unset")
                print("      Expecting: unset <name> <option_name>\n")
                return
            unset_name = arguments[0]
            option_name = arguments[1]
            instance = None
            if unset_name in self.bypasses:
                instance = self.bypasses[unset_name]
            elif unset_name in self.modules:
                instance = self.modules[unset_name]
            elif unset_name in self.templates:
                instance = self.templates[unset_name]
            elif unset_name == 'obfuscator':
                instance = self.obfuscator
            else:
                print("\n  [!] Error: Invalid name.")
                print("      {} is not a module, template, bypass, nor obfuscator\n".format(unset_name))
                return
            instance.set_option(option_name)
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process unset command: {}\n".format(msg))

    def complete_unset(self, text, line, start_index, end_index):
        results = []
        unset_name = ""
        instance = None
        arguments = self.helpers.getarguments(line)
        if len(arguments) > 1:
            unset_name = arguments[1]
        if unset_name in self.modules:
            instance = self.modules[unset_name]
        elif unset_name in self.bypasses:
            instance = self.bypasses[unset_name]
        elif unset_name in self.templates:
            instance = self.templates[unset_name]
        elif unset_name == 'obfuscator':
            instance = self.obfuscator
        else:
            for module_name, module_instance in self.modules.items():
                if module_name.startswith(text):
                    results.append(module_name)
            for bypass_name, bypass_instance in self.bypasses.items():
                if bypass_name.startswith(text):
                    results.append(bypass_name)
            for template_name, template_instance in self.templates.items():
                if template_name.startswith(text):
                    results.append(template_name)
            if self.obfuscator is not None and "obfuscator".startswith(text):
                    results.append("obfuscator")
        if instance is not None:
            option_name = ""
            key_list = []
            for key in instance.options.keys():
                if 'hidden' not in instance.options[key]:
                    key_list.append(key)
                elif instance.options[key]['hidden'] == False:
                    key_list.append(key)
            if len(arguments) > 2:
                option_name = arguments[2]
            if option_name in key_list:
                results = []
            else:
                results = [
                    name for name in key_list
                    if name.startswith(text)
                ]
        return results


    def do_set(self, cmd):
        """
        set <name> <option_name> <option_value>
        
        Set a specific option to a specific value for a specifically named 
        
        bypass, module, template, or obfuscator.
        You can set an option for the obfuscator by using 'obfuscator' as the 
        name.
        
        An option may have to meet specific value requirements which will be 
        validated before setting.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) < 3:
                print("\n  [!] Error: Invalid usage of set")
                print("      Expecting: set <name> <option_name> <option_value> (option_args)\n")
                return
            set_name = arguments[0]
            option_name = arguments[1]
            #option_value = arguments[2]
            option_value = ' '.join(arguments[2:])
            instance = None
            if set_name in self.bypasses:
                instance = self.bypasses[set_name]
            elif set_name in self.modules:
                instance = self.modules[set_name]
            elif set_name in self.templates:
                instance = self.templates[set_name]
            elif set_name == 'obfuscator':
                instance = self.obfuscator
            else:
                print("\n  [!] Error: Invalid name.")
                print("      {} is not a module, template, bypass, nor obfuscator\n".format(set_name))
                return
            return_status = instance.set_option(option_name, option_value)
            if return_status == 0:
                if len(option_value) > 1:
                    print("\n  Set {}.{} to {}\n".format(set_name,option_name,option_value))
                else:
                    print("\n  Reset {}.{}\n".format(set_name,option_name))
            elif return_status == 1:
                print("\n  [!] Error: Option failed validation, so refusing to set {} to {}\n".format(option_name,option_value))
            elif return_status == 2:
                print("\n  [!] Error: No option with name '{}', so failed to set\n".format(option_name))
            
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process set command: {}\n".format(msg))

    def complete_set(self, text, line, start_index, end_index):
        results = []
        set_name = ""
        instance = None
        arguments = self.helpers.getarguments(line)
        if len(arguments) > 1:
            set_name = arguments[1]
        if set_name in self.modules:
            instance = self.modules[set_name]
        elif set_name in self.bypasses:
            instance = self.bypasses[set_name]
        elif set_name in self.templates:
            instance = self.templates[set_name]
        elif set_name == 'obfuscator':
            instance = self.obfuscator
        else:
            for module_name, module_instance in self.modules.items():
                if module_name.startswith(text):
                    results.append(module_name)
            for bypass_name, bypass_instance in self.bypasses.items():
                if bypass_name.startswith(text):
                    results.append(bypass_name)
            for template_name, template_instance in self.templates.items():
                if template_name.startswith(text):
                    results.append(template_name)
            if self.obfuscator is not None and "obfuscator".startswith(text):
                    results.append("obfuscator")
        if instance is not None:
            option_name = ""
            key_list = []
            for key in instance.options.keys():
                if 'hidden' not in instance.options[key]:
                    key_list.append(key)
                elif instance.options[key]['hidden'] == False:
                    key_list.append(key)
            if len(arguments) > 2:
                option_name = arguments[2]
            if option_name in key_list:
                if 'tab_complete' in instance.options[option_name]:
                    args = {}
                    if 'tab_args' in instance.options[option_name]:
                        args = instance.options[option_name]['tab_args']
                        if not isinstance(args, dict):
                            print("\n  [!] Error: tab_args for argument {} is invalid\n".format(option_name))
                            args = {}
                    results = instance.options[option_name]['tab_complete'](text, line, **args)
                else:
                    results = []
            else:
                results = [
                    name for name in key_list
                    if name.startswith(text)
                ]
        return results


    def do_dump_history(self, cmd):
        """
        dump_history <file_name>
        
        Writes the VenomousSway command history to a file.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of dump_history")
                print("      Expecting: dump_history <file_name>\n")
                return
            file_name = arguments[0]
            readline.write_history_file(file_name)
            
            print("\n  Dumped history to {}\n".format(file_name))
            
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process dump_history: {}\n".format(msg))

    def complete_dump_history(self, text, line, start_index, end_index):
        return self.helpers.complete_path(text, line)


    def do_save_config(self, cmd):
        """
        save_config <file_name>
        
        Saves the current VenomousSway configuration to a file.
        
        This includes all the modules, bypasses, template, and obfuscator added 
        as well as their optiosn.
        """
        
        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of save_config")
                print("      Expecting: save_config <file_name>\n")
                return
            file_name = arguments[0]
            configSettings = {'templates': {}, 'modules': {}, 'bypasses': {}, 'obfuscator': {}}
            print("\n  Saving {} bypasses...".format(len(self.bypasses)))
            for k,v in self.bypasses.items():
                configSettings['bypasses'][k] = {}
                for name, opt in v.options.items():
                    configSettings['bypasses'][k][name] = opt['value']
            print("  Saving {} modules...".format(len(self.modules)))
            for k,v in self.modules.items():
                configSettings['modules'][k] = {}
                for name, opt in v.options.items():
                    configSettings['modules'][k][name] = opt['value']
            print("  Saving {} templates...".format(len(self.templates)))
            for k,v in self.templates.items():
                configSettings['templates'][k] = {}
                for name, opt in v.options.items():
                    configSettings['templates'][k][name] = opt['value']
            if self.obfuscator is not None:
                print("  Saving obfuscator...")
                v = self.obfuscator
                for name, opt in v.options.items():
                    configSettings['obfuscator'][name] = opt['value']
            with open(file_name, 'w') as fp:
                fp.write(json.dumps(configSettings))
                
            print("  Saved configuration to {}\n".format(file_name))
            
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process save_config: {}\n".format(msg))

    def complete_save_config(self, text, line, start_index, end_index):
        return self.helpers.complete_path(text, line)


    def do_load_config(self, cmd):
        """
        load_config <file_name>
        
        Loads a saved VenomousSway configuration from a file.
        
        This includes all the modules, bypasses, template, and obfuscator to be 
        added as well as their optiosn.
        """
        
        try:
            self.templates = {}
            self.modules = {}
            self.bypasses = {}
            self.obfuscator = None
            configSettings = {}
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 1:
                print("\n  [!] Error: Invalid usage of load_config")
                print("      Expecting: load_config <file_name>\n")
                return
            file_name = arguments[0]
            with open(file_name, 'r') as fp:
                configSettings = json.load(fp)
            print("\n  Loading {} bypasses...".format(len(configSettings['bypasses'])))
            for name, opts in configSettings['bypasses'].items():
                bypass, vba = self.helpers.bypasses[name]
                print("    Loading {}...".format(name))
                instance = bypass.Bypass(vba, self.helpers)
                for optname, optval in opts.items():
                    instance.set_option(optname, optval)
                self.bypasses[name] = instance
                instance = None
            print("  Loading {} modules...".format(len(configSettings['modules'])))
            for name, opts in configSettings['modules'].items():
                module, vba = self.helpers.modules[name]
                print("    Loading {}...".format(name))
                instance = module.Module(vba, self.helpers)
                for optname, optval in opts.items():
                    instance.set_option(optname, optval)
                self.modules[name] = instance
                instance = None
            print("  Loading {} templates...".format(len(configSettings['templates'])))
            for name, opts in configSettings['templates'].items():
                template, vba = self.helpers.templates[name]
                print("    Loading {}...".format(name))
                instance = template.Template(vba, self.helpers)
                for optname, optval in opts.items():
                    instance.set_option(optname, optval)
                self.templates[name] = instance
                instance = None
                                
            if 'obfuscator' in configSettings:
                print("  Loading obfuscator...")
                opts = configSettings['obfuscator']
                if len(opts) > 1:
                    instance = Obfuscator(self.helpers)
                    for optname, optval in opts.items():
                        instance.set_option(optname, optval)
                    self.obfuscator = instance
                    instance = None
            print("  Loaded configuration from {}\n".format(file_name))
            
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process load_config: {}\n".format(msg))

    def complete_load_config(self, text, line, start_index, end_index):
        return self.helpers.complete_path(text, line)


    def do_generate(self, cmd):
        """
        generate [file_name]
        
        Generates a VBA module using the added bypasses, modules, template, 
        and/or obfuscator and their configured options.
        
        VenoumousSway will first check for all required options, process all 
        the modules, and finally obfuscate the results if the obfuscator was 
        added.
        
        The results will be displayed to the terminal unless an output file 
        name is specified.
        """
        
        check_success = True
        bypassCalls = []
        bypassCode = []
        moduleCalls = []
        moduleCode = []
        declares = set()
        context = {}
        generatedCode = ""
        declaresStub = ""
        fileName = ""

        try:
            arguments = self.helpers.getarguments(cmd)
            if len(arguments) != 1 and len(arguments) != 0:
                print("\n  [!] Error: Invalid usage of generate")
                print("      Expecting: generate [output_file]\n")
                return
            if len(arguments) == 1:
                fileName = arguments[0]

            print("\n  Checking for required options...")
            for name, bypass in self.bypasses.items():
                check_success = bypass.check_required() and check_success
            for name, module in self.modules.items():
                check_success = module.check_required() and check_success
            for name, template in self.templates.items():
                check_success = template.check_required() and check_success
            if self.obfuscator is not None:
                check_success = self.obfuscator.check_required() and check_success
            if not check_success:
                print("\n  [!] Error: Not all required options are set.\n")
                return

            print("  Preprocessing...")
            for name, bypass in self.bypasses.items():
                bypass.preprocess()
            for name, module in self.modules.items():
                module.preprocess()
            for name, template in self.templates.items():
                template.preprocess()
            if self.obfuscator is not None:
                self.obfuscator.preprocess()

            print("  Processing...")
            for name, bypass in self.bypasses.items():
                generatedCode = bypass.process(context)
                declares = declares.union(bypass.declares)
                bypassCalls.append("{}".format(name))
                bypassCode.append(generatedCode)
            context['bypassCode'] = "\n".join(bypassCode)
            context['bypassCalls'] = "\n".join(bypassCalls)
            for name, module in self.modules.items():
                generatedCode = module.process(context)
                declares = declares.union(module.declares)
                moduleCalls.append("If {} Then Exit Sub".format(name))
                moduleCode.append(generatedCode)
            context['moduleCode'] = "\n".join(moduleCode)
            context['moduleCalls'] = "\n".join(moduleCalls)
            template = None
            for name, template in self.templates.items():
                generatedCode = template.process(context)
                declares = declares.union(template.declares)

            print("    Including declares...")
            declaresStub = "\n".join(declares) + "\n"
            generatedCode = declaresStub + generatedCode

            if self.obfuscator is not None:
                wordMappings = {}
                print("    Running obfuscator...")
                if self.DEBUG:
                    with open("DEBUG-preobfuscation.vba", 'w') as fp:
                        fp.write(generatedCode)
                generatedCode = self.helpers.obfuscatevba(generatedCode, self.obfuscator, wordMappings)
                if template is not None and template.name in wordMappings:
                    print("    [*] Encoded template function: {}".format(wordMappings[template.name]))

            if fileName != "":
                with open(fileName, 'w') as fp:
                    fp.write(generatedCode)
                print("    Wrote generated code to file: {}".format(cmd))
            else:
                print("\n=======================================================\n")
                print(generatedCode)
                print("\n=======================================================\n")

            print("  Postprocessing...")
            for name, bypass in self.bypasses.items():
                bypass.postprocess()
            for name, module in self.modules.items():
                module.postprocess()
            for name, template in self.templates.items():
                template.postprocess()
            if self.obfuscator is not None:
                self.obfuscator.postprocess()

            print("  Cleaning...")
            for name, bypass in self.bypasses.items():
                bypass.cleanup()
            for name, module in self.modules.items():
                module.cleanup()
            for name, template in self.templates.items():
                template.cleanup()
            if self.obfuscator is not None:
                self.obfuscator.cleanup()
            
            print("\n  Generation complete.\n")
            
        except Exception as msg:
            if self.DEBUG:
                traceback.print_exc()
            print("\n  [!] Error: Failed to process generate command: {}\n".format(msg))

    def complete_generate(self, text, line, start_index, end_index):
        return self.helpers.complete_path(text, line)


    def emptyline(self):
        return


    def do_exit(self, cmd):
        """
        exit
        
        Exits the program.
        """
        
        print("\n  Shutting down...\n")
        return True

