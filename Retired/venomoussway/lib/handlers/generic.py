from lib.core import helpers
def quotedstring(value, **kwargs):
    if value[0] != "\"":
        return '"{}"'.format(value)
    return value

def escapebackslash(value, **kwargs):
    if "\\" in value:
        return value.replace("\\", "\\\\")
    return value

def makeint(value, **kwargs):
    return int(value)

def makelist(value, **kwargs):
    args = helpers.Helpers.getarguments(value)
    return args

def escapequotes(value, **kwargs):
    return value.replace('"', '""').replace("\\", "\\\\")

def makebool(value, **kwargs):
    if value.lower() == 'true':
        return True
    else:
        return False