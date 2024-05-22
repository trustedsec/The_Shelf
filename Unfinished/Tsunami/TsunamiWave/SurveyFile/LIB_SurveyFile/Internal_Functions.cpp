#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

// extern'ed global handle
HMODULE				g_SurveyFile_hKernel32 = NULL;
// extern'ed global function pointers
f_SurveyFile_GetFullPathNameA	m_SurveyFile_GetFullPathNameA = NULL;
f_SurveyFile_FindFirstFileA		m_SurveyFile_FindFirstFileA = NULL;
f_SurveyFile_FindNextFileA		m_SurveyFile_FindNextFileA = NULL;
f_SurveyFile_FindClose			m_SurveyFile_FindClose = NULL;

DWORD Initialize_SurveyFile_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyFile_hKernel32)
			{
				g_SurveyFile_hKernel32 = LoadLibraryA(
					SZ_SurveyFile_Kernel32
				);
				if (NULL == g_SurveyFile_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyFile_hKernel32:  %p"), g_SurveyFile_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_SurveyFile_hKernel32,
					SZ_SurveyFile_GetFullPathNameA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyFile_GetFullPathNameA, fpFunctionPointer);
				m_SurveyFile_GetFullPathNameA = (f_SurveyFile_GetFullPathNameA)fpFunctionPointer;
			
				fpFunctionPointer = GetProcAddress(
					g_SurveyFile_hKernel32,
					SZ_SurveyFile_FindFirstFileA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyFile_FindFirstFileA, fpFunctionPointer);
				m_SurveyFile_FindFirstFileA = (f_SurveyFile_FindFirstFileA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyFile_hKernel32,
					SZ_SurveyFile_FindNextFileA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyFile_FindNextFileA, fpFunctionPointer);
				m_SurveyFile_FindNextFileA = (f_SurveyFile_FindNextFileA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyFile_hKernel32,
					SZ_SurveyFile_FindClose
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyFile_FindClose, fpFunctionPointer);
				m_SurveyFile_FindClose = (f_SurveyFile_FindClose)fpFunctionPointer;
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

DWORD Finalize_SurveyFile_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyFile_hKernel32)
			{
				FreeLibrary(g_SurveyFile_hKernel32);
				g_SurveyFile_hKernel32 = NULL;
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



