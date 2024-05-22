#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"

// Global function pointers
f_SurveyRegistry_RegOpenKeyExA m_SurveyRegistry_RegOpenKeyExA = NULL;
f_SurveyRegistry_RegEnumValueA m_SurveyRegistry_RegEnumValueA = NULL;
f_SurveyRegistry_RegCloseKey m_SurveyRegistry_RegCloseKey = NULL;

DWORD Initialize_SurveyRegistry_Advapi32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == g_SurveyRegistry_hAdvapi32)
			{
				g_SurveyRegistry_hAdvapi32 = LoadLibraryA(
					SZ_SurveyRegistry_Advapi32
				);
				if (NULL == g_SurveyRegistry_hAdvapi32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("g_SurveyRegistry_hAdvapi32:  %p"), g_SurveyRegistry_hAdvapi32);

				fpFunctionPointer = GetProcAddress(
					g_SurveyRegistry_hAdvapi32,
					SZ_SurveyRegistry_RegOpenKeyExA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyRegistry_RegOpenKeyExA, fpFunctionPointer);
				m_SurveyRegistry_RegOpenKeyExA = (f_SurveyRegistry_RegOpenKeyExA)fpFunctionPointer;
				
				fpFunctionPointer = GetProcAddress(
					g_SurveyRegistry_hAdvapi32,
					SZ_SurveyRegistry_RegEnumValueA
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyRegistry_RegEnumValueA, fpFunctionPointer);
				m_SurveyRegistry_RegEnumValueA = (f_SurveyRegistry_RegEnumValueA)fpFunctionPointer;

				fpFunctionPointer = GetProcAddress(
					g_SurveyRegistry_hAdvapi32,
					SZ_SurveyRegistry_RegCloseKey
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S:  %p"), SZ_SurveyRegistry_RegCloseKey, fpFunctionPointer);
				m_SurveyRegistry_RegCloseKey = (f_SurveyRegistry_RegCloseKey)fpFunctionPointer;
				

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

DWORD Finalize_SurveyRegistry_Advapi32_Functions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != g_SurveyRegistry_hAdvapi32)
			{
				FreeLibrary(g_SurveyRegistry_hAdvapi32);
				g_SurveyRegistry_hAdvapi32 = NULL;
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



