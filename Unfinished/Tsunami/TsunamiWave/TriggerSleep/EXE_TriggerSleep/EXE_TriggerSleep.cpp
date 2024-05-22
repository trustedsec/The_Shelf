#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_TriggerSleep.h"
#include "LIB_TriggerSleep.h"

// Primary TsunamiWave Configuration
CHAR g_TriggerSleep_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_TriggerSleep_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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
	CHAR*	szNewConfigurationJson = NULL;
	DWORD	dwNewConfigurationJsonSize = 0;

	cJSON*	lpJsonTriggerSleepInput = NULL;
	cJSON*	lpJsonTriggerSleepInputSleepSeconds = NULL;
	cJSON*	lpJsonTriggerSleepInputVariationSeconds = NULL;

	CHAR*	szTriggerSleepInput = NULL;
	DWORD	dwTriggerSleepInputSize = 0;
	CHAR*	szTriggerSleepOutput = NULL;
	DWORD	dwTriggerSleepOutputSize = 0;

	cJSON*	lpJsonTriggerSleepOutput = NULL;
	cJSON*	lpJsonTriggerSleepOutputWakeTime = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwSleepSeconds = 0;
	DWORD	dwVariationSeconds = 0;

	LPBYTE	pBuffer = NULL;
	INT		nBufferSize = 0;

	PSYSTEMTIME pWakeTime = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_TriggerSleep wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			if (1 < argc)
			{
				dwSleepSeconds = _wtoi(argv[1]);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s sleep_time variation_time"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSleepSeconds: %d"), dwSleepSeconds);
			if (2 < argc)
			{
				dwVariationSeconds = _wtoi(argv[2]);
			}
			else
			{
				dwVariationSeconds = 0;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwVariationSeconds: %d"), dwVariationSeconds);


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_TriggerSleep_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_TriggerSleep_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_TriggerSleep_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_TriggerSleep_szPackedConfigurationPassword);

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
				Initialize the TriggerSleep plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the TriggerSleep plugin"));
			dwErrorCode = TriggerSleepInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TriggerSleepInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the TriggerSleepCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the TriggerSleepCallbackInterface"));

			// Create the input JSON object for the callback
			lpJsonTriggerSleepInput = cJSON_CreateObject();
			if (NULL == lpJsonTriggerSleepInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTriggerSleepInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create TriggerSleep_Request_Sleep_Seconds for the input
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSleepSeconds:  %d"), dwSleepSeconds);
			lpJsonTriggerSleepInputSleepSeconds = cJSON_CreateNumber(dwSleepSeconds);
			if (NULL == lpJsonTriggerSleepInputSleepSeconds)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTriggerSleepInputSleepSeconds = cJSON_CreateNumber(%d) failed."),
					dwSleepSeconds
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonTriggerSleepInput, TriggerSleep_Request_Sleep_Seconds, lpJsonTriggerSleepInputSleepSeconds);

			// Create TriggerSleep_Request_Variation_Seconds for the input
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwVariationSeconds:  %d"), dwVariationSeconds);
			lpJsonTriggerSleepInputVariationSeconds = cJSON_CreateNumber(dwVariationSeconds);
			if (NULL == lpJsonTriggerSleepInputVariationSeconds)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTriggerSleepInputVariationSeconds = cJSON_CreateNumber(%d) failed."),
					dwVariationSeconds
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonTriggerSleepInput, TriggerSleep_Request_Variation_Seconds, lpJsonTriggerSleepInputVariationSeconds);


			// Create the string representation of the TriggerSleep_Request
			szTriggerSleepInput = cJSON_Print(lpJsonTriggerSleepInput);
			if (NULL == szTriggerSleepInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szTriggerSleepInput = cJSON_Print(%p) failed."),
					szTriggerSleepInput
				);
				__leave;
			}
			dwTriggerSleepInputSize = (DWORD)strlen(szTriggerSleepInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwTriggerSleepInputSize: %d"), dwTriggerSleepInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szTriggerSleepInput:     %p"), szTriggerSleepInput);
			//printf("szTriggerSleepInput: \n%s\n", szTriggerSleepInput);

			// Call the TriggerSleepCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = TriggerSleepCallbackInterface(
				(LPBYTE)szTriggerSleepInput,
				dwTriggerSleepInputSize,
				(LPBYTE*)&szTriggerSleepOutput,
				&dwTriggerSleepOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TriggerSleepCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szTriggerSleepInput,
					dwTriggerSleepInputSize,
					(LPBYTE*)&szTriggerSleepOutput,
					&dwTriggerSleepOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwTriggerSleepOutputSize: %d"), dwTriggerSleepOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szTriggerSleepOutput:     %p"), szTriggerSleepOutput);
			//printf("szTriggerSleepOutput:\n%s\n", szTriggerSleepOutput);


			// Parse the output string into a JSON object
			lpJsonTriggerSleepOutput = cJSON_Parse(szTriggerSleepOutput);
			if (NULL == lpJsonTriggerSleepOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTriggerSleepOutput = cJSON_Parse(%p) failed."),
					szTriggerSleepOutput
				);
				__leave;
			}

			// Get the TriggerSleep_Response_Wake_Time JSON object
			lpJsonTriggerSleepOutputWakeTime = cJSON_GetObjectItemCaseSensitive(lpJsonTriggerSleepOutput, TriggerSleep_Response_Wake_Time);
			if (NULL == lpJsonTriggerSleepOutputWakeTime)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTriggerSleepOutputWakeTime = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonTriggerSleepOutput, TriggerSleep_Response_Wake_Time
				);
				__leave;
			}

			// Check the type and get the value of TriggerSleep_Response_Wake_Time
			if (
				(!cJSON_IsBytes(lpJsonTriggerSleepOutputWakeTime)) ||
				(NULL == lpJsonTriggerSleepOutputWakeTime->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonTriggerSleepOutputWakeTime
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonTriggerSleepOutputWakeTime, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonTriggerSleepOutputWakeTime, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("nBufferSize: %d"), nBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("pBuffer:     %p"), pBuffer);
			pWakeTime = (PSYSTEMTIME)pBuffer;
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %04d-%02d-%02d %02d:%02d:%02d"), TriggerSleep_Response_Wake_Time, pWakeTime->wYear, pWakeTime->wMonth, pWakeTime->wDay, pWakeTime->wHour, pWakeTime->wMinute, pWakeTime->wSecond);
			pWakeTime = NULL;

			// free allocated buffer
			if (NULL != pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonTriggerSleepInput)
			{
				cJSON_Delete(lpJsonTriggerSleepInput);
				lpJsonTriggerSleepInput = NULL;
			}
			if (NULL != lpJsonTriggerSleepOutput)
			{
				cJSON_Delete(lpJsonTriggerSleepOutput);
				lpJsonTriggerSleepOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szTriggerSleepInput)
			{
				cJSON_free(szTriggerSleepInput);
				szTriggerSleepInput = NULL;
			}
			if (NULL != szTriggerSleepOutput)
			{
				cJSON_free(szTriggerSleepOutput);
				szTriggerSleepOutput = NULL;
			}


		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonTriggerSleepInput)
			{
				cJSON_Delete(lpJsonTriggerSleepInput);
				lpJsonTriggerSleepInput = NULL;
			}
			if (NULL != lpJsonTriggerSleepOutput)
			{
				cJSON_Delete(lpJsonTriggerSleepOutput);
				lpJsonTriggerSleepOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szTriggerSleepInput)
			{
				cJSON_free(szTriggerSleepInput);
				szTriggerSleepInput = NULL;
			}
			if (NULL != szTriggerSleepOutput)
			{
				cJSON_free(szTriggerSleepOutput);
				szTriggerSleepOutput = NULL;
			}

			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the TriggerSleep plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the TriggerSleep plugin"));
			dwTempErrorCode = TriggerSleepFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TriggerSleepFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_TriggerSleep wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
