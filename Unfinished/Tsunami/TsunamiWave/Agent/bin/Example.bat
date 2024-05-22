@echo off
for %%I in (..) do set CurrentModuleName=%%~nxI
echo Deleting old files...
del /Q /F %CurrentModuleName%_x64_Release.exe
del /Q /S /F ..\..\TsunamiServer\agent\12345\6789
mkdir ..\..\TsunamiServer\agent\12345\6789
mkdir ..\..\TsunamiServer\agent\12345\6789\admin\
mkdir ..\..\TsunamiServer\agent\12345\6789\uploads\
mkdir ..\..\TsunamiServer\agent\12345\6789\sents\
mkdir ..\..\TsunamiServer\agent\12345\6789\downloads\
copy /Y ..\..\TsunamiServer\tsunami.py .\tsunami.py
echo Configuring %CurrentModuleName%...
python ConfigureAgent.py --debug --input EXE_%CurrentModuleName%_x64_Release.exe --output %CurrentModuleName%_x64_Release.exe --random --config TestAgentConfig.json
echo Copying new files...
copy /Y /B .\%CurrentModuleName%_x64_Release.exe ..\..\TsunamiServer\agent\12345\6789\admin\TsunamiWave_x64.exe
copy /Y .\AgentConfig.json ..\..\TsunamiServer\agent\12345\6789\admin\AgentConfig.json
copy /Y .\message_id ..\..\TsunamiServer\agent\12345\6789\admin\message_id
start cmd.exe /K "cd ..\..\TsunamiServer\agent\12345\6789\admin\ && dir"