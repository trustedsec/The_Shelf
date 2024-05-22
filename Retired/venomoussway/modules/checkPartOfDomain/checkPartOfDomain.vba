Public Function checkPartOfDomain()
    checkPartOfDomain = True
    Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
    Set colItems = objWMIService.ExecQuery("Select * from Win32_ComputerSystem", , 48)

    For Each objItem In colItems
        If objItem.partOfDomain Then
            checkPartOfDomain = False
        End If
    Next

End Function
