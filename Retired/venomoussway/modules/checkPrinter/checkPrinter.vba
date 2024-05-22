Public Function checkPrinter() As Boolean
    checkPrinter = True
    Set objWMIService = GetObject("winmgmts:\\.\root\CIMV2")
    Set colItems = objWMIService.ExecQuery("SELECT Name FROM Win32_Printer")
    For Each p In colItems
        If (p.Name Like "*Microsoft*" Or p.Name Like "*Fax*" Or p.Name Like "*OneNote*") = True Then
            checkPrinter = False
        End If
    Next
End Function
