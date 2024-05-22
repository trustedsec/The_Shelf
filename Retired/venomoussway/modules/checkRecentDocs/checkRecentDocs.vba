Public Function checkRecentDocs()
    If Application.RecentFiles.Count < 3 Then
        checkRecentDocs = True
    Else
        checkRecentDocs = False
    End If
End Function
