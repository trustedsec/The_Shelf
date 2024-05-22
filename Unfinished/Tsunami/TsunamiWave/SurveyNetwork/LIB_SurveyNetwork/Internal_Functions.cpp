#include "Internal_Functions.h"

#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"




//extern'ed handles
HMODULE g_SurveyNetwork_hWs2_32 = NULL;
//extern'ed function pointers
f_SurveyNetwork_WSAStartup m_SurveyNetwork_WSAStartup = NULL;
f_SurveyNetwork_WSACleanup m_SurveyNetwork_WSACleanup = NULL;
f_SurveyNetwork_gethostname m_SurveyNetwork_gethostname = NULL;

DWORD Initialize_SurveyNetwork_Ws2_32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyNetwork_hWs2_32)
			{
				g_SurveyNetwork_hWs2_32 = LoadLibraryA(
					SZ_SurveyNetwork_Ws2_32
				);
				if (NULL == g_SurveyNetwork_hWs2_32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyNetwork_hWs2_32:  %p"), g_SurveyNetwork_hWs2_32);

				fpFunctionPointer = GetProcAddress(
					g_SurveyNetwork_hWs2_32,
					SZ_SurveyNetwork_WSAStartup
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyNetwork_WSAStartup, fpFunctionPointer);
				m_SurveyNetwork_WSAStartup = (f_SurveyNetwork_WSAStartup)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyNetwork_hWs2_32,
					SZ_SurveyNetwork_WSACleanup
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyNetwork_WSACleanup, fpFunctionPointer);
				m_SurveyNetwork_WSACleanup = (f_SurveyNetwork_WSACleanup)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyNetwork_hWs2_32,
					SZ_SurveyNetwork_gethostname
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyNetwork_gethostname, fpFunctionPointer);
				m_SurveyNetwork_gethostname = (f_SurveyNetwork_gethostname)fpFunctionPointer;
				
				

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

DWORD Finalize_SurveyNetwork_Ws2_32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyNetwork_hWs2_32)
			{
				FreeLibrary(g_SurveyNetwork_hWs2_32);
				g_SurveyNetwork_hWs2_32 = NULL;
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


//extern'ed handles
HMODULE g_hIphlpapi = NULL;
//extern'ed function pointers
f_SurveyNetwork_GetAdaptersAddresses m_SurveyNetwork_GetAdaptersAddresses = NULL;

DWORD InitializeIphlpapiFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_hIphlpapi)
			{
				g_hIphlpapi = LoadLibraryA(
					SZ_SurveyNetwork_Iphlpapi
				);
				if (NULL == g_hIphlpapi)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_hIphlpapi:  %p"), g_hIphlpapi);

				fpFunctionPointer = GetProcAddress(
					g_hIphlpapi,
					SZ_SurveyNetwork_GetAdaptersAddresses
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyNetwork_GetAdaptersAddresses, fpFunctionPointer);
				m_SurveyNetwork_GetAdaptersAddresses = (f_SurveyNetwork_GetAdaptersAddresses)fpFunctionPointer;




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

DWORD FinalizeIphlpapiFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_hIphlpapi)
			{
				FreeLibrary(g_hIphlpapi);
				g_hIphlpapi = NULL;
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
