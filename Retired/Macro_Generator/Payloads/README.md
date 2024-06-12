# Payload Usage
Just a simple writeup of what payloads exist and what to use them for. 

## HTA
Due to how an hta would have to be ran when phishing the payload has to be 
32 bit. So for the vbs code I'm leaving this just 32 bit only.

## VBA
So for this it can be either 32 or 64 bit, so because of that I generated a 
MultiArch payload for CustomInjection and CustomStager that should determine 
if its a 32 and 64 bit process and can load different follow on stagers, or 
run different shellcode (mostly used so 64 bit can use Wow64GetThreadContext 
instead of GetThreadContext).



## Payloads

### CustomInjection
Starts a process, then injects code into it.
Values to change
``` 
    dllPath = "BINARY_TO_SPAWN"
    ShellCode = "BASE64_ENCODED_SHELLCODE"
```
Replace with the binary you want to inject into, and the Shellcode that you to
have injected into the process.

### CustomStager
Gets encrypted follow on stage from server and loads it into the process.
Values to change.
```
    dllPath = "URL_TO_STAGE"
```
Replace with the paths to the follow on stage (the survey or stage itself)

### WMIExec
This just runs a command through WMI and returns back.
Values to change.
```
    dllPath = "CommandLineToRun"
```
Replace with the command line you want to execute.
