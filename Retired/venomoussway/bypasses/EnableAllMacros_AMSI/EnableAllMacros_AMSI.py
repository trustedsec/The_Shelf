
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
Private Const HKEY_CURRENT_USER = &H80000001
Private Const REG_DWORD = 4
Private Const KEY_READWRITE = &H2001F
Private Const ERROR_SUCCESS = 0&
""")
		self.declares.add('Private Declare PtrSafe Function m_RegCloseKey Lib "advapi32.dll" Alias "RegCloseKey" (ByVal hKey As LongPtr) As Long')
		self.declares.add('Private Declare PtrSafe Function m_RegOpenKeyEx Lib "advapi32.dll" Alias "RegOpenKeyExA" (ByVal hKey As LongPtr, ByVal lpSubKey As String, ByVal ulOptions As Long, ByVal samDesired As Long, phkResult As LongPtr) As Long')
		self.declares.add('Private Declare PtrSafe Function m_RegQueryValueEx Lib "advapi32.dll" Alias "RegQueryValueExA" (ByVal hKey As LongPtr, ByVal lpValueName As String, ByVal lpReserved As LongPtr, lpType As Long, lpData As Any, lpcbData As Long) As Long')
		self.declares.add('Private Declare PtrSafe Function m_RegSetValueEx Lib "advapi32.dll" Alias "RegSetValueExA" (ByVal hKey As LongPtr, ByVal lpValueName As String, ByVal dwReserved As Long, ByVal dwType As Long, lpData As Any, ByVal cbData As Long) As Long')

