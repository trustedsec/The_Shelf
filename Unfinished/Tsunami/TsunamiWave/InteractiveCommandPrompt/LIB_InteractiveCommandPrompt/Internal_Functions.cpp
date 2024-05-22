#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

// Kernel32
f_InteractiveCommandPrompt_CreateProcessA m_InteractiveCommandPrompt_CreateProcessA;
f_InteractiveCommandPrompt_TerminateProcess m_InteractiveCommandPrompt_TerminateProcess;
f_InteractiveCommandPrompt_CreatePipe m_InteractiveCommandPrompt_CreatePipe;
f_InteractiveCommandPrompt_DuplicateHandle m_InteractiveCommandPrompt_DuplicateHandle;
f_InteractiveCommandPrompt_PeekNamedPipe m_InteractiveCommandPrompt_PeekNamedPipe;
f_InteractiveCommandPrompt_GetExitCodeProcess m_InteractiveCommandPrompt_GetExitCodeProcess;
f_InteractiveCommandPrompt_OpenProcessToken m_InteractiveCommandPrompt_OpenProcessToken;
f_InteractiveCommandPrompt_AdjustTokenPrivileges m_InteractiveCommandPrompt_AdjustTokenPrivileges;
f_InteractiveCommandPrompt_InitializeProcThreadAttributeList m_InteractiveCommandPrompt_InitializeProcThreadAttributeList;
f_InteractiveCommandPrompt_UpdateProcThreadAttribute m_InteractiveCommandPrompt_UpdateProcThreadAttribute;
f_InteractiveCommandPrompt_DeleteProcThreadAttributeList m_InteractiveCommandPrompt_DeleteProcThreadAttributeList;
f_InteractiveCommandPrompt_Process32First m_InteractiveCommandPrompt_Process32First;
f_InteractiveCommandPrompt_Process32Next m_InteractiveCommandPrompt_Process32Next;
f_InteractiveCommandPrompt_CreateToolhelp32Snapshot m_InteractiveCommandPrompt_CreateToolhelp32Snapshot;

// Advapi32
f_InteractiveCommandPrompt_LookupPrivilegeValueA m_InteractiveCommandPrompt_LookupPrivilegeValueA;


DWORD Initialize_InteractiveCommandPrompt_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_InteractiveCommandPrompt_hKernel32)
			{
				g_InteractiveCommandPrompt_hKernel32 = LoadLibraryA(
					SZ_InteractiveCommandPrompt_Kernel32
				);
				if (NULL == g_InteractiveCommandPrompt_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_InteractiveCommandPrompt_hKernel32:  %p"), g_InteractiveCommandPrompt_hKernel32);


				m_InteractiveCommandPrompt_CreateProcessA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_CreateProcessA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_CreateProcessA, fpFunctionPointer);
				m_InteractiveCommandPrompt_CreateProcessA = (f_InteractiveCommandPrompt_CreateProcessA)fpFunctionPointer;


				m_InteractiveCommandPrompt_TerminateProcess = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_TerminateProcess
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_TerminateProcess, fpFunctionPointer);
				m_InteractiveCommandPrompt_TerminateProcess = (f_InteractiveCommandPrompt_TerminateProcess)fpFunctionPointer;


				m_InteractiveCommandPrompt_CreatePipe = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_CreatePipe
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_CreatePipe, fpFunctionPointer);
				m_InteractiveCommandPrompt_CreatePipe = (f_InteractiveCommandPrompt_CreatePipe)fpFunctionPointer;


				m_InteractiveCommandPrompt_DuplicateHandle = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_DuplicateHandle
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_DuplicateHandle, fpFunctionPointer);
				m_InteractiveCommandPrompt_DuplicateHandle = (f_InteractiveCommandPrompt_DuplicateHandle)fpFunctionPointer;


				m_InteractiveCommandPrompt_PeekNamedPipe = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_PeekNamedPipe
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_PeekNamedPipe, fpFunctionPointer);
				m_InteractiveCommandPrompt_PeekNamedPipe = (f_InteractiveCommandPrompt_PeekNamedPipe)fpFunctionPointer;


				m_InteractiveCommandPrompt_GetExitCodeProcess = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_GetExitCodeProcess
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_GetExitCodeProcess, fpFunctionPointer);
				m_InteractiveCommandPrompt_GetExitCodeProcess = (f_InteractiveCommandPrompt_GetExitCodeProcess)fpFunctionPointer;


				m_InteractiveCommandPrompt_OpenProcessToken = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_OpenProcessToken
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_OpenProcessToken, fpFunctionPointer);
				m_InteractiveCommandPrompt_OpenProcessToken = (f_InteractiveCommandPrompt_OpenProcessToken)fpFunctionPointer;


				m_InteractiveCommandPrompt_InitializeProcThreadAttributeList = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_InitializeProcThreadAttributeList
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_InitializeProcThreadAttributeList, fpFunctionPointer);
				m_InteractiveCommandPrompt_InitializeProcThreadAttributeList = (f_InteractiveCommandPrompt_InitializeProcThreadAttributeList)fpFunctionPointer;


				m_InteractiveCommandPrompt_UpdateProcThreadAttribute = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_UpdateProcThreadAttribute
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_UpdateProcThreadAttribute, fpFunctionPointer);
				m_InteractiveCommandPrompt_UpdateProcThreadAttribute = (f_InteractiveCommandPrompt_UpdateProcThreadAttribute)fpFunctionPointer;


				m_InteractiveCommandPrompt_DeleteProcThreadAttributeList = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_DeleteProcThreadAttributeList
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_DeleteProcThreadAttributeList, fpFunctionPointer);
				m_InteractiveCommandPrompt_DeleteProcThreadAttributeList = (f_InteractiveCommandPrompt_DeleteProcThreadAttributeList)fpFunctionPointer;


				m_InteractiveCommandPrompt_CreateToolhelp32Snapshot = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_CreateToolhelp32Snapshot
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_CreateToolhelp32Snapshot, fpFunctionPointer);
				m_InteractiveCommandPrompt_CreateToolhelp32Snapshot = (f_InteractiveCommandPrompt_CreateToolhelp32Snapshot)fpFunctionPointer;


				m_InteractiveCommandPrompt_Process32First = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_Process32First
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_Process32First, fpFunctionPointer);
				m_InteractiveCommandPrompt_Process32First = (f_InteractiveCommandPrompt_Process32First)fpFunctionPointer;


				m_InteractiveCommandPrompt_Process32Next = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hKernel32,
					SZ_InteractiveCommandPrompt_Process32Next
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_Process32Next, fpFunctionPointer);
				m_InteractiveCommandPrompt_Process32Next = (f_InteractiveCommandPrompt_Process32Next)fpFunctionPointer;



			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD Finalize_InteractiveCommandPrompt_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_InteractiveCommandPrompt_hKernel32)
			{
				FreeLibrary(g_InteractiveCommandPrompt_hKernel32);
				g_InteractiveCommandPrompt_hKernel32 = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}




DWORD Initialize_InteractiveCommandPrompt_Advapi32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_InteractiveCommandPrompt_hAdvapi32)
			{
				g_InteractiveCommandPrompt_hAdvapi32 = LoadLibraryA(
					SZ_InteractiveCommandPrompt_Advapi32
				);
				if (NULL == g_InteractiveCommandPrompt_hAdvapi32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_InteractiveCommandPrompt_hAdvapi32:  %p"), g_InteractiveCommandPrompt_hAdvapi32);

				m_InteractiveCommandPrompt_LookupPrivilegeValueA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hAdvapi32,
					SZ_InteractiveCommandPrompt_LookupPrivilegeValueA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_LookupPrivilegeValueA, fpFunctionPointer);
				m_InteractiveCommandPrompt_LookupPrivilegeValueA = (f_InteractiveCommandPrompt_LookupPrivilegeValueA)fpFunctionPointer;

				m_InteractiveCommandPrompt_AdjustTokenPrivileges = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InteractiveCommandPrompt_hAdvapi32,
					SZ_InteractiveCommandPrompt_AdjustTokenPrivileges
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InteractiveCommandPrompt_AdjustTokenPrivileges, fpFunctionPointer);
				m_InteractiveCommandPrompt_AdjustTokenPrivileges = (f_InteractiveCommandPrompt_AdjustTokenPrivileges)fpFunctionPointer;

			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD Finalize_InteractiveCommandPrompt_Advapi32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_InteractiveCommandPrompt_hAdvapi32)
			{
				FreeLibrary(g_InteractiveCommandPrompt_hAdvapi32);
				g_InteractiveCommandPrompt_hAdvapi32 = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}