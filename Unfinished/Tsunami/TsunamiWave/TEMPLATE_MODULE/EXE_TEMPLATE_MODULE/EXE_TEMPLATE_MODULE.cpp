#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_TEMPLATE_MODULE.h"
#include "LIB_TEMPLATE_MODULE.h"

// Primary TsunamiWave Configuration
CHAR g_TEMPLATE_MODULE_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_TEMPLATE_MODULE_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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

	cJSON*	lpJsonTEMPLATE_MODULEInput = NULL;
	cJSON*	lpJsonTEMPLATE_MODULEInputSleepMilliseconds = NULL;

	CHAR*	szTEMPLATE_MODULEInput = NULL;
	DWORD	dwTEMPLATE_MODULEInputSize = 0;
	CHAR*	szTEMPLATE_MODULEOutput = NULL;
	DWORD	dwTEMPLATE_MODULEOutputSize = 0;

	cJSON*	lpJsonTEMPLATE_MODULEOutput = NULL;
	cJSON*	lpJsonTEMPLATE_MODULEOutputWakeTime = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwSleepTime = 0;

	LPBYTE	pBuffer = NULL;
	INT		nBufferSize = 0;

	PSYSTEMTIME pWakeTime = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_TEMPLATE_MODULE wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			if (1 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				dwSleepTime = _wtoi(argv[1]);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s sleep_time"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSleepTime: %d"), dwSleepTime);


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_TEMPLATE_MODULE_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_TEMPLATE_MODULE_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_TEMPLATE_MODULE_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_TEMPLATE_MODULE_szPackedConfigurationPassword);

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
				Initialize the TEMPLATE_MODULE plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the TEMPLATE_MODULE plugin"));
			dwErrorCode = TEMPLATE_MODULEInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TEMPLATE_MODULEInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the TEMPLATE_MODULECallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the TEMPLATE_MODULECallbackInterface"));

			// Create the input JSON object for the callback
			lpJsonTEMPLATE_MODULEInput = cJSON_CreateObject();
			if (NULL == lpJsonTEMPLATE_MODULEInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTEMPLATE_MODULEInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create TEMPLATE_MODULE_Request_Sleep_Milliseconds for the input
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSleepTime:  %d"), dwSleepTime);
			lpJsonTEMPLATE_MODULEInputSleepMilliseconds = cJSON_CreateNumber(dwSleepTime);
			if (NULL == lpJsonTEMPLATE_MODULEInputSleepMilliseconds)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTEMPLATE_MODULEInputSleepMilliseconds = cJSON_CreateNumber(%d) failed."),
					dwSleepTime
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonTEMPLATE_MODULEInput, TEMPLATE_MODULE_Request_Sleep_Milliseconds, lpJsonTEMPLATE_MODULEInputSleepMilliseconds);

			// Create the string representation of the TEMPLATE_MODULE_Request
			szTEMPLATE_MODULEInput = cJSON_Print(lpJsonTEMPLATE_MODULEInput);
			if (NULL == szTEMPLATE_MODULEInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szTEMPLATE_MODULEInput = cJSON_Print(%p) failed."),
					szTEMPLATE_MODULEInput
				);
				__leave;
			}
			dwTEMPLATE_MODULEInputSize = (DWORD)strlen(szTEMPLATE_MODULEInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwTEMPLATE_MODULEInputSize: %d"), dwTEMPLATE_MODULEInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szTEMPLATE_MODULEInput:     %p"), szTEMPLATE_MODULEInput);
			//printf("szTEMPLATE_MODULEInput: \n%s\n", szTEMPLATE_MODULEInput);

			// Call the TEMPLATE_MODULECallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = TEMPLATE_MODULECallbackInterface(
				(LPBYTE)szTEMPLATE_MODULEInput,
				dwTEMPLATE_MODULEInputSize,
				(LPBYTE*)&szTEMPLATE_MODULEOutput,
				&dwTEMPLATE_MODULEOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TEMPLATE_MODULECallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szTEMPLATE_MODULEInput,
					dwTEMPLATE_MODULEInputSize,
					(LPBYTE*)&szTEMPLATE_MODULEOutput,
					&dwTEMPLATE_MODULEOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwTEMPLATE_MODULEOutputSize: %d"), dwTEMPLATE_MODULEOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szTEMPLATE_MODULEOutput:     %p"), szTEMPLATE_MODULEOutput);
			//printf("szTEMPLATE_MODULEOutput:\n%s\n", szTEMPLATE_MODULEOutput);


			// Parse the output string into a JSON object
			lpJsonTEMPLATE_MODULEOutput = cJSON_Parse(szTEMPLATE_MODULEOutput);
			if (NULL == lpJsonTEMPLATE_MODULEOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTEMPLATE_MODULEOutput = cJSON_Parse(%p) failed."),
					szTEMPLATE_MODULEOutput
				);
				__leave;
			}

			// Get the TEMPLATE_MODULE_Response_Wake_Time JSON object
			lpJsonTEMPLATE_MODULEOutputWakeTime = cJSON_GetObjectItemCaseSensitive(lpJsonTEMPLATE_MODULEOutput, TEMPLATE_MODULE_Response_Wake_Time);
			if (NULL == lpJsonTEMPLATE_MODULEOutputWakeTime)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonTEMPLATE_MODULEOutputWakeTime = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonTEMPLATE_MODULEOutput, TEMPLATE_MODULE_Response_Wake_Time
				);
				__leave;
			}

			// Check the type and get the value of TEMPLATE_MODULE_Response_Wake_Time
			if (
				(!cJSON_IsBytes(lpJsonTEMPLATE_MODULEOutputWakeTime)) ||
				(NULL == lpJsonTEMPLATE_MODULEOutputWakeTime->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonTEMPLATE_MODULEOutputWakeTime
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonTEMPLATE_MODULEOutputWakeTime, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonTEMPLATE_MODULEOutputWakeTime, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("nBufferSize: %d"), nBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("pBuffer:     %p"), pBuffer);
			pWakeTime = (PSYSTEMTIME)pBuffer;
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %04d-%02d-%02d %02d:%02d:%02d"), TEMPLATE_MODULE_Response_Wake_Time, pWakeTime->wYear, pWakeTime->wMonth, pWakeTime->wDay, pWakeTime->wHour, pWakeTime->wMinute, pWakeTime->wSecond);
			pWakeTime = NULL;

			// free allocated buffer
			if (NULL != pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonTEMPLATE_MODULEInput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEInput);
				lpJsonTEMPLATE_MODULEInput = NULL;
			}
			if (NULL != lpJsonTEMPLATE_MODULEOutput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEOutput);
				lpJsonTEMPLATE_MODULEOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szTEMPLATE_MODULEInput)
			{
				cJSON_free(szTEMPLATE_MODULEInput);
				szTEMPLATE_MODULEInput = NULL;
			}
			if (NULL != szTEMPLATE_MODULEOutput)
			{
				cJSON_free(szTEMPLATE_MODULEOutput);
				szTEMPLATE_MODULEOutput = NULL;
			}


		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonTEMPLATE_MODULEInput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEInput);
				lpJsonTEMPLATE_MODULEInput = NULL;
			}
			if (NULL != lpJsonTEMPLATE_MODULEOutput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEOutput);
				lpJsonTEMPLATE_MODULEOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szTEMPLATE_MODULEInput)
			{
				cJSON_free(szTEMPLATE_MODULEInput);
				szTEMPLATE_MODULEInput = NULL;
			}
			if (NULL != szTEMPLATE_MODULEOutput)
			{
				cJSON_free(szTEMPLATE_MODULEOutput);
				szTEMPLATE_MODULEOutput = NULL;
			}

			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the TEMPLATE_MODULE plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the TEMPLATE_MODULE plugin"));
			dwTempErrorCode = TEMPLATE_MODULEFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TEMPLATE_MODULEFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_TEMPLATE_MODULE wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
