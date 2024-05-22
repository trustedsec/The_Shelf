#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_SurveyProcess.h"
#include "LIB_SurveyProcess.h"

// Primary TsunamiWave Configuration
CHAR g_SurveyProcess_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_SurveyProcess_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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


	cJSON*	lpJsonSurveyProcessRequestEntries = NULL;
	cJSON*	lpJsonSurveyProcessRequestEntry = NULL;

	CHAR*	szSurveyProcessInput = NULL;
	DWORD	dwSurveyProcessInputSize = 0;
	CHAR*	szSurveyProcessOutput = NULL;
	DWORD	dwSurveyProcessOutputSize = 0;

	cJSON*	lpJsonSurveyProcessOutput = NULL;
	//cJSON*	lpJsonSurveyProcessResponse = NULL;
	cJSON*	lpJsonSurveyProcessResponseResults = NULL;
	cJSON*	lpJsonSurveyProcessResponseResult = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwIndex = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyProcess wmain() start"));

			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_SurveyProcess_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_SurveyProcess_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_SurveyProcess_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_SurveyProcess_szPackedConfigurationPassword);

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

			// Get the SurveyProcess request entries
			lpJsonSurveyProcessRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, SurveyProcess_Request_Entries);
			if (NULL == lpJsonSurveyProcessRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyProcessRequestEntries = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					SurveyProcess_Request_Entries
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyProcessRequestEntries))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyProcessRequestEntries
				);
				__leave;
			}
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyProcessRequestEntry, lpJsonSurveyProcessRequestEntries)
			{
				if (
					!cJSON_IsString(lpJsonSurveyProcessRequestEntry) ||
					(NULL == lpJsonSurveyProcessRequestEntry->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyProcessRequestEntry
					);
					continue;
				}
				DBGPRINT(DEBUG_INFO, TEXT("%S[%d]: %S"), SurveyProcess_Request_Entries, dwIndex, lpJsonSurveyProcessRequestEntry->valuestring);
				dwIndex++;
			}
			

			/***************************************************************************
				Initialize the SurveyProcess plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyProcess plugin"));
			dwErrorCode = SurveyProcessInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyProcessInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the SurveyProcessCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the SurveyProcessCallbackInterface"));

			szSurveyProcessInput = szConfigurationJson;
			dwSurveyProcessInputSize = dwConfigurationJsonSize;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyProcessInputSize: %d"), dwSurveyProcessInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyProcessInput:     %p"), szSurveyProcessInput);

			// Call the SurveyProcessCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = SurveyProcessCallbackInterface(
				(LPBYTE)szSurveyProcessInput,
				dwSurveyProcessInputSize,
				(LPBYTE*)&szSurveyProcessOutput,
				&dwSurveyProcessOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyProcessCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szSurveyProcessInput,
					dwSurveyProcessInputSize,
					(LPBYTE*)&szSurveyProcessOutput,
					&dwSurveyProcessOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyProcessOutputSize: %d"), dwSurveyProcessOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyProcessOutput:     %p"), szSurveyProcessOutput);
			//printf("szSurveyProcessOutput:\n%s\n", szSurveyProcessOutput);

			// Parse the output string into a JSON object
			lpJsonSurveyProcessOutput = cJSON_Parse(szSurveyProcessOutput);
			if (NULL == lpJsonSurveyProcessOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyProcessOutput = cJSON_Parse(%p) failed."),
					szSurveyProcessOutput
				);
				__leave;
			}

			//// Get the SurveyProcess_Response JSON object
			//lpJsonSurveyProcessResponse = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyProcessOutput, SurveyProcess_Response);
			//if (NULL == lpJsonSurveyProcessResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyProcessResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyProcessOutput, SurveyProcess_Response
			//	);
			//	__leave;
			//}

			// Get the response results array
			lpJsonSurveyProcessResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyProcessOutput, SurveyProcess_Response_Results);
			if (NULL == lpJsonSurveyProcessResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyProcessResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyProcessOutput,
					SurveyProcess_Response_Results
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyProcessResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyProcessResponseResults
				);
				__leave;
			}
			// Loop through all the output results objects and display them
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyProcessResponseResult, lpJsonSurveyProcessResponseResults)
			{
				cJSON* lpJsonSurveyProcessResponseResultExeFile = NULL;
				cJSON* lpJsonSurveyProcessResponseResultProcessId = NULL;
				cJSON* lpJsonSurveyProcessResponseResultParentProcessId = NULL;

				// Get and display the value name
				lpJsonSurveyProcessResponseResultExeFile = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ExeFile);
				if (NULL == lpJsonSurveyProcessResponseResultExeFile)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonSurveyProcessResponseResultExeFile = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyProcessResponseResult,
						SurveyProcess_Response_ExeFile
					);
					continue;
				}
				if (
					!cJSON_IsString(lpJsonSurveyProcessResponseResultExeFile) ||
					(NULL == lpJsonSurveyProcessResponseResultExeFile->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyProcessResponseResultExeFile
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), SurveyProcess_Response_ExeFile, lpJsonSurveyProcessResponseResultExeFile->valuestring);

				// Get and display the SurveyProcess_Response_ProcessId
				lpJsonSurveyProcessResponseResultProcessId = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ProcessId);
				if (NULL == lpJsonSurveyProcessResponseResultProcessId)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyProcessResponseResult,
						SurveyProcess_Response_ProcessId
					);
					continue;
				}
				if (!cJSON_IsNumber(lpJsonSurveyProcessResponseResultProcessId))
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonSurveyProcessResponseResultProcessId
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %d"), SurveyProcess_Response_ProcessId, lpJsonSurveyProcessResponseResultProcessId->valueint);

				// Get and display the SurveyProcess_Response_ParentProcessId
				lpJsonSurveyProcessResponseResultParentProcessId = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ParentProcessId);
				if (NULL == lpJsonSurveyProcessResponseResultParentProcessId)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyProcessResponseResult,
						SurveyProcess_Response_ParentProcessId
					);
					continue;
				}
				if (!cJSON_IsNumber(lpJsonSurveyProcessResponseResultParentProcessId))
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonSurveyProcessResponseResultParentProcessId
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %d"), SurveyProcess_Response_ParentProcessId, lpJsonSurveyProcessResponseResultParentProcessId->valueint);
				
				dwIndex++;

			} // end cJSON_ArrayForEach(lpJsonSurveyProcessResponseResult, lpJsonSurveyProcessResponseResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyProcessOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonSurveyProcessOutput);
				cJSON_Delete(lpJsonSurveyProcessOutput);
				lpJsonSurveyProcessOutput = NULL;
			}
			// Free configuration string
			if (NULL != szSurveyProcessInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szSurveyProcessInput);
				FREE(szSurveyProcessInput);
				szSurveyProcessInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szSurveyProcessOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szSurveyProcessOutput);
				cJSON_free(szSurveyProcessOutput);
				szSurveyProcessOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the SurveyProcess plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyProcess plugin"));
			dwTempErrorCode = SurveyProcessFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyProcessFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyProcess wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}