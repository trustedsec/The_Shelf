#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"
#include "LIB_LoaderMemoryModule.h"

#include "EXE_ManagerBasic.h"
#include "LIB_ManagerBasic.h"

// Primary TsunamiWave Configuration
CHAR g_ManagerBasic_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_ManagerBasic_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;



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

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;
	cJSON*	lpJsonRegister = NULL;

	cJSON*	lpJsonManagerBasicInput = NULL;
	CHAR*	szManagerBasicInput = NULL;
	DWORD	dwManagerBasicInputSize = 0;

	cJSON*	lpJsonManagerBasicOutput = NULL;
	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;

	cJSON*	lpJsonManagerBasicDispatchRequest = NULL;
	CHAR	szRequestCommand[MAX_PATH];
	cJSON*	lpJsonManagerBasicDispatchRequestCommandEntry = NULL;
	cJSON*	lpJsonManagerBasicDispatchRequestCommand = NULL;
	LPBYTE	lpInputMessage = NULL;
	DWORD	dwInputMessageSize = NULL;
	cJSON*	lpJsonManagerBasicDispatchResponse = NULL;

	cJSON*	lpJsonManagerBasicListHandlersRequest = NULL;
	cJSON*	lpJsonManagerBasicListHandlersResponse = NULL;
	cJSON*	lpJsonManagerBasicListHandlersResponseHandler = NULL;
	cJSON*	lpJsonManagerBasicListHandlersResponseHandlers = NULL;
	DWORD	dwMessageHandersIndex = 0;


	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;

	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;

	cJSON*	lpJsonLoadLibraryRequest = NULL;
	cJSON*	lpJsonLoadLibraryRequestLibraryBuffer = NULL;
	cJSON*	lpJsonLoadLibraryResponseModuleHandle = NULL;

	cJSON*	lpJsonGetProcAddrRequest = NULL;
	cJSON*	lpJsonGetProcAddrRequestFunctionName = NULL;
	cJSON*	lpJsonGetProcAddrRequestMemoryModule = NULL;
	cJSON*	lpJsonGetProcAddrResponseFunctionPointer = NULL;

	cJSON*	lpJsonFreeLibraryRequest = NULL;
	cJSON*	lpJsonFreeLibraryRequestMemoryModule = NULL;
	cJSON*	lpJsonFreeLibraryResponseResults = NULL;

	CHAR	szLibraryFilename[MAX_PATH];
	LPBYTE	lpLibraryContents     = NULL;
	DWORD	dwLibraryContentsSize = 0;
	CHAR	szInputMessageFilename[MAX_PATH];
	LPBYTE	lpInputMessageContents     = NULL;
	DWORD	dwInputMessageContentsSize = 0;
	cJSON*	lpJsonInputMessageContents = NULL;
	cJSON*	lpJsonInputMessageContent = NULL;

	CHAR	szFunctionName[MAX_PATH];

	HANDLE					hMemoryModule = NULL;
	f_CallbackInterface		m_Initialization = NULL;
	f_CallbackInterface		m_Finalization = NULL;
	f_CallbackInterface		m_CallbackInterface = NULL;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_ManagerBasic wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			ZeroMemory(szLibraryFilename, MAX_PATH);
			if (1 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				wcstombs(szLibraryFilename, argv[1], MAX_PATH);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s <dll_filename> <input_message>"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLibraryFilename: %S"), szLibraryFilename);
			if (2 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[2]: %s"), argv[2]);
				wcstombs(szInputMessageFilename, argv[2], MAX_PATH);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s <dll_filename> <input_message>"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInputMessageFilename: %S"), szInputMessageFilename);


			/********************************************************************************************
				Read input message into buffer
			********************************************************************************************/
			dwErrorCode = ReadFileIntoBuffer(
				szInputMessageFilename,
				&lpInputMessageContents,
				&dwInputMessageContentsSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer( %S, %p, %p ) failed.(%08x)"),
					szInputMessageFilename,
					&lpInputMessageContents,
					&dwInputMessageContentsSize,
					dwErrorCode);
				__leave;
			}

			lpJsonInputMessageContents = cJSON_Parse((char*)lpInputMessageContents);
			if (NULL == lpJsonInputMessageContents)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputMessageContents
				);
				__leave;
			}

			lpJsonManagerBasicInput = cJSON_Duplicate(lpJsonInputMessageContents, cJSON_True);

			cJSON_ArrayForEach(lpJsonInputMessageContent, lpJsonInputMessageContents)
			{
				if ( NULL == lpJsonInputMessageContent->string )
				{
					dwErrorCode = ERROR_BAD_ARGUMENTS;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInputMessageContent->string is NULL")
					);
					__leave;
				}
				//printf("lpJsonInputMessageContent->string: %s\n", lpJsonInputMessageContent->string);
				memset(szRequestCommand, 0, MAX_PATH);
				strcpy(szRequestCommand, lpJsonInputMessageContent->string);
			}
			printf("szRequestCommand: %s\n", szRequestCommand);

			/********************************************************************************************
				Read DLL into buffer
			********************************************************************************************/
			dwErrorCode = ReadFileIntoBuffer(
				szLibraryFilename,
				&lpLibraryContents,
				&dwLibraryContentsSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer( %S, %p, %p ) failed.(%08x)"),
					szLibraryFilename,
					&lpLibraryContents,
					&dwLibraryContentsSize,
					dwErrorCode);
				__leave;
			}


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_ManagerBasic_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_ManagerBasic_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_ManagerBasic_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_ManagerBasic_szPackedConfigurationPassword);

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

			// Parse the configuration string into a JSON object
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

			FREE(szConfigurationJson);
			szConfigurationJson = NULL;


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





			cJSON*	lpJsonInitializationInput = NULL;
			cJSON*	lpJsonConfigurationSettings = NULL;
			CHAR*	szInitializationInput = NULL;
			DWORD	dwInitializationInputSize = 0;

			cJSON*	lpJsonInitializationOutput = NULL;
			CHAR*	szInitializationOutput = NULL;
			DWORD	dwInitializationOutputSize = 0;
			
			// Create the input JSON object for the initialization
			lpJsonInitializationInput = cJSON_CreateObject();
			if (NULL == lpJsonInitializationInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInitializationInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create TSM_CONFIGURATION_SETTINGS for the input
			szConfigurationJson = cJSON_Print(lpJsonConfiguration);
			if (NULL == szConfigurationJson)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szConfigurationJson = cJSON_Print(%p) failed."),
					lpJsonConfiguration
				);
				__leave;
			}
			lpJsonConfigurationSettings = cJSON_CreateString(szConfigurationJson);
			if (NULL == lpJsonConfigurationSettings)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonConfigurationSettings = cJSON_CreateString(%p) failed."),
					szConfigurationJson
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonInitializationInput, TSM_CONFIGURATION_SETTINGS, lpJsonConfigurationSettings);

			// Create the string representation of the TriggerSleep_Request
			szInitializationInput = cJSON_Print(lpJsonInitializationInput);
			if (NULL == szInitializationInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szInitializationInput = cJSON_Print(%p) failed."),
					lpJsonInitializationInput
				);
				__leave;
			}
			dwInitializationInputSize = (DWORD)strlen(szInitializationInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitializationInputSize: %d"), dwInitializationInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInitializationInput:     %p"), szInitializationInput);
			//printf("szTriggerSleepInput: \n%s\n", szTriggerSleepInput);


			/***************************************************************************
				Initialize the ManagerBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the ManagerBasic plugin"));
			dwErrorCode = ManagerBasicInitialization(
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				(LPBYTE*)(&szInitializationOutput),
				&dwInitializationOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szInitializationInput,
					dwInitializationInputSize,
					(LPBYTE*)(&szInitializationOutput),
					&dwInitializationOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitializationOutputSize: %d"), dwInitializationOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInitializationOutput:     %p"), szInitializationOutput);
			//printf("[%s(%d)] szInitializationOutput:\n%s\n", __FILE__, __LINE__, szInitializationOutput);

			// Parse the output string into a JSON object
			lpJsonInitializationOutput = cJSON_Parse(szInitializationOutput);
			if (NULL == lpJsonInitializationOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInitializationOutput = cJSON_Parse(%p) failed."),
					szInitializationOutput
				);
				__leave;
			}

			// Get the Register function
			lpJsonRegister = cJSON_GetObjectItemCaseSensitive(lpJsonInitializationOutput, TSM_CONFIGURATION_REGISTER);
			if (NULL == lpJsonRegister)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonRegister = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInitializationOutput, TSM_CONFIGURATION_REGISTER
				);
				__leave;
			}
			// Check the TSM_CONFIGURATION_REGISTER JSON type
			if (
				!cJSON_IsBytes(lpJsonRegister) ||
				(NULL == lpJsonRegister->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonRegister
				);
				__leave;
			}
			// Get the TSM_CONFIGURATION_REGISTER byte buffer
			pBuffer = cJSON_GetBytesValue(lpJsonRegister, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonRegister,
					&nBufferSize
				);
				__leave;
			}
			// Convert the byte buffer into the HandlerFunction
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("%S: %p"),
				TSM_CONFIGURATION_REGISTER,
				*((f_CallbackInterface*)pBuffer)
			);
			// Free the byte buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;


			if (NULL != szConfigurationJson)
			{
				cJSON_free(szConfigurationJson);
				szConfigurationJson = NULL;
			}

			if (NULL != szInitializationInput)
			{
				cJSON_free(szInitializationInput);
				szInitializationInput = NULL;
			}

			if (NULL != szInitializationOutput)
			{
				cJSON_free(szInitializationOutput);
				szInitializationOutput = NULL;
			}

			if (NULL != lpJsonInitializationInput)
			{
				cJSON_Delete(lpJsonInitializationInput);
				lpJsonInitializationInput = NULL;
			}


			/***************************************************************************
				Updated the Initialize Input Arguments
			***************************************************************************/
			szInitializationInput = cJSON_Print(lpJsonInitializationOutput);
			if (NULL == szInitializationInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szInitializationInput = cJSON_Print(%p) failed."),
					lpJsonInitializationOutput
				);
				__leave;
			}
			dwInitializationInputSize = strlen(szInitializationInput);


			/***************************************************************************
				Initialize the LoaderMemoryModule plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the LoaderMemoryModule plugin"));
			dwErrorCode = LoaderMemoryModuleInitialization(
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szInitializationInput,
					dwInitializationInputSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the LoaderMemoryModuleLoadLibrary
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleLoadLibrary"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_LoadLibrary_Request JSON object
			lpJsonLoadLibraryRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_LoadLibrary_Request);
			if (NULL == lpJsonLoadLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadLibraryRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_LoadLibrary_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadLibraryRequest:    %p"), lpJsonLoadLibraryRequest);

			// Create the LoaderMemoryModule_LoadLibrary_LibraryBuffer input JSON object
			lpJsonLoadLibraryRequestLibraryBuffer = cJSON_CreateBytes(lpLibraryContents, dwLibraryContentsSize);
			if (NULL == lpJsonLoadLibraryRequestLibraryBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadLibraryRequestLibraryBuffer = cJSON_CreateBytes(%p,%d) failed."),
					lpLibraryContents,
					dwLibraryContentsSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoadLibraryRequest, LoaderMemoryModule_LoadLibrary_LibraryBuffer, lpJsonLoadLibraryRequestLibraryBuffer);

			// Create the string representation of the LoaderMemoryModule_LoadLibrary_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleLoadLibrary passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleLoadLibrary(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleLoadLibrary(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleLoadLibrary(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			//// Get the response JSON object
			//lpJsonLoadLibraryResponse = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_Response);
			//if (NULL == lpJsonLoadLibraryResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonLoadLibraryResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_Response
			//	);
			//	__leave;
			//}

			// Get and display the LoaderMemoryModule_LoadLibrary_ModuleHandle
			lpJsonLoadLibraryResponseModuleHandle = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_ModuleHandle);
			if (NULL == lpJsonLoadLibraryResponseModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonLoadLibraryResponseModuleHandle = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_ModuleHandle
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonLoadLibraryResponseModuleHandle)) ||
				(NULL == lpJsonLoadLibraryResponseModuleHandle->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonLoadLibraryResponseModuleHandle
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonLoadLibraryResponseModuleHandle, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoadLibraryResponseModuleHandle, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_LoadLibrary_ModuleHandle, *((HMODULE*)pBuffer));
			hMemoryModule = *((HMODULE*)pBuffer);

			// free allocated buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}



			/***************************************************************************
				Use the LoaderMemoryModuleGetProc to get the Initialization Function
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleGetProc to get the Initialization Function"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_GetProcAddr_Request JSON object
			lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request);
			if (NULL == lpJsonGetProcAddrRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonGetProcAddrRequest:    %p"), lpJsonGetProcAddrRequest);

			// Create the LoaderMemoryModule_GetProcAddr_FunctionName input JSON object
			lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(SZ_DEFAULT_Initialization_FUNC_NAME);
			if (NULL == lpJsonGetProcAddrRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(%S) failed."),
					szFunctionName
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName, lpJsonGetProcAddrRequestFunctionName);

			// Create the LoaderMemoryModule_GetProcAddr_ModuleHandle input JSON object
			lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonGetProcAddrRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hMemoryModule),
					sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_ModuleHandle, lpJsonGetProcAddrRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_GetProcAddr_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleGetProc passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleGetProcAddr(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			//// Get the response JSON object
			//lpJsonGetProcAddrResponse = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response);
			//if (NULL == lpJsonGetProcAddrResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonGetProcAddrResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response
			//	);
			//	__leave;
			//}

			// Get and display the LoaderMemoryModule_GetProcAddr_FunctionPointer
			lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer);
			if (NULL == lpJsonGetProcAddrResponseFunctionPointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonGetProcAddrResponseFunctionPointer)) ||
				(NULL == lpJsonGetProcAddrResponseFunctionPointer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_GetProcAddr_FunctionPointer, *((HMODULE*)pBuffer));
			m_Initialization = *((f_CallbackInterface*)pBuffer);

			// free allocated buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}

			DBGPRINT(DEBUG_INFO, TEXT("m_Initialization:        %p"), m_Initialization);


			/***************************************************************************
				Use the LoaderMemoryModuleGetProc to get the Finalization function
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleGetProc to get the Finalization function"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_GetProcAddr_Request JSON object
			lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request);
			if (NULL == lpJsonGetProcAddrRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonGetProcAddrRequest:    %p"), lpJsonGetProcAddrRequest);

			// Create the LoaderMemoryModule_GetProcAddr_FunctionName input JSON object
			lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(SZ_DEFAULT_Finalization_FUNC_NAME);
			if (NULL == lpJsonGetProcAddrRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(%S) failed."),
					szFunctionName
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName, lpJsonGetProcAddrRequestFunctionName);

			// Create the LoaderMemoryModule_GetProcAddr_ModuleHandle input JSON object
			lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonGetProcAddrRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hMemoryModule),
					sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_ModuleHandle, lpJsonGetProcAddrRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_GetProcAddr_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleGetProc passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleGetProcAddr(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			//// Get the response JSON object
			//lpJsonGetProcAddrResponse = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response);
			//if (NULL == lpJsonGetProcAddrResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonGetProcAddrResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response
			//	);
			//	__leave;
			//}

			// Get and display the LoaderMemoryModule_GetProcAddr_FunctionPointer
			lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer);
			if (NULL == lpJsonGetProcAddrResponseFunctionPointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonGetProcAddrResponseFunctionPointer)) ||
				(NULL == lpJsonGetProcAddrResponseFunctionPointer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_GetProcAddr_FunctionPointer, *((HMODULE*)pBuffer));
			m_Finalization = *((f_CallbackInterface*)pBuffer);

			// free allocated buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}

			DBGPRINT(DEBUG_INFO, TEXT("m_Finalization:        %p"), m_Finalization);


			/***************************************************************************
				Use the m_Initialization
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the %S"), szLibraryFilename);
			dwErrorCode = m_Initialization(
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_Initialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szInitializationInput,
					dwInitializationInputSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_Initialization(%p, %d, %p, %p) was successful"),
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				NULL,
				NULL
			);

			/***************************************************************************
				Use the ManagerBasicDispatchMessage
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the ManagerBasicDispatchMessage"));

			/***************************************************************************
				Create the TSM_CONFIGURATION_REGISTER input JSON
			***************************************************************************/
			// Create the input JSON object
			//lpJsonManagerBasicInput = lpJsonInputMessageContent;
			//if (NULL == lpJsonManagerBasicInput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonManagerBasicInput = cJSON_CreateObject failed.\n")
			//	);
			//	__leave;
			//}

			// Add the command type
			lpJsonManagerBasicDispatchRequestCommand = cJSON_AddStringToObject(lpJsonManagerBasicInput, TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_COMMAND, szRequestCommand);
			if (NULL == lpJsonManagerBasicDispatchRequestCommand)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicDispatchRequestCommand = cJSON_AddStringToObject( %p, %S, %S ) failed."),
					lpJsonManagerBasicInput, TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_COMMAND, szRequestCommand
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonManagerBasicDispatchRequestCommand:     %p"), lpJsonManagerBasicDispatchRequestCommand);


			/********************************************************************************
				Create the TSM_MANAGER_DISPATCH_MESSAGE input string
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create TSM_MANAGER_DISPATCH_MESSAGE input message..."));
			szManagerBasicInput = cJSON_Print(lpJsonManagerBasicInput);
			if (NULL == szManagerBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicInput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szManagerBasicInput:     %p"), szManagerBasicInput);
			//printf("[%s(%d)] szManagerBasicInput:\n%s\n", __FILE__, __LINE__, szManagerBasicInput);
			dwManagerBasicInputSize = (DWORD)strlen(szManagerBasicInput);

			// Call the ManagerBasicDispatchMessage
			dwErrorCode = ManagerBasicCallbackDispatchMessage(
				(LPBYTE)szManagerBasicInput,
				dwManagerBasicInputSize,
				(LPBYTE*)&szManagerBasicOutput,
				&dwManagerBasicOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ManagerBasicCallbackDispatchMessage(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szManagerBasicInput,
					dwManagerBasicInputSize,
					(LPBYTE*)&szManagerBasicOutput,
					&dwManagerBasicOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwManagerBasicOutputSize: %d"), dwManagerBasicOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szManagerBasicOutput:     %p"), szManagerBasicOutput);
			//printf("[%s(%d)] szManagerBasicOutput:\n%s\n", __FILE__, __LINE__, szManagerBasicOutput);

			// Parse the output string into a JSON object
			lpJsonManagerBasicOutput = cJSON_Parse(szManagerBasicOutput);
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonDispatchResponse = cJSON_Parse(%p) failed."),
					szManagerBasicOutput
				);
				__leave;
			}


			// Get the response output
			lpJsonManagerBasicDispatchResponse = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicOutput, TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE);
			if (NULL == lpJsonManagerBasicDispatchResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicOutput, TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonManagerBasicDispatchResponse)) ||
				(NULL == lpJsonManagerBasicDispatchResponse->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonManagerBasicDispatchResponse
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonManagerBasicDispatchResponse, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonManagerBasicDispatchResponse, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S:      %p"), TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE, pBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S_size: %d"), TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE, nBufferSize);
			printf("[%s(%d)]  %s:\n%s\n", __FILE__, __LINE__, TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE, pBuffer);

			// free allocated buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;

			// Delete input and output JSON objects
			if (NULL != lpJsonManagerBasicInput)
			{
				cJSON_Delete(lpJsonManagerBasicInput);
				lpJsonManagerBasicInput = NULL;
			}
			if (NULL != lpJsonManagerBasicOutput)
			{
				cJSON_Delete(lpJsonManagerBasicOutput);
				lpJsonManagerBasicOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szManagerBasicInput)
			{
				cJSON_free(szManagerBasicInput);
				szManagerBasicInput = NULL;
			}
			if (NULL != szManagerBasicOutput)
			{
				cJSON_free(szManagerBasicOutput);
				szManagerBasicOutput = NULL;
			}



			/***************************************************************************
				Use the ManagerBasicListHandlers
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the ManagerBasicListHandlers"));

			/***************************************************************************
				Create the TSM_CONFIGURATION_REGISTER input JSON
			***************************************************************************/
			// Create the input JSON object
			lpJsonManagerBasicInput = cJSON_CreateObject();
			if (NULL == lpJsonManagerBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Create the TSM_MANAGER_LIST_HANDLERS input string
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create TSM_MANAGER_LIST_HANDLERS input message..."));
			szManagerBasicInput = cJSON_Print(lpJsonManagerBasicInput);
			if (NULL == szManagerBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicInput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szManagerBasicInput:     %p"), szManagerBasicInput);
			//printf("[%s(%d)] szManagerBasicInput:\n%s\n", __FILE__, __LINE__, szManagerBasicInput);
			dwManagerBasicInputSize = (DWORD)strlen(szManagerBasicInput);

			// Call the ManagerBasicListHandlers
			dwErrorCode = ManagerBasicCallbackListHandlers(
				(LPBYTE)szManagerBasicInput,
				dwManagerBasicInputSize,
				(LPBYTE*)&szManagerBasicOutput,
				&dwManagerBasicOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ManagerBasicCallbackListHandlers(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szManagerBasicInput,
					dwManagerBasicInputSize,
					(LPBYTE*)&szManagerBasicOutput,
					&dwManagerBasicOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwManagerBasicOutputSize: %d"), dwManagerBasicOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szManagerBasicOutput:     %p"), szManagerBasicOutput);
			//printf("[%s(%d)] szManagerBasicOutput:\n%s\n", __FILE__, __LINE__, szManagerBasicOutput);

			// Parse the output string into a JSON object
			lpJsonManagerBasicOutput = cJSON_Parse(szManagerBasicOutput);
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonDispatchResponse = cJSON_Parse(%p) failed."),
					szManagerBasicOutput
				);
				__leave;
			}


			// Get the response output
			lpJsonManagerBasicListHandlersResponseHandlers = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicOutput, TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS);
			if (NULL == lpJsonManagerBasicListHandlersResponseHandlers)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicListHandlersResponseHandlers, TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS
				);
				__leave;
			}
			if ( !cJSON_IsArray(lpJsonManagerBasicListHandlersResponseHandlers) )
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonManagerBasicListHandlersResponseHandlers
				);
				__leave;
			}
			
			// Loop through each output message and display the result
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S:"), TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS);
			cJSON_ArrayForEach(lpJsonManagerBasicListHandlersResponseHandler, lpJsonManagerBasicListHandlersResponseHandlers)
			{
				CHAR* szCurrentMessageHandler = NULL;
				szCurrentMessageHandler = cJSON_GetStringValue(lpJsonManagerBasicListHandlersResponseHandler);
				if (NULL == szCurrentMessageHandler)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_GetStringValue(%p) failed."),
						lpJsonManagerBasicListHandlersResponseHandler
					);
					__leave;
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("  %S[%d]: %S"), TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS, dwMessageHandersIndex, szCurrentMessageHandler);

				dwMessageHandersIndex++;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonManagerBasicInput)
			{
				cJSON_Delete(lpJsonManagerBasicInput);
				lpJsonManagerBasicInput = NULL;
			}
			if (NULL != lpJsonManagerBasicOutput)
			{
				cJSON_Delete(lpJsonManagerBasicOutput);
				lpJsonManagerBasicOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szManagerBasicInput)
			{
				cJSON_free(szManagerBasicInput);
				szManagerBasicInput = NULL;
			}
			if (NULL != szManagerBasicOutput)
			{
				cJSON_free(szManagerBasicOutput);
				szManagerBasicOutput = NULL;
			}

			

			/***************************************************************************
				Use the m_Finalization
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the %S"), szLibraryFilename);
			dwTempErrorCode = m_Finalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_Finalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			


			/***************************************************************************
				Use the LoaderMemoryModuleFreeLibrary
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleFreeLibrary"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_FreeLibrary_Request JSON object
			lpJsonFreeLibraryRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_FreeLibrary_Request);
			if (NULL == lpJsonFreeLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFreeLibraryRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_FreeLibrary_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonFreeLibraryRequest:    %p"), lpJsonFreeLibraryRequest);

			// Create the LoaderMemoryModule_FreeLibrary_ModuleHandle input JSON object
			lpJsonFreeLibraryRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonFreeLibraryRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFreeLibraryRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hMemoryModule), sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFreeLibraryRequest, LoaderMemoryModule_FreeLibrary_ModuleHandle, lpJsonFreeLibraryRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_FreeLibrary_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleFreeLibrary passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleFreeLibrary(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleFreeLibrary(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleFreeLibrary(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			//// Get the response JSON object
			//lpJsonFreeLibraryResponse = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Response);
			//if (NULL == lpJsonFreeLibraryResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonFreeLibraryResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Response
			//	);
			//	__leave;
			//}

			// Get and display the LoaderMemoryModule_FreeLibrary_Results
			lpJsonFreeLibraryResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Results);
			if (NULL == lpJsonFreeLibraryResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonFreeLibraryResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Results
				);
				__leave;
			}
			if (!cJSON_IsBool(lpJsonFreeLibraryResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBool(%p) failed."),
					lpJsonFreeLibraryResponseResults
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %d"), LoaderMemoryModule_FreeLibrary_Results, lpJsonFreeLibraryResponseResults->valueint);

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}


		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the LoaderMemoryModule plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the LoaderMemoryModule plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = LoaderMemoryModuleFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the ManagerBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the ManagerBasic plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("ManagerBasicFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = ManagerBasicFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ManagerBasicFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_ManagerBasic wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
