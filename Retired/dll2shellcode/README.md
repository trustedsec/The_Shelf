This project was originally used to convert some outputs of our internal payload packer.
It was heavily inspired by https://github.com/monoxgas/sRDI 

This project won't build if you clone it down as it relies on a non-public argument parsing library. If you would like to try to use this you will need to flip out argument handling.

# Dll2Shellcode

This Project converts any standard dll to shellcode, regardless of if it has a reflective loader export or not

## usage

./Dll2Shellcode \<Input Dll path\> \<output shellcode path\>  

Additional options:  
     -NoClearHeaders (switch): Does not stomp the PE headers in the loaded section  
     -NoClearStagingMemory (switch): Does not free the used memory of the initial dll blob load  
     -DelayImports (int): Adds the specified number of seconds between each dll import resolution  
     -ExportToCall (string): Calls the specified dll export after running dllmain.  Called export is started using beginthreadex  

## examples

(convert don't stomp PE header, Delay 5 seconds between import resolutions, and call ExploitHarder after dllmain is run)  
./Dll2Shellcode ~/payload.dll ~/payload.bin -NoClearHeaders -DelayImports 5 -ExportToCall ExploitHarder

(convert stomp headers and clear staged memory)  
./Dll2Shellcode ~/payload.dll ~/payload.bin
