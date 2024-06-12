# History
This is a macro generation framework that predated venomoussway.  It was one of the first developed and was a quick and dirty project to meet the need quickly at the time.

# Macro Obfuscator And Sandbox Check Generator
This is just a quick hacked together script to obfuscate a Payload and add in checks to the payload.


## Help menu and Example
```
python generateMacro.py -c "0,1,2,3,4,5,6" -o ./output/testgenerated.txt -f ./Payloads/CustomStager.vba
```

```
Usage: generateMacro.py [options]

Options:
  -h, --help            show this help message and exit
  -c CHOICES, --choices=CHOICES
                        Choices (ex. 0,2,3,4)
  -o OUTPUT, --output=OUTPUT
                        Output
  -f INFILE, --infile=INFILE
                        Template input file.
  -t MACROTYPE, --type=MACROTYPE
                        Type of script vba, vbs, hta, or js
  -s STRINGTYPES, --stringtypes=STRINGTYPES
                        String obfuscation types. 1 - replace method, 2 - chr
                        method
  -l                    List sandbox checks available.

```


## Usage 
To use copy the payload template to a temporary location. Once the template 
is copied over modify the template to do the action you need it to. Then 
test to make sure it does what you want, if it doesn't fix it until it does
once it works then run generateMacro (as shown above) with the choices you 
want to include as a comma seperated list. Once done you should have a 
generated macro in the output location.

1. Copy template you want to use to /tmp/ or somewhere else.
2. Modify template to have the action you want to do.
3. Setup custom stager if using that.
4. Test locally
5. Run generateMacro with your options.
6. Test generated macro locally.
7. If it works as planned deploy.

## Checks Available
The following checks are available for vba's.

```
python generateMacro.py -l
Checks Available:
    0 : checkPartOfDomain.txt
    1 : checkUsername.txt
    2 : checkCores.txt
    3 : checkFilenameHash.txt
    4 : checkRecentDocs.txt
    5 : checkBios.txt
    6 : checkPrinter.txt
    7 : checkFilenameBad.txt
```

## Payloads Available
The following payload templates are available, Please only use the vba's 
until I test out all the vbs's and make sure the obfuscation works 100% 
of the time.

```
Payloads/
├── jscript
├── README.md
├── vba
│   ├── CustomInjection_MutiArch.vba
│   ├── CustomInjection.vba
│   ├── CustomInjectionx64.vba
│   ├── CustomStager_MultiArch.vba
│   ├── CustomStager.vba
│   ├── WMIExecEvalTemplate.vba
│   └── WMIExec.vba
└── vbs
    ├── CustomInjection.vbs
    ├── CustomStaging.vbs
    └── WMIExec.vbs
```
