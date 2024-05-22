#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"


f_TriggerSleep_Sleep m_TriggerSleep_Sleep = NULL;
f_TriggerSleep_GetSystemTime m_TriggerSleep_GetSystemTime = NULL;

DWORD Initialize_TriggerSleep_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_TriggerSleep_hKernel32)
			{
				g_TriggerSleep_hKernel32 = LoadLibraryA(
					SZ_TriggerSleep_Kernel32
				);
				if (NULL == g_TriggerSleep_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_TriggerSleep_hKernel32:  %p"), g_TriggerSleep_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_TriggerSleep_hKernel32,
					SZ_TriggerSleep_Sleep
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_TriggerSleep_Sleep, fpFunctionPointer);
				m_TriggerSleep_Sleep = (f_TriggerSleep_Sleep)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_TriggerSleep_hKernel32,
					SZ_TriggerSleep_GetSystemTime
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_TriggerSleep_GetSystemTime, fpFunctionPointer);
				m_TriggerSleep_GetSystemTime = (f_TriggerSleep_GetSystemTime)fpFunctionPointer;

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

DWORD Finalize_TriggerSleep_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_TriggerSleep_hKernel32)
			{
				FreeLibrary(g_TriggerSleep_hKernel32);
				g_TriggerSleep_hKernel32 = NULL;
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




