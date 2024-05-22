#include "Internal_Functions.h"
#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"



//extern'ed handles
HMODULE g_SurveyHost_hKernel32 = NULL;
//extern'ed function pointers
f_SurveyHost_GetComputerNameExA m_SurveyHost_GetComputerNameExA = NULL;
f_SurveyHost_GetVersionExA m_SurveyHost_GetVersionExA = NULL;
f_SurveyHost_GetNativeSystemInfo m_SurveyHost_GetNativeSystemInfo = NULL;
f_SurveyHost_ExpandEnvironmentStringsA m_SurveyHost_ExpandEnvironmentStringsA = NULL;
f_SurveyHost_GetVolumeInformationA m_SurveyHost_GetVolumeInformationA = NULL;

DWORD Initialize_SurveyHost_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyHost_hKernel32)
			{
				g_SurveyHost_hKernel32 = LoadLibraryA(
					SZ_SurveyHost_Kernel32
				);
				if (NULL == g_SurveyHost_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyHost_hKernel32:  %p"), g_SurveyHost_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_SurveyHost_hKernel32,
					SZ_SurveyHost_GetComputerNameExA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyHost_GetComputerNameExA, fpFunctionPointer);
				m_SurveyHost_GetComputerNameExA = (f_SurveyHost_GetComputerNameExA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyHost_hKernel32,
					SZ_SurveyHost_GetVersionExA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyHost_GetVersionExA, fpFunctionPointer);
				m_SurveyHost_GetVersionExA = (f_SurveyHost_GetVersionExA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyHost_hKernel32,
					SZ_SurveyHost_GetNativeSystemInfo
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyHost_GetNativeSystemInfo, fpFunctionPointer);
				m_SurveyHost_GetNativeSystemInfo = (f_SurveyHost_GetNativeSystemInfo)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyHost_hKernel32,
					SZ_SurveyHost_ExpandEnvironmentStringsA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyHost_ExpandEnvironmentStringsA, fpFunctionPointer);
				m_SurveyHost_ExpandEnvironmentStringsA = (f_SurveyHost_ExpandEnvironmentStringsA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyHost_hKernel32,
					SZ_SurveyHost_GetVolumeInformationA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyHost_GetVolumeInformationA, fpFunctionPointer);
				m_SurveyHost_GetVolumeInformationA = (f_SurveyHost_GetVolumeInformationA)fpFunctionPointer;
	
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

DWORD Finalize_SurveyHost_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyHost_hKernel32)
			{
				FreeLibrary(g_SurveyHost_hKernel32);
				g_SurveyHost_hKernel32 = NULL;
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


