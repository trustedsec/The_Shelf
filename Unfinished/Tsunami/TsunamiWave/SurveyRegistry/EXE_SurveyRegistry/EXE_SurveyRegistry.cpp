#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_SurveyRegistry.h"
#include "LIB_SurveyRegistry.h"

// Primary TsunamiWave Configuration
CHAR g_SurveyRegistry_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_SurveyRegistry_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpPackedConfigurationBuffer = NULL;
	DWORD	dwPackedConfigurationBufferSize = 0;
	LPBYTE	lpPackedConfigurationPasswordBuffer = NULL;
	DWORD	dwPackedConfigurationPasswordBufferSize = 0;
	CHAR*	szConfigurationJson = NULL;
	DWORD	dwConfigurationJsonSize = 0;

	
	cJSON*	lpJsonSurveyRegistryRequestEntries = NULL;
	cJSON*	lpJsonSurveyRegistryRequestEntry = NULL;

	CHAR*	szSurveyRegistryInput = NULL;
	DWORD	dwSurveyRegistryInputSize = 0;
	CHAR*	szSurveyRegistryOutput = NULL;
	DWORD	dwSurveyRegistryOutputSize = 0;

	cJSON*	lpJsonSurveyRegistryOutput = NULL;
	//cJSON*	lpJsonSurveyRegistryResponse = NULL;
	cJSON*	lpJsonSurveyRegistryResponseResults = NULL;
	cJSON*	lpJsonSurveyRegistryResponseResult = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwIndex = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyRegistry wmain() start"));

			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_SurveyRegistry_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_SurveyRegistry_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_SurveyRegistry_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_SurveyRegistry_szPackedConfigurationPassword);

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

			// Get the SurveyRegistry request entries
			lpJsonSurveyRegistryRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, SurveyRegistry_Request_Entries);
			if (NULL == lpJsonSurveyRegistryRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyRegistryRequestEntries = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					SurveyRegistry_Request_Entries
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyRegistryRequestEntries))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyRegistryRequestEntries
				);
				__leave;
			}
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyRegistryRequestEntry, lpJsonSurveyRegistryRequestEntries)
			{
				if (
					!cJSON_IsString(lpJsonSurveyRegistryRequestEntry) ||
					(NULL == lpJsonSurveyRegistryRequestEntry->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyRegistryRequestEntry
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]: %S"), SurveyRegistry_Request_Entries, dwIndex, lpJsonSurveyRegistryRequestEntry->valuestring);
				dwIndex++;
			}
			
			
			/***************************************************************************
				Initialize the SurveyRegistry plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyRegistry plugin"));
			dwErrorCode = SurveyRegistryInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyRegistryInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the SurveyRegistryCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the SurveyRegistryCallbackInterface"));

			szSurveyRegistryInput = szConfigurationJson;
			dwSurveyRegistryInputSize = dwConfigurationJsonSize;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyRegistryInputSize: %d"), dwSurveyRegistryInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyRegistryInput:     %p"), szSurveyRegistryInput);

			// Call the SurveyRegistryCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = SurveyRegistryCallbackInterface(
				(LPBYTE)szSurveyRegistryInput,
				dwSurveyRegistryInputSize,
				(LPBYTE*)&szSurveyRegistryOutput,
				&dwSurveyRegistryOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyRegistryCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szSurveyRegistryInput,
					dwSurveyRegistryInputSize,
					(LPBYTE*)&szSurveyRegistryOutput,
					&dwSurveyRegistryOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyRegistryOutputSize: %d"), dwSurveyRegistryOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyRegistryOutput:     %p"), szSurveyRegistryOutput);
			//printf("szSurveyRegistryOutput:\n%s\n", szSurveyRegistryOutput);
			
			// Parse the output string into a JSON object
			lpJsonSurveyRegistryOutput = cJSON_Parse(szSurveyRegistryOutput);
			if (NULL == lpJsonSurveyRegistryOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyRegistryOutput = cJSON_Parse(%p) failed."),
					szSurveyRegistryOutput
				);
				__leave;
			}

			//// Get the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			//lpJsonSurveyRegistryResponse = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyRegistryOutput, SurveyRegistry_Response);
			//if (NULL == lpJsonSurveyRegistryResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyRegistryResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyRegistryOutput, SurveyRegistry_Response
			//	);
			//	__leave;
			//}

			// Get the response results array
			lpJsonSurveyRegistryResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyRegistryOutput, SurveyRegistry_Response_Results);
			if (NULL == lpJsonSurveyRegistryResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyRegistryResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyRegistryOutput,
					SurveyRegistry_Response_Results
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyRegistryResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyRegistryResponseResults
				);
				__leave;
			}
			// Loop through all the output results objects and display them
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyRegistryResponseResult, lpJsonSurveyRegistryResponseResults)
			{
				cJSON* lpJsonValue = NULL;
				cJSON* lpJsonData = NULL;
				int nBufferSize = 0;
				unsigned char* lpBuffer = NULL;

				DBGPRINT(DEBUG_INFO, TEXT("%S[%d]:"), SurveyRegistry_Response_Results, dwIndex);

				// Get and display the SurveyRegistry_Response_Results_Value
				lpJsonValue = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyRegistryResponseResult, SurveyRegistry_Response_Results_Value);
				if (NULL == lpJsonValue)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyRegistryResponseResult,
						SurveyRegistry_Response_Results_Value
					);
					continue;
				}
				if (
					!cJSON_IsString(lpJsonValue) ||
					(NULL == lpJsonValue->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonValue
					);
					continue;
				}
				DBGPRINT(DEBUG_INFO, TEXT("  ->%S: %S"), SurveyRegistry_Response_Results_Value, lpJsonValue->valuestring);

				// Get and display the SurveyRegistry_Response_Results_Data
				lpJsonData = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyRegistryResponseResult, SurveyRegistry_Response_Results_Data);
				if (NULL == lpJsonData)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonData = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyRegistryResponseResult,
						SurveyRegistry_Response_Results_Data
					);
					continue;
				}
				if (
					!cJSON_IsBytes(lpJsonData) ||
					(NULL == lpJsonData->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsBytes(%p) failed."),
						lpJsonData
					);
					continue;
				}
				
				// Convert the data into the original bytes
				lpBuffer = cJSON_GetBytesValue(lpJsonData, &nBufferSize);
				if (NULL == lpBuffer)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_GetBytesValue(%p, %p) failed."),
						lpJsonData, 
						&nBufferSize
					);
					continue;
				}
				DBGPRINT(DEBUG_INFO, TEXT("  ->%S: %S"), SurveyRegistry_Response_Results_Data, (char*)lpBuffer );
				//DBGPRINT(DEBUG_VERBOSE, TEXT("nBufferSize:      %d"), nBufferSize);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpBuffer:         %p"), lpBuffer);
				//if (NULL != lpBuffer)
				//{
				//	for (int i = 0; i < nBufferSize; i++)
				//	{
				//		DBGPRINT(DEBUG_VERBOSE, TEXT("%.2x "), lpBuffer[i]);
				//	}
				//}
				cJSON_free(lpBuffer);
				lpBuffer = NULL;
				
				dwIndex++;

			} // end cJSON_ArrayForEach(lpJsonSurveyRegistryResponseResult, lpJsonSurveyRegistryResponseResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyRegistryOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonSurveyRegistryOutput);
				cJSON_Delete(lpJsonSurveyRegistryOutput);
				lpJsonSurveyRegistryOutput = NULL;
			}
			// Free configuration string
			if (NULL != szSurveyRegistryInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szSurveyRegistryInput);
				FREE(szSurveyRegistryInput);
				szSurveyRegistryInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szSurveyRegistryOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szSurveyRegistryOutput);
				cJSON_free(szSurveyRegistryOutput);
				szSurveyRegistryOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the SurveyRegistry plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyRegistry plugin"));
			dwTempErrorCode = SurveyRegistryFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("SurveyRegistryFinalization( %p, %d, %p, %p ) failed. (%08x)"), 
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyRegistry wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
