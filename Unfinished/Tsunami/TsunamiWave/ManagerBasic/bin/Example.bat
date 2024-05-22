@echo off
for %%I in (..) do set CurrentModuleName=%%~nxI
echo Deleting old files... 
del ConfigureBinary.py
del %CurrentModuleName%_x64_Release.exe
echo Copying the most recent ConfigureBinary script...
copy ..\..\Utilities\bin\ConfigureBinary.py .
echo Configuring %CurrentModuleName%...
python ConfigureBinary.py --debug --input EXE_%CurrentModuleName%_x64_Release.exe --output %CurrentModuleName%_x64_Release.exe --random
dir %CurrentModuleName%_x64_Release.exe
echo Command line to run:
echo %CurrentModuleName%_x64_Release.exe DLL_SurveyProcess_x64_Release.dll survey_process_request_c2.json
