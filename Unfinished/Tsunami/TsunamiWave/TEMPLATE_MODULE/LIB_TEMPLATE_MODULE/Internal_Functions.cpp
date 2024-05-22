#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"


f_TEMPLATE_MODULE_Sleep m_TEMPLATE_MODULE_Sleep = NULL;
f_TEMPLATE_MODULE_GetSystemTime m_TEMPLATE_MODULE_GetSystemTime = NULL;

DWORD Initialize_TEMPLATE_MODULE_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_TEMPLATE_MODULE_hKernel32)
			{
				g_TEMPLATE_MODULE_hKernel32 = LoadLibraryA(
					SZ_TEMPLATE_MODULE_Kernel32
				);
				if (NULL == g_TEMPLATE_MODULE_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_TEMPLATE_MODULE_hKernel32:  %p"), g_TEMPLATE_MODULE_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_TEMPLATE_MODULE_hKernel32,
					SZ_TEMPLATE_MODULE_Sleep
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_TEMPLATE_MODULE_Sleep, fpFunctionPointer);
				m_TEMPLATE_MODULE_Sleep = (f_TEMPLATE_MODULE_Sleep)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_TEMPLATE_MODULE_hKernel32,
					SZ_TEMPLATE_MODULE_GetSystemTime
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_TEMPLATE_MODULE_GetSystemTime, fpFunctionPointer);
				m_TEMPLATE_MODULE_GetSystemTime = (f_TEMPLATE_MODULE_GetSystemTime)fpFunctionPointer;

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

DWORD Finalize_TEMPLATE_MODULE_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_TEMPLATE_MODULE_hKernel32)
			{
				FreeLibrary(g_TEMPLATE_MODULE_hKernel32);
				g_TEMPLATE_MODULE_hKernel32 = NULL;
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




