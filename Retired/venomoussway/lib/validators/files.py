import traceback
import os

def isreadable(path, **kwargs):
    errmsg = None
    try:
        if path.lower() == "none":
            return True
        ret = os.access(path, os.R_OK)
        if ret == False:
            print("Could not read specified file")
        return ret
    except Exception as msg:
        traceback.print_exc()
        print('Exception checking path')
        return False

def isbasename(path, **kwargs):
    try:
        if '\\' in path:
            print("Path given should be basename only, do not use a full path")
            return False
        else:
            return True
    except Exception as msg:
        traceback.print_exc()
        print('Exception checking path')
        return False


def isvalidfilename(val, **kwargs):
    bad_charlist = ['/', '\\', ':', '?', '*', '\"', '|', ' ']
    try:
        if len(val) > 260:
            errmsg = "  [!] Error: Invalid file name. Cannot be longer than 260 characters."
            print(errmsg)
            return False
        
        if any ([x in val for x in bad_charlist]):
            errmsg = "  [!] Error: Invalid file name. Cannot contain: '/', '\\', ':', '?', '*', '\"', '|', ' '"
            print(errmsg)
            return False
            
        return True
    except:
        traceback.print_exc()
        print("  [!] Exception checking filename")
        return False




