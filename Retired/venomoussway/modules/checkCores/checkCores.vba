Public Function checkCores()
    checkCores = False
    Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
    Set colItems = objWMIService.ExecQuery("Select * from Win32_Processor", , 48)

    For Each objItem In colItems
            If objItem.NumberOfCores < {{coreCount}} Then
                checkCores = True
            End If
    Next
End Function

