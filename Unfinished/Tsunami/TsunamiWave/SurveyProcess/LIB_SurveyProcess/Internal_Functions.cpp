#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

// extern'ed global handle
extern HMODULE g_SurveyProcess_hKernel32 = NULL;
// extern'ed global function pointers
f_SurveyProcess_OpenProcess m_SurveyProcess_OpenProcess = NULL;
f_SurveyProcess_CreateToolhelp32Snapshot m_SurveyProcess_CreateToolhelp32Snapshot = NULL;
f_SurveyProcess_Process32First m_SurveyProcess_Process32First = NULL;
f_SurveyProcess_Process32Next m_SurveyProcess_Process32Next = NULL;
f_SurveyProcess_Module32First m_SurveyProcess_Module32First = NULL;
f_SurveyProcess_Module32Next m_SurveyProcess_Module32Next = NULL;


// extern'ed global handle
extern HMODULE g_SurveyProcess_hntdll = NULL;
// extern'ed global function pointers
f_SurveyProcess_NtQueryInformationProcess m_SurveyProcess_NtQueryInformationProcess = NULL;



DWORD Initialize_SurveyProcess_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyProcess_hKernel32)
			{
				g_SurveyProcess_hKernel32 = LoadLibraryA(
					SZ_SurveyProcess_Kernel32
				);
				if (NULL == g_SurveyProcess_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyProcess_hKernel32:  %p"), g_SurveyProcess_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_OpenProcess
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_OpenProcess, fpFunctionPointer);
				m_SurveyProcess_OpenProcess = (f_SurveyProcess_OpenProcess)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_CreateToolhelp32Snapshot
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_CreateToolhelp32Snapshot, fpFunctionPointer);
				m_SurveyProcess_CreateToolhelp32Snapshot = (f_SurveyProcess_CreateToolhelp32Snapshot)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_Process32First
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_Process32First, fpFunctionPointer);
				m_SurveyProcess_Process32First = (f_SurveyProcess_Process32First)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_Process32Next
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_Process32Next, fpFunctionPointer);
				m_SurveyProcess_Process32Next = (f_SurveyProcess_Process32Next)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_Module32First
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_Module32First, fpFunctionPointer);
				m_SurveyProcess_Module32First = (f_SurveyProcess_Module32First)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hKernel32,
					SZ_SurveyProcess_Module32Next
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_Module32Next, fpFunctionPointer);
				m_SurveyProcess_Module32Next = (f_SurveyProcess_Module32Next)fpFunctionPointer;
				

				

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

DWORD Finalize_SurveyProcess_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyProcess_hKernel32)
			{
				FreeLibrary(g_SurveyProcess_hKernel32);
				g_SurveyProcess_hKernel32 = NULL;
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



DWORD Initialize_SurveyProcess_Ntdll_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyProcess_hntdll)
			{
				g_SurveyProcess_hntdll = LoadLibraryA(
					SZ_SurveyProcess_ntdll
				);
				if (NULL == g_SurveyProcess_hntdll)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyProcess_hntdll:  %p"), g_SurveyProcess_hntdll);

				fpFunctionPointer = GetProcAddress(
					g_SurveyProcess_hntdll,
					SZ_SurveyProcess_NtQueryInformationProcess
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyProcess_NtQueryInformationProcess, fpFunctionPointer);
				m_SurveyProcess_NtQueryInformationProcess = (f_SurveyProcess_NtQueryInformationProcess)fpFunctionPointer;

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

DWORD Finalize_SurveyProcess_Ntdll_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyProcess_hntdll)
			{
				FreeLibrary(g_SurveyProcess_hntdll);
				g_SurveyProcess_hntdll = NULL;
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