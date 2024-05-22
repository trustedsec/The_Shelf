Public Function checkFilenameHash()
    Dim hexchars As String
    Dim Count As Long
    hexchars = "0123456789abcdef"
    Count = 0
    Dim dname As String
    If InStrRev({{doctype}}.name, ".") <> 0 Then
        dname = Left({{doctype}}.name, InStrRev({{doctype}}.name, ".") - 1)
    Else
        dname = {{doctype}}.name
    End If
    For tempcounter = 1 To Len(dname)
        tempvariable = Mid(LCase(dname), tempcounter, 1)

        If InStr(hexchars, tempvariable) > 0 Then
            Count = Count + 1
        End If

    Next

    If Count >= Len(dname) Then
        checkFilenameHash = True
    Else
        checkFilenameHash = False
    End If
End Function
