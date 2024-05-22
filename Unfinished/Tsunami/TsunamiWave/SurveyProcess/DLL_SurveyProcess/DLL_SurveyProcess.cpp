#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "DLL_SurveyProcess.h"
#include "LIB_SurveyProcess.h"

HINSTANCE g_SurveyProcess_hModule = NULL;


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

			g_SurveyProcess_hModule = hinstDLL;

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


DWORD WINAPI Initialization(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	return SurveyProcessInitialization(lpInputBuffer, dwInputBufferSize, lppOutputBuffer, lpdwOutputBufferSize);
}

DWORD WINAPI Finalization(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	return SurveyProcessFinalization(lpInputBuffer, dwInputBufferSize, lppOutputBuffer, lpdwOutputBufferSize);
}

DWORD WINAPI Callback1(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	return SurveyProcessCallbackInterface(lpInputBuffer, dwInputBufferSize, lppOutputBuffer, lpdwOutputBufferSize);
}
