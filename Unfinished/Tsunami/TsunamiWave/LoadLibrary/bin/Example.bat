echo Configure a binary with a random key
del ConfigBinary.py
del LoadLibrary_x64_Release.exe
copy ..\..\ConfigBinary\bin\ConfigBinary.py .
dir ConfigBinary.py
python ConfigBinary.py --debug --input EXE_LoadLibrary_x64_Release.exe --output LoadLibrary_x64_Release.exe --random
dir LoadLibrary_x64_Release.exe
