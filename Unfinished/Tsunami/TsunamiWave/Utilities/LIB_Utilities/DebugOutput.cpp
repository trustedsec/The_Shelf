#include "DebugOutput.h"

#define DEBUG_LEVEL_EMPTY L"12345678"
#define DEBUG_ERROR_STRING L"*ERROR*"
#define DEBUG_WARNING_STRING L"WARNING"
#define DEBUG_INFO_STRING L"INFO"
#define DEBUG_VERBOSE_STRING L"VERBOSE"

#define STR_DBGPRINT_DLL_FORMAT L"%8s [%s:%d] %s"
#define DBEBUG_DLL_PRINT_FUNCTION OutputDebugStringW

#define STR_DBGPRINT_EXE_FORMAT L"%8s [%s:%d] %s\n"
#define DBEBUG_EXE_PRINT_FUNCTION wprintf

VOID _DBGPRINT_DLL(LPCWSTR lpcwszFunction, INT nLineNumber, INT nDebugLevel, LPCWSTR lpcwszDebugFormatString, ...)
{
	INT nDebugStringLength = 0;
	INT nDebugBodyStringLength = 0;
	INT nReturnError = 0;
	va_list args;
	PWCHAR wszDebugStringBody = NULL;
	PWCHAR wszDebugString = NULL;
	DWORD dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			// Begin variable arguments use
			va_start(args, lpcwszDebugFormatString);

			// Get the length of the debug message body
			nDebugBodyStringLength = _vscwprintf(lpcwszDebugFormatString, args) + 1;
			if (-1 == nDebugBodyStringLength)
			{
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _vscwprintf failed.");
				__leave;
			}

			// Allocate memory for the debug message body
			wszDebugStringBody = (PWCHAR)VirtualAlloc(NULL, nDebugBodyStringLength * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (NULL == wszDebugStringBody)
			{
				dwErrorCode = GetLastError();
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualAlloc failed. ");
				__leave;
			}

			// Fill in the debug message body
			nReturnError = vswprintf_s(wszDebugStringBody, (nDebugBodyStringLength * sizeof(WCHAR)), lpcwszDebugFormatString, args);
			if (-1 == nReturnError)
			{
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] vswprintf_s failed.");
				__leave;
			}

			// Get the length of the debug message (header and body)
			nDebugStringLength = _scwprintf(STR_DBGPRINT_DLL_FORMAT, DEBUG_LEVEL_EMPTY, lpcwszFunction, nLineNumber, wszDebugStringBody) + 1;
			if (-1 == nDebugStringLength)
			{
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _scwprintf failed.");
				__leave;
			}

			// Allocate memory for the debug message (header and body)
			wszDebugString = (PWCHAR)VirtualAlloc(NULL, nDebugStringLength * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (NULL == wszDebugStringBody)
			{
				dwErrorCode = GetLastError();
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualAlloc failed. ");
				__leave;
			}

			// Fill in the debug message string to include the header and body
			switch (nDebugLevel)
			{
				case DEBUG_ERROR:
				{
					nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_DLL_FORMAT, DEBUG_ERROR_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
					break;
				}
				case DEBUG_WARNING:
				{
					nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_DLL_FORMAT, DEBUG_WARNING_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
					break;
				}
				case DEBUG_INFO:
				{
					nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_DLL_FORMAT, DEBUG_INFO_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
					break;
				}
				case DEBUG_VERBOSE:
				{
					nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_DLL_FORMAT, DEBUG_VERBOSE_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
					break;
				}
				default:
				{
					DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] Unknown debug level.");
					__leave;
				}
			}

			if (-1 == nReturnError)
			{
				DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _snwprintf failed.");
				__leave;
			}

			// Print out the debug message
			DBEBUG_DLL_PRINT_FUNCTION(wszDebugString);
		}
		__finally
		{
			// Free the filled in debug message body
			if (NULL != wszDebugString)
			{
				if (FALSE == VirtualFree(wszDebugString, 0, MEM_RELEASE))
				{
					dwErrorCode = GetLastError();
					DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualFree failed.");
				}
			}

			// Free the filled in debug message
			if (NULL != wszDebugStringBody)
			{
				if (FALSE == VirtualFree(wszDebugStringBody, 0, MEM_RELEASE))
				{
					dwErrorCode = GetLastError();
					DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualFree failed.");
				}
			}

			// End variable arguments use
			va_end(args);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DBEBUG_DLL_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] Exception occurred.");
	}
}

VOID _DBGPRINT_EXE(LPCWSTR lpcwszFunction, INT nLineNumber, INT nDebugLevel, LPCWSTR lpcwszDebugFormatString, ...)
{
	INT nDebugStringLength = 0;
	INT nDebugBodyStringLength = 0;
	INT nReturnError = 0;
	va_list args;
	PWCHAR wszDebugStringBody = NULL;
	PWCHAR wszDebugString = NULL;
	DWORD dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			// Begin variable arguments use
			va_start(args, lpcwszDebugFormatString);

			// Get the length of the debug message body
			nDebugBodyStringLength = _vscwprintf(lpcwszDebugFormatString, args) + 1;
			if (-1 == nDebugBodyStringLength)
			{
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _vscwprintf failed.");
				__leave;
			}

			// Allocate memory for the debug message body
			wszDebugStringBody = (PWCHAR)VirtualAlloc(NULL, nDebugBodyStringLength * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (NULL == wszDebugStringBody)
			{
				dwErrorCode = GetLastError();
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualAlloc failed. ");
				__leave;
			}

			// Fill in the debug message body
			nReturnError = vswprintf_s(wszDebugStringBody, (nDebugBodyStringLength * sizeof(WCHAR)), lpcwszDebugFormatString, args);
			if (-1 == nReturnError)
			{
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] vswprintf_s failed.");
				__leave;
			}

			// Get the length of the debug message (header and body)
			nDebugStringLength = _scwprintf(STR_DBGPRINT_EXE_FORMAT, DEBUG_LEVEL_EMPTY, lpcwszFunction, nLineNumber, wszDebugStringBody) + 1;
			if (-1 == nDebugStringLength)
			{
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _scwprintf failed.");
				__leave;
			}

			// Allocate memory for the debug message (header and body)
			wszDebugString = (PWCHAR)VirtualAlloc(NULL, nDebugStringLength * sizeof(WCHAR), MEM_COMMIT, PAGE_READWRITE);
			if (NULL == wszDebugStringBody)
			{
				dwErrorCode = GetLastError();
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualAlloc failed. ");
				__leave;
			}

			// Fill in the debug message string to include the header and body
			switch (nDebugLevel)
			{
			case DEBUG_ERROR:
			{
				nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_EXE_FORMAT, DEBUG_ERROR_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
				break;
			}
			case DEBUG_WARNING:
			{
				nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_EXE_FORMAT, DEBUG_WARNING_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
				break;
			}
			case DEBUG_INFO:
			{
				nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_EXE_FORMAT, DEBUG_INFO_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
				break;
			}
			case DEBUG_VERBOSE:
			{
				nReturnError = _snwprintf_s(wszDebugString, (nDebugStringLength * sizeof(WCHAR)), (nDebugStringLength * sizeof(WCHAR)), STR_DBGPRINT_EXE_FORMAT, DEBUG_VERBOSE_STRING, lpcwszFunction, nLineNumber, wszDebugStringBody);
				break;
			}
			default:
			{
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] Unknown debug level.");
				__leave;
			}
			}

			if (-1 == nReturnError)
			{
				DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] _snwprintf failed.");
				__leave;
			}

			// Print out the debug message
			DBEBUG_EXE_PRINT_FUNCTION(wszDebugString);
		}
		__finally
		{
			// Free the filled in debug message body
			if (NULL != wszDebugString)
			{
				if (FALSE == VirtualFree(wszDebugString, 0, MEM_RELEASE))
				{
					dwErrorCode = GetLastError();
					DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualFree failed.");
				}
			}

			// Free the filled in debug message
			if (NULL != wszDebugStringBody)
			{
				if (FALSE == VirtualFree(wszDebugStringBody, 0, MEM_RELEASE))
				{
					dwErrorCode = GetLastError();
					DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] VirtualFree failed.");
				}
			}

			// End variable arguments use
			va_end(args);
		}
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		DBEBUG_EXE_PRINT_FUNCTION(L"*ERROR*  [DBGPRINT] Exception occurred.");
	}
}
