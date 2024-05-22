Public Function checkFilenameBad()
    checkFilenameBad = False
    Dim badNames({{filenames|length -1}}) As String
    {% for name in filenames %}badNames({{loop.index0}}) = "{{name}}"
    {% endfor %}

    For Each n In badNames
        If InStr(LCase({{doctype}}.FullName), n) > 0 Then
            checkFilenameBad = True
        End If
    Next


End Function

