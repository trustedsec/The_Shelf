#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

f_ExecuteCommand_CreateProcessA m_ExecuteCommand_CreateProcessA = NULL;

DWORD Initialize_ExecuteCommand_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_ExecuteCommand_hKernel32)
			{
				g_ExecuteCommand_hKernel32 = LoadLibraryA(
					SZ_ExecuteCommand_Kernel32
				);
				if (NULL == g_ExecuteCommand_hKernel32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_ExecuteCommand_hKernel32:  %p"), g_ExecuteCommand_hKernel32);

				fpFunctionPointer = GetProcAddress(
					g_ExecuteCommand_hKernel32,
					SZ_ExecuteCommand_CreateProcessA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_ExecuteCommand_CreateProcessA, fpFunctionPointer);
				m_ExecuteCommand_CreateProcessA = (f_ExecuteCommand_CreateProcessA)fpFunctionPointer;
				

				

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

DWORD Finalize_ExecuteCommand_Kernel32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_ExecuteCommand_hKernel32)
			{
				FreeLibrary(g_ExecuteCommand_hKernel32);
				g_ExecuteCommand_hKernel32 = NULL;
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



