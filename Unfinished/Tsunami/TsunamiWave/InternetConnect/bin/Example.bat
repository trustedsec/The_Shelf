@echo off
for %%I in (..) do set CurrentModuleName=%%~nxI
echo Deleting old files... 
del ConfigureAgent.py
del %CurrentModuleName%_x64_Release.exe
echo Copying the most recent configuration script...
copy ..\..\Utilities\bin\ConfigureAgent.py .
echo Configuring %CurrentModuleName%...
python ConfigureAgent.py --debug --input EXE_%CurrentModuleName%_x64_Release.exe --output %CurrentModuleName%_x64_Release.exe --random --config AgentConfig.json
dir %CurrentModuleName%_x64_Release.exe
echo Command line to run:
echo %CurrentModuleName%_x64_Release.exe
