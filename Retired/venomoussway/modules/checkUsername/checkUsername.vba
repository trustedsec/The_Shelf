Public Function checkUsername()
    checkUsername = False
    Dim badUsernames As Variant
    badUsernames = Array("admin", "malfind", "sandbox", "test")

    Set objWMIService = GetObject("winmgmts:\\.\root\cimv2")
    Set colItems = objWMIService.ExecQuery("Select * from Win32_ComputerSystem", , 48)

    For Each objItem In colItems
        For Each n In badUsernames
            If InStr(LCase(objItem.UserName), n) > 0 Then
                checkusername = True
            End If
        Next
    Next
End Function
