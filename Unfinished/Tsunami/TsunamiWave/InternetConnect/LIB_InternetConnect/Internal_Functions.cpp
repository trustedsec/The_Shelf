#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

f_InternetConnect_InternetOpenA m_InternetConnect_InternetOpenA = NULL;
f_InternetConnect_InternetCrackUrlA m_InternetConnect_InternetCrackUrlA = NULL;
f_InternetConnect_InternetConnectA m_InternetConnect_InternetConnectA = NULL;
f_InternetConnect_HttpOpenRequestA m_InternetConnect_HttpOpenRequestA = NULL;
f_InternetConnect_InternetSetOptionA m_InternetConnect_InternetSetOptionA = NULL;
f_InternetConnect_HttpSendRequestA m_InternetConnect_HttpSendRequestA = NULL;
f_InternetConnect_InternetCloseHandle m_InternetConnect_InternetCloseHandle = NULL;
f_InternetConnect_HttpQueryInfoA m_InternetConnect_HttpQueryInfoA = NULL;
f_InternetConnect_InternetReadFile m_InternetConnect_InternetReadFile = NULL;
f_InternetConnect_InternetQueryDataAvailable m_InternetConnect_InternetQueryDataAvailable = NULL;


DWORD Initialize_InternetConnect_Wininet_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_InternetConnect_hWininet)
			{
				g_InternetConnect_hWininet = LoadLibraryA(
					SZ_InternetConnect_Wininet
				);
				if (NULL == g_InternetConnect_hWininet)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_hWininet:  %p"), g_InternetConnect_hWininet);


				m_InternetConnect_InternetOpenA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetOpenA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetOpenA, fpFunctionPointer);
				m_InternetConnect_InternetOpenA = (f_InternetConnect_InternetOpenA)fpFunctionPointer;


				m_InternetConnect_InternetCrackUrlA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetCrackUrlA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetCrackUrlA, fpFunctionPointer);
				m_InternetConnect_InternetCrackUrlA = (f_InternetConnect_InternetCrackUrlA)fpFunctionPointer;


				m_InternetConnect_InternetConnectA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetConnectA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetConnectA, fpFunctionPointer);
				m_InternetConnect_InternetConnectA = (f_InternetConnect_InternetConnectA)fpFunctionPointer;


				m_InternetConnect_HttpOpenRequestA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_HttpOpenRequestA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_HttpOpenRequestA, fpFunctionPointer);
				m_InternetConnect_HttpOpenRequestA = (f_InternetConnect_HttpOpenRequestA)fpFunctionPointer;


				m_InternetConnect_InternetSetOptionA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetSetOptionA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetSetOptionA, fpFunctionPointer);
				m_InternetConnect_InternetSetOptionA = (f_InternetConnect_InternetSetOptionA)fpFunctionPointer;


				m_InternetConnect_HttpSendRequestA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_HttpSendRequestA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_HttpSendRequestA, fpFunctionPointer);
				m_InternetConnect_HttpSendRequestA = (f_InternetConnect_HttpSendRequestA)fpFunctionPointer;


				m_InternetConnect_InternetCloseHandle = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetCloseHandle
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetCloseHandle, fpFunctionPointer);
				m_InternetConnect_InternetCloseHandle = (f_InternetConnect_InternetCloseHandle)fpFunctionPointer;


				m_InternetConnect_HttpQueryInfoA = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_HttpQueryInfoA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_HttpQueryInfoA, fpFunctionPointer);
				m_InternetConnect_HttpQueryInfoA = (f_InternetConnect_HttpQueryInfoA)fpFunctionPointer;


				m_InternetConnect_InternetReadFile = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetReadFile
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetReadFile, fpFunctionPointer);
				m_InternetConnect_InternetReadFile = (f_InternetConnect_InternetReadFile)fpFunctionPointer;


				m_InternetConnect_InternetQueryDataAvailable = NULL;
				fpFunctionPointer = GetProcAddress(
					g_InternetConnect_hWininet,
					SZ_InternetConnect_InternetQueryDataAvailable
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_InternetConnect_InternetQueryDataAvailable, fpFunctionPointer);
				m_InternetConnect_InternetQueryDataAvailable = (f_InternetConnect_InternetQueryDataAvailable)fpFunctionPointer;

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

DWORD Finalize_InternetConnect_Wininet_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_InternetConnect_hWininet)
			{
				FreeLibrary(g_InternetConnect_hWininet);
				g_InternetConnect_hWininet = NULL;
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




