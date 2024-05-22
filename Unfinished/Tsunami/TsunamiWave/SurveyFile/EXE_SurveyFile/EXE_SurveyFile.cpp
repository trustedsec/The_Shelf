#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_SurveyFile.h"
#include "LIB_SurveyFile.h"

// Primary TsunamiWave Configuration
CHAR g_SurveyFile_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_SurveyFile_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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


	cJSON*	lpJsonSurveyFileRequestEntries = NULL;
	cJSON*	lpJsonSurveyFileRequestEntry = NULL;

	CHAR*	szSurveyFileInput = NULL;
	DWORD	dwSurveyFileInputSize = 0;
	CHAR*	szSurveyFileOutput = NULL;
	DWORD	dwSurveyFileOutputSize = 0;

	cJSON*	lpJsonSurveyFileOutput = NULL;
	cJSON*	lpJsonSurveyFileResponse = NULL;
	cJSON*	lpJsonSurveyFileResponseResults = NULL;
	cJSON*	lpJsonSurveyFileResponseResult = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwIndex = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyFile wmain() start"));


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_SurveyFile_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_SurveyFile_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_SurveyFile_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_SurveyFile_szPackedConfigurationPassword);

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
				
			// Get the SurveyFile request entries
			lpJsonSurveyFileRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, SurveyFile_Request_Entries);
			if (NULL == lpJsonSurveyFileRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyFileRequestEntries = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					SurveyFile_Request_Entries
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyFileRequestEntries))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyFileRequestEntries
				);
				__leave;
			}
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyFileRequestEntry, lpJsonSurveyFileRequestEntries)
			{
				if (
					!cJSON_IsString(lpJsonSurveyFileRequestEntry) ||
					(NULL == lpJsonSurveyFileRequestEntry->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyFileRequestEntry
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]: %S"), SurveyFile_Request_Entries, dwIndex, lpJsonSurveyFileRequestEntry->valuestring);
				dwIndex++;
			}


			/***************************************************************************
				Initialize the SurveyFile plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyFile plugin"));
			dwErrorCode = SurveyFileInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyFileInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the SurveyFileCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the SurveyFileCallbackInterface"));

			szSurveyFileInput = szConfigurationJson;
			dwSurveyFileInputSize = dwConfigurationJsonSize;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyFileInputSize: %d"), dwSurveyFileInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyFileInput:     %p"), szSurveyFileInput);

			// Call the SurveyFileCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = SurveyFileCallbackInterface(
				(LPBYTE)szSurveyFileInput,
				dwSurveyFileInputSize,
				(LPBYTE*)&szSurveyFileOutput,
				&dwSurveyFileOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyFileCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szSurveyFileInput,
					dwSurveyFileInputSize,
					(LPBYTE*)&szSurveyFileOutput,
					&dwSurveyFileOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyFileOutputSize: %d"), dwSurveyFileOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyFileOutput:     %p"), szSurveyFileOutput);
			//printf("szSurveyFileOutput:\n%s\n", szSurveyFileOutput);

			// Parse the output string into a JSON object
			lpJsonSurveyFileOutput = cJSON_Parse(szSurveyFileOutput);
			if (NULL == lpJsonSurveyFileOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyFileOutput = cJSON_Parse(%p) failed."),
					szSurveyFileOutput
				);
				__leave;
			}

			//// Get the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			//lpJsonSurveyFileResponse = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileOutput, SurveyFile_Response);
			//if (NULL == lpJsonSurveyFileResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyFileResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyFileOutput, SurveyFile_Response
			//	);
			//	__leave;
			//}

			// Get the response results array
			lpJsonSurveyFileResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileOutput, SurveyFile_Response_Results);
			if (NULL == lpJsonSurveyFileResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyFileResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyFileOutput,
					SurveyFile_Response_Results
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyFileResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyFileResponseResults
				);
				__leave;
			}
			// Loop through all the output results objects and display them
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyFileResponseResult, lpJsonSurveyFileResponseResults)
			{
				cJSON* lpJsonDirectoryName = NULL;
				cJSON* lpJsonFileName = NULL;
				cJSON* lpJsonFileSizeHigh = NULL;
				cJSON* lpJsonFileSizeLow = NULL;

				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]:"), SurveyFile_Response_Results, dwIndex);

				// Get and display the SurveyFile_Response_DirectoryName
				lpJsonDirectoryName = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileResponseResult, SurveyFile_Response_DirectoryName);
				if (NULL == lpJsonDirectoryName)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyFileResponseResult,
						SurveyFile_Response_DirectoryName
					);
					continue;
				}
				if (
					!cJSON_IsString(lpJsonDirectoryName) ||
					(NULL == lpJsonDirectoryName->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonDirectoryName
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), SurveyFile_Response_DirectoryName, lpJsonDirectoryName->valuestring);

				// Get and display the SurveyFile_Response_FileName
				lpJsonFileName = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileResponseResult, SurveyFile_Response_FileName);
				if (NULL == lpJsonFileName)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyFileResponseResult,
						SurveyFile_Response_FileName
					);
					continue;
				}
				if (
					!cJSON_IsString(lpJsonFileName) ||
					(NULL == lpJsonFileName->valuestring)
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonFileName
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), SurveyFile_Response_FileName, lpJsonFileName->valuestring);

				// Get and display the SurveyFile_Response_FileSizeLow
				lpJsonFileSizeLow = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileResponseResult, SurveyFile_Response_FileSizeLow);
				if (NULL == lpJsonFileSizeLow)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyFileResponseResult,
						SurveyFile_Response_FileSizeLow
					);
					continue;
				}
				if ( !cJSON_IsNumber(lpJsonFileSizeLow)	)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonFileSizeLow
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %d"), SurveyFile_Response_FileSizeLow, lpJsonFileSizeLow->valueint);

				// Get and display the SurveyFile_Response_FileSizeHigh
				lpJsonFileSizeHigh = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyFileResponseResult, SurveyFile_Response_FileSizeHigh);
				if (NULL == lpJsonFileSizeHigh)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyFileResponseResult,
						SurveyFile_Response_FileSizeHigh
					);
					continue;
				}
				if (!cJSON_IsNumber(lpJsonFileSizeHigh))
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonFileSizeHigh
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %d"), SurveyFile_Response_FileSizeHigh, lpJsonFileSizeHigh->valueint);


				dwIndex++;

			} // end cJSON_ArrayForEach(lpJsonSurveyFileResponseResult, lpJsonSurveyFileResponseResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyFileOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonSurveyFileOutput);
				cJSON_Delete(lpJsonSurveyFileOutput);
				lpJsonSurveyFileOutput = NULL;
			}
			// Free configuration string
			if (NULL != szSurveyFileInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szSurveyFileInput);
				FREE(szSurveyFileInput);
				szSurveyFileInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szSurveyFileOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szSurveyFileOutput);
				cJSON_free(szSurveyFileOutput);
				szSurveyFileOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the SurveyFile plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyFile plugin"));
			dwTempErrorCode = SurveyFileFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyFileFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyFile wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
