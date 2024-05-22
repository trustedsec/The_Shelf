#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "DLL_LoadLibrary.h"
#include "LIB_LoadLibrary.h"


HINSTANCE g_hModule = NULL;


BOOL WINAPI DllMain(
	HINSTANCE hinstDLL, // handle to DLL module
	DWORD fdwReason, // reason for calling function
	LPVOID lpReserved) // reserved
{
	BOOL bReturn = TRUE;
	DWORD dwErrorCode = ERROR_SUCCESS;
	

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("DllMain(%p,%d,%p)"), hinstDLL, fdwReason, lpReserved);

			g_hModule = hinstDLL;

			// Perform actions based on the reason for calling.
			switch (fdwReason)
			{
				case DLL_PROCESS_ATTACH:
				{
					DBGPRINT( DEBUG_INFO, TEXT("DLL_PROCESS_ATTACH()"));

					

					break;
				}

				case DLL_THREAD_ATTACH:
				{
					DBGPRINT(DEBUG_INFO, TEXT("DLL_THREAD_ATTACH()"));

					break;
				}

				case DLL_THREAD_DETACH:
				{
					DBGPRINT(DEBUG_INFO, TEXT("DLL_THREAD_DETACH()"));

					break;
				}

				case DLL_PROCESS_DETACH:
				{
					DBGPRINT(DEBUG_INFO, TEXT("DLL_PROCESS_DETACH()"));

					break;
				}
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except
			
	return bReturn;
}


DWORD DLL_LoadLibrary_ExportedFunction(void)
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwThreadId = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("DLL_LoadLibrary_ExportedFunction() start"));


		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("DLL_LoadLibrary_ExportedFunction() end"));
			
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
