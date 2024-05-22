import os
import sys
import binascii
import base64
import random
import string

def obfuscateStringChr(instring):
    outstring = []
    tempstring = ""
    skipval = 0
    for count, char in enumerate(instring):
        #print(char)
        #print("%d out of %d"%(count, len(instring)))
        if skipval != 0:
            skipval -= 1
            tempstring += char
            if skipval == 0 or count == len(instring)-1:
                tempstring += "\""
                #print(tempstring)
                outstring.append(tempstring)
                tempstring = ""
            continue
        randval = random.randint(0, 8)
        if randval > 3 or randval == 0 or count == len(instring)-1:
            outstring.append("chr(%d)"%(ord(char)))
        else:
            tempstring = "\""+ char
            skipval = randval

    return " & ".join(outstring)


class MacroClass:

    def __init__(self, options=""):
        self.options = options
        self.verbose = 0

    '''Generate string of random length, or specified size.'''
    def string_generator(self, size=0, chars=string.ascii_lowercase):
        if size == 0:
            size = random.randint(4, 10)
        return ''.join(random.choice(chars) for _ in range(size))
    
    def split_every(self, n, s):
        return [ s[i:i+n] for i in range(0, len(s), n) ]

    def generate_vba_array(self, content, varname="serialized_obj"):
        content_hex = binascii.hexlify(content).decode('utf-8')
        if self.verbose == 1:
            print("Hex: %s"%(content_hex))
        payloadArrays = self.split_every(64, content_hex)
        payload_string = "%s = \""%(varname)
        for count, item in enumerate(payloadArrays):
            if count == len(payloadArrays)-1:
                payload_string = payload_string+item+"\"\n"
            else:
                payload_string = payload_string+item+"\"\n    %s = %s & \""%(varname, varname)
        return payload_string

    def generate_js_array(self, content):
        """Content is bytes type, encode this way, then decode to string"""
        payload_string = base64.b64encode(content).replace(b"=", b"A").decode('utf-8')
        n = 64
        payload_string_array = [payload_string[i:i+n] for i in range(0, len(payload_string), n)]
        payload_string = ""
        for count, item in enumerate(payload_string_array):
            if count == len(payload_string_array)-1:
                payload_string = payload_string + item
            else:
                payload_string = payload_string + item + "\"+\n\""
        return payload_string

    def generate_vbs_array(self, content, varname="s"):
        payload_string = base64.b64encode(content).replace(b"=", b"A").decode('utf-8')
        n = 64
        payload_string_array = [payload_string[i:i+n] for i in range(0, len(payload_string), n)]
        payload_string = ""
        for count, item in enumerate(payload_string_array):
            if count == len(payload_string_array)-1:
                payload_string = payload_string+item
            else:
                payload_string = payload_string + item + "\"\n%s = %s & \""%(varname, varname)
        return payload_string
 
    
    def getVariableNames(self, inputdata, functionnames, stringobfuscation, stype="vbs"):
        variables = set()
        stringlist = []
        firststringlist = []
        inputdataCleaned = ""
        for line in inputdata.split("\n"):
            clean = line.lstrip()
            if clean.startswith("'"):
                #ignore
                print("Skipping line")
                continue
            elif "=" in clean:
                variable = clean.split("=")[0].lstrip().rstrip()
                if stype == "vbs":
                    if "Set " in variable:
                        variable = variable.replace("Set ", "")
                if stype == "js":
                    if "var " in variable:
                        variable = variable.replace("var ", "")
                if " " not in variable and "." not in variable:
                    variables.add(variable)
            if "\"" in line:
                quotecount = line.count("\"")
                strings = line.split("\"")
                for count in range(int(quotecount/2)):
                    #print(strings[count*2+1])
                    #Only do the serialized_obj obfuscation with method 1
                    if stringobfuscation == 1:
                        stringlist.append(strings[count*2+1])
                    elif stringobfuscation == 2:
                        if "serialized_obj" in line:
                            print("Skipping line")
                        #elif "RunClass" in line or "DynamicInvoke" in line or "SurrogateSelector" in line or "Deserialize_2" in line:
                        #    firststringlist.append(strings[count*2+1])
                        else:
                            stringlist.append(strings[count*2+1])
            inputdataCleaned = inputdataCleaned + line+"\n"

        orderedVars = sorted(variables, key=len)
        orderedFuncs = sorted(functionnames, key=len)
        return inputdataCleaned, stringlist, orderedVars, orderedFuncs

    def obfuscate(self, inputdata, functionnames, stringobfuscation, stype="vbs"):
        #print("Variables:")
        #for item in variables:
        #    print("\t%s"%(item))
        inputdataCleaned, stringlist, orderedVars, orderedFuncs = self.getVariableNames(inputdata, functionnames, stringobfuscation, stype)
        mappings = {}
        used = []
        print("Variables Ordered:")
        for item in orderedVars[::-1]:
            substitutestring = None
            while substitutestring is None:
                substitutestring = self.string_generator()
                if substitutestring in used:
                    substitutestring = None
            mappings[item] = substitutestring
            used.append(substitutestring)
            print("\t%s"%(item))
        print("Functions Ordered:")
        for item in orderedFuncs[::-1]:
            if item not in mappings.keys():
                substitutestring = None
                while substitutestring is None:
                    substitutestring = self.string_generator()
                    if substitutestring in used:
                        substitutestring = None
                mappings[item] = substitutestring
                used.append(substitutestring)
                print("\t%s"%(item))
        
        results = inputdataCleaned
        for item in mappings.keys():
            results = results.replace(item, mappings[item])
        for tempstring in stringlist:
            replacementstring = ""
            if stringobfuscation == 1 and stype == "js":
                for item in tempstring:
                    if random.randint(0, 1) == 0:
                        replacementstring = replacementstring+item
                    else:
                        replacementstring = replacementstring +item+"|"
                if tempstring != "\"\"" and tempstring != "":
                    results = results.replace("\"%s\""%(tempstring), "\"%s\".split('|').join('')"%(replacementstring))
            elif stringobfuscation == 1:
                for item in tempstring:
                    if random.randint(0, 1) == 0:
                        replacementstring = replacementstring+item
                    else:
                        replacementstring = replacementstring +item+"|"
                results = results.replace("\"%s\""%(tempstring), "Replace(\"%s\", \"|\", \"\")"%(replacementstring))
            elif stringobfuscation == 2:
                temptempstring = tempstring
                if "RunClass" in tempstring or "DynamicInvoke" in tempstring or "SurrogateSelector" in tempstring or "Deserialize_2" in tempstring:
                    for tempkey in mappings.keys():
                        temptempstring = temptempstring.replace(tempkey, mappings[tempkey])
                replacementstring = obfuscateStringChr(temptempstring)
                results = results.replace("\"%s\""%(temptempstring), replacementstring)

        return results
 

