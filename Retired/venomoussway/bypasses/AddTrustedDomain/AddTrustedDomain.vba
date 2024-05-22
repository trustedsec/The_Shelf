Function AddTrustedDomain_RegKeySave(szRegKey As String, szValue As String, szType As String)
    Dim objWscriptShell As Object
	On Error Resume Next
    Set objWscriptShell = CreateObject("WScript.Shell")
    objWscriptShell.RegWrite szRegKey, szValue, szType
End Function

Sub AddTrustedDomain()
    
    Dim szUrl As String
    Dim szProtocol As String
    Dim szDomainName As String
    Dim szHostName As String
    Dim nLastDotIndex As Integer
    Dim nSecondLastDotIndex As Integer
    Dim nThirdSlashIndex As Integer
    Dim nColonIndex As Integer
    Dim nSecondColonIndex As Integer

    szUrl = "{{TrustedUrl}}"

    nColonIndex = InStr(szUrl, ":")
    If nColonIndex = 0 Then
        End
    End If
    
    nSecondColonIndex = InStr(10, szUrl, ":")
    If nSecondColonIndex = 0 Then
        nThirdSlashIndex = InStr(10, szUrl, "/")
        If nThirdSlashIndex = 0 Then
            nThirdSlashIndex = Len(szUrl)
        Else
            nThirdSlashIndex = nThirdSlashIndex - 1
        End If
    Else
        nThirdSlashIndex = nSecondColonIndex - 1
    End If
    
    nLastDotIndex = InStrRev(szUrl, ".", nThirdSlashIndex)
    If nLastDotIndex = 0 Then
        End
    End If

    nSecondLastDotIndex = InStrRev(szUrl, ".", nLastDotIndex - 1)
    If nSecondLastDotIndex = 0 Then
        End
    End If
    
    szProtocol = Left(szUrl, nColonIndex - 1)
    szHostName = Mid(szUrl, nColonIndex + 3, nSecondLastDotIndex - nColonIndex - 3)
    szDomainName = Mid(szUrl, nSecondLastDotIndex + 1, nThirdSlashIndex - nSecondLastDotIndex)
    
    AddTrustedDomain_RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Internet Settings\ZoneMap\Domains\" & szDomainName & "\" & szHostName & "\" & szProtocol, 2, "REG_DWORD"
           
End Sub
