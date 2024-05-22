from argparse import ArgumentParser
import os
vbamod_base = """
Function {}
{} = False
End Function
"""

vbatemplate_base = """
{{{{modulecode}}}}

Sub {}
{{{{modulecalls}}}}
End Sub
"""

pymod_base = """
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring

class Module(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {{}}
        self.helpers = helpers
{}
        super().__init__(templatePath, options)
"""

pytemplate_base = """
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring

class Template(ModuleInterface):
    def __init__(self, templatePath, helpers):
        options = {{}}
        self.helpers = helpers
{}
        super().__init__(templatePath, options)
"""

option_base = \
"""        options['{}'] = {{ #list of strings for expected options
            "value": None,
            "required": True,
            "description": "<Replace with your description>",
            "validator": None,
            "handler": None
        }}"""

def main():
    parser = ArgumentParser(description="Generate a new module template")
    modtypetitle = parser.add_argument_group(title="Module Type")
    modtype = modtypetitle.add_mutually_exclusive_group(required=True)
    modtype.add_argument("--template", action="store_true", help="This is a base exploit template")
    modtype.add_argument("--module", action="store_true", help="This is a module that performs a runtime check")
    parser.add_argument("name", type=str, help="Filename for the .py and .vba file we will generate")
    parser.add_argument("DestFolder", type=str, help="Path to parent folder to create this new item in")
    parser.add_argument("options", nargs="*", type=str, help="what options should be available in this module")

    args = parser.parse_args()
    if args.template:
        vbabase = vbatemplate_base
        pybase = pytemplate_base
    else:
        vbabase = vbamod_base
        pybase = pymod_base

    if not os.access(args.DestFolder, os.W_OK):
        print("Could not open the specified destination folder for writing")
        return
    args.DestFolder = os.path.join(args.DestFolder, args.name)
    os.mkdir(args.DestFolder)
    vbapath = os.path.join(args.DestFolder, args.name + ".vba")
    pypath = os.path.join(args.DestFolder, args.name + ".py")

    with open(vbapath, 'w') as fp:
        fp.write(vbabase.format(args.name, args.name))
    alloptions = []
    for item in args.options:
        alloptions.append(option_base.format(item))
    with open(pypath, 'w') as fp:
        fp.write(pybase.format("\n".join(alloptions)))








if __name__ == "__main__":
    main()