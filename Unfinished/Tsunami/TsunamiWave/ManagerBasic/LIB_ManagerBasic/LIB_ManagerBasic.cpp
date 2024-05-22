#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_ManagerBasic.h"
#include "Internal_Functions.h"


// Global variables
cJSON* g_ManagerBasic_lpJsonManagerBasicConfig = NULL;


DWORD WINAPI ManagerBasicInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonManagerBasicInitializeInput = NULL;
	cJSON*	lpJsonConfigurationSettings = NULL;
	CHAR*	szConfigurationSettings = NULL;

	f_CallbackInterface fpRegisterFunction = NULL;
	cJSON*	lpJsonRegister = NULL;
	f_CallbackInterface fpUnregisterFunction = NULL;
	cJSON*	lpJsonUnregister = NULL;

	cJSON*	lpJsonManagerBasicInitializeOutput = NULL;
	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;

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
			

			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonManagerBasicInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonManagerBasicInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonManagerBasicInitializeInput: %p"), lpJsonManagerBasicInitializeInput);

			
			/********************************************************************************
				Get the configuration settings
			********************************************************************************/
			//// Get the TSM_CONFIGURATION_SETTINGS
			//lpJsonConfigurationSettings = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicInitializeInput, TSM_CONFIGURATION_SETTINGS);
			//if (NULL == lpJsonConfigurationSettings)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonConfigurationSettings = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonManagerBasicInitializeInput, TSM_CONFIGURATION_SETTINGS
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonConfigurationSettings: %p"), lpJsonConfigurationSettings);

			//// Get the actual string  from the TSM_CONFIGURATION_SETTINGS JSON
			//szConfigurationSettings = cJSON_GetStringValue(lpJsonConfigurationSettings);
			//if ( NULL == szConfigurationSettings )
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("szConfigurationSettings = cJSON_GetStringValue(%p) failed."),
			//		lpJsonConfigurationSettings
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szConfigurationSettings: %p"), szConfigurationSettings);
			//printf("[%s(%d)] szConfigurationSettings:\n%s\n", __FILE__, __LINE__, szConfigurationSettings);

			//// Set the local module's configuration to the input configuration
			//lpJsonConfigurationSettings = cJSON_Parse(szConfigurationSettings);
			//if (NULL == g_ManagerBasic_lpJsonManagerBasicConfig)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Parse(%p) failed."),
			//		(CHAR*)lpInputBuffer
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("g_ManagerBasic_lpJsonManagerBasicConfig: %p"), g_ManagerBasic_lpJsonManagerBasicConfig);

			//if (NULL != szConfigurationSettings)
			//{
			//	cJSON_free(szConfigurationSettings);
			//	szConfigurationSettings = NULL;
			//}


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/
			// Create a duplicate of the JSON configuration passed in
			lpJsonManagerBasicInitializeOutput = cJSON_Duplicate(lpJsonManagerBasicInitializeInput, cJSON_True);
			if (NULL == lpJsonManagerBasicInitializeOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicInitializeOutput = cJSON_Duplicate(%p,%d) failed.\n"),
					lpJsonManagerBasicInitializeInput, cJSON_True
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonManagerBasicInitializeOutput: %p"), lpJsonManagerBasicInitializeOutput);

			fpRegisterFunction = (f_CallbackInterface)(ManagerBasicCallbackRegisterHandler);
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpRegisterFunction:     %p"), fpRegisterFunction);
			lpJsonRegister = cJSON_CreateBytes((LPBYTE)(&fpRegisterFunction), sizeof(fpRegisterFunction));
			if (NULL == lpJsonRegister)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonRegister = cJSON_CreateBytes(%p,%d) failed.\n"),
					(LPBYTE)fpRegisterFunction, sizeof(fpRegisterFunction)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonManagerBasicInitializeOutput, TSM_CONFIGURATION_REGISTER, lpJsonRegister);

			fpUnregisterFunction = (f_CallbackInterface)(ManagerBasicCallbackUnregisterHandler);
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpUnregisterFunction:     %p"), fpUnregisterFunction);
			lpJsonUnregister = cJSON_CreateBytes((LPBYTE)(&fpUnregisterFunction), sizeof(fpUnregisterFunction));
			if (NULL == lpJsonUnregister)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonUnregister = cJSON_CreateBytes(%p,%d) failed.\n"),
					(LPBYTE)fpUnregisterFunction, sizeof(fpUnregisterFunction)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonManagerBasicInitializeOutput, TSM_CONFIGURATION_UNREGISTER, lpJsonUnregister);

			// Set our own copy of the initialization configuration settings
			g_ManagerBasic_lpJsonManagerBasicConfig = lpJsonManagerBasicInitializeOutput;


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register TSM_MANAGER_LIST_HANDLERS_REQUEST"));
			if ( FALSE == RegisterModuleMessageHandler(
						g_ManagerBasic_lpJsonManagerBasicConfig,
						TSM_MANAGER_LIST_HANDLERS_REQUEST,
						ManagerBasicCallbackListHandlers
					)
				)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_ManagerBasic_lpJsonManagerBasicConfig,
					TSM_MANAGER_LIST_HANDLERS_REQUEST,
					ManagerBasicCallbackListHandlers
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register TSM_MANAGER_LIST_HANDLERS_REQUEST was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szManagerBasicOutput = cJSON_Print(lpJsonManagerBasicInitializeOutput);
			if (NULL == szManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicInitializeOutput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szManagerBasicOutput:     %p"), szManagerBasicOutput);
			//printf("szManagerBasicOutput:\n%s\n", szManagerBasicOutput);
			dwManagerBasicOutputSize = (DWORD)strlen(szManagerBasicOutput);

			(*lppOutputBuffer) = (LPBYTE)szManagerBasicOutput;
			(*lpdwOutputBufferSize) = dwManagerBasicOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != szConfigurationSettings)
			{
				cJSON_free(szConfigurationSettings);
				szConfigurationSettings = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonManagerBasicInitializeInput)
			{
				cJSON_Delete(lpJsonManagerBasicInitializeInput);
				lpJsonManagerBasicInitializeInput = NULL;
			}
			//if (NULL != lpJsonManagerBasicInitializeOutput)
			//{
			//	cJSON_Delete(lpJsonManagerBasicInitializeOutput);
			//	lpJsonManagerBasicInitializeOutput = NULL;
			//}
			
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


DWORD WINAPI ManagerBasicFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

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
			//szManagerBasicInput = (char*)lpInputBuffer;
			//dwManagerBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TSM_SURVEY_QUERY_REQUEST"));
			//bRegisterResult = UnregisterModuleMessageHandler(
			//	g_ManagerBasic_lpJsonStorageBinaryConfig,
			//	TSM_SURVEY_REGISTRY_ID,
			//	TSM_SURVEY_QUERY_REQUEST
			//);
			//if (FALSE == bRegisterResult)
			//{
			//	//dwErrorCode = ERROR_BAD_PROVIDER;
			//	DBGPRINT(
			//		DEBUG_WARNING,
			//		TEXT("UnregisterModuleMessageHandler(%p, %d, %d) failed."),
			//		g_ManagerBasic_lpJsonStorageBinaryConfig,
			//		TSM_SURVEY_REGISTRY_ID,
			//		TSM_SURVEY_QUERY_REQUEST
			//	);
			//	//__leave;
			//}
			//else
			//{
			//	DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TSM_SURVEY_QUERY_REQUEST was successful."));
			//}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/
			// Remove all message handlers
			MessageHandlerRemove(NULL);


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("FinalizeAdvapi32Functions()"));
			//dwTempErrorCode = FinalizeAdvapi32Functions();
			//if (ERROR_SUCCESS != dwTempErrorCode)
			//{
			//	DBGPRINT(DEBUG_ERROR, TEXT("FinalizeAdvapi32Functions() failed.(%08x)"), dwTempErrorCode);
			//	//__leave;
			//}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_ManagerBasic_lpJsonManagerBasicConfig)
			{
				cJSON_Delete(g_ManagerBasic_lpJsonManagerBasicConfig);
				g_ManagerBasic_lpJsonManagerBasicConfig = NULL;
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


DWORD WINAPI ManagerBasicCallbackRegisterHandler(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szManagerBasicInput = NULL;
	DWORD	dwManagerBasicInputSize = 0;
	cJSON*	lpJsonManagerBasicInput = NULL;
	cJSON*	lpJsonManagerBasicRegisterRequest = NULL;
	cJSON*	lpJsonManagerBasicRegisterRequestMessageHandlerType = NULL;
	cJSON*	lpJsonManagerBasicRegisterRequestMessageHandler = NULL;

	CHAR*	szMessageHandlerType = NULL;
	f_CallbackInterface	fpMessageHandler = NULL;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;
	cJSON*	lpJsonManagerBasicOutput = NULL;
	cJSON*	lpJsonManagerBasicRegisterResponse = NULL;
	cJSON*	lpJsonManagerBasicRegisterResponseResult = NULL;


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
			szManagerBasicInput = (char*)lpInputBuffer;
			dwManagerBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonManagerBasicInput = cJSON_Parse(szManagerBasicInput);
			if (NULL == lpJsonManagerBasicInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szManagerBasicInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonManagerBasicOutput = cJSON_CreateObject();
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the TSM_MANAGER_REGISTER_RESPONSE JSON object
			lpJsonManagerBasicRegisterResponse = cJSON_AddObjectToObject(lpJsonManagerBasicOutput, TSM_MANAGER_REGISTER_RESPONSE);
			if (NULL == lpJsonManagerBasicRegisterResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonManagerBasicOutput, TSM_MANAGER_REGISTER_RESPONSE
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonManagerBasicRegisterResponse:    %p"), lpJsonManagerBasicRegisterResponse);



			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the TSM_MANAGER_REGISTER_REQUEST object
			lpJsonManagerBasicRegisterRequest = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicInput, TSM_MANAGER_REGISTER_REQUEST);
			if (NULL == lpJsonManagerBasicRegisterRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicInput, TSM_MANAGER_REGISTER_REQUEST
				);
				__leave;
			}
				
			// Get the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			lpJsonManagerBasicRegisterRequestMessageHandlerType = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE);
			if (NULL == lpJsonManagerBasicRegisterRequestMessageHandlerType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicRegisterRequestMessageId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE
				);
				__leave;
			}
			// Check the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE JSON type
			if (
				!cJSON_IsString(lpJsonManagerBasicRegisterRequestMessageHandlerType) ||
				(NULL == lpJsonManagerBasicRegisterRequestMessageHandlerType->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonManagerBasicRegisterRequestMessageHandlerType
				);
				__leave;
			}
			// Set the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			szMessageHandlerType = lpJsonManagerBasicRegisterRequestMessageHandlerType->valuestring;
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType);

			// Get the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER
			lpJsonManagerBasicRegisterRequestMessageHandler = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER);
			if (NULL == lpJsonManagerBasicRegisterRequestMessageHandler)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicRegisterRequestMessageHandler = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER
				);
				__leave;
			}
			// Check the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER JSON type
			if (
				!cJSON_IsBytes(lpJsonManagerBasicRegisterRequestMessageHandler) ||
				(NULL == lpJsonManagerBasicRegisterRequestMessageHandler->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonManagerBasicRegisterRequestMessageHandler
				);
				__leave;
			}
			// Get the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER byte buffer
			pBuffer = cJSON_GetBytesValue(lpJsonManagerBasicRegisterRequestMessageHandler, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonManagerBasicRegisterRequestMessageHandler,
					&nBufferSize
				);
				__leave;
			}
			// Convert the byte buffer into the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER
			fpMessageHandler = *((f_CallbackInterface*)pBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %p"), TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER, fpMessageHandler);
			// Free the byte buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;


			// Add the request registration to our list of registered functions
			//if (MessageHandlerInsert(dwRequestModuleId, dwRequestMessageId, fpMessageHandler ))
			if (MessageHandlerInsert(szMessageHandlerType, fpMessageHandler))
			{
				cJSON_AddBoolToObject(lpJsonManagerBasicRegisterResponse, TSM_MANAGER_REGISTER_RESPONSE_RESULT, cJSON_True);
			}
			else
			{
				cJSON_AddBoolToObject(lpJsonManagerBasicRegisterResponse, TSM_MANAGER_REGISTER_RESPONSE_RESULT, cJSON_False);
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szManagerBasicOutput = cJSON_Print(lpJsonManagerBasicOutput);
			if (NULL == szManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicOutput
				);
				__leave;
			}
			dwManagerBasicOutputSize = (DWORD)strlen(szManagerBasicOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szManagerBasicOutput;
			(*lpdwOutputBufferSize) = dwManagerBasicOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szManagerBasicOutput);
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


DWORD WINAPI ManagerBasicCallbackUnregisterHandler(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szManagerBasicInput = NULL;
	DWORD	dwManagerBasicInputSize = 0;
	cJSON*	lpJsonManagerBasicInput = NULL;
	cJSON*	lpJsonManagerBasicUnregisterRequest = NULL;
	cJSON*	lpJsonManagerBasicUnregisterRequestMessageHandlerType = NULL;

	CHAR*	szMessageHandlerType = NULL;

	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;
	cJSON*	lpJsonManagerBasicOutput = NULL;
	cJSON*	lpJsonManagerBasicUnregisterResponse = NULL;
	cJSON*	lpJsonManagerBasicUnregisterResponseResult = NULL;


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
			szManagerBasicInput = (char*)lpInputBuffer;
			dwManagerBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonManagerBasicInput = cJSON_Parse(szManagerBasicInput);
			if (NULL == lpJsonManagerBasicInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szManagerBasicInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonManagerBasicOutput = cJSON_CreateObject();
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			lpJsonManagerBasicUnregisterResponse = cJSON_AddObjectToObject(lpJsonManagerBasicOutput, TSM_MANAGER_UNREGISTER_RESPONSE);
			if (NULL == lpJsonManagerBasicUnregisterResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonManagerBasicOutput, TSM_MANAGER_UNREGISTER_RESPONSE
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonManagerBasicUnregisterResponse:    %p"), lpJsonManagerBasicUnregisterResponse);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the TSM_MANAGER_UNREGISTER_REQUEST object
			lpJsonManagerBasicUnregisterRequest = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicInput, TSM_MANAGER_UNREGISTER_REQUEST);
			if (NULL == lpJsonManagerBasicUnregisterRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicInput, TSM_MANAGER_UNREGISTER_REQUEST
				);
				__leave;
			}

			// Get the TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			lpJsonManagerBasicUnregisterRequestMessageHandlerType = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicUnregisterRequest, TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE);
			if (NULL == lpJsonManagerBasicUnregisterRequestMessageHandlerType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicRegisterRequestMessageId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicUnregisterRequestMessageHandlerType, TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE
				);
				__leave;
			}
			// Check the TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE JSON type
			if (
				!cJSON_IsString(lpJsonManagerBasicUnregisterRequestMessageHandlerType) ||
				(NULL == lpJsonManagerBasicUnregisterRequestMessageHandlerType->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonManagerBasicUnregisterRequestMessageHandlerType
				);
				__leave;
			}
			// Set the TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			szMessageHandlerType = lpJsonManagerBasicUnregisterRequestMessageHandlerType->valuestring;
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType);


			// Add the request registration to our list of registered functions
			//if (MessageHandlerRemove(dwRequestModuleId, dwRequestMessageId))
			if (MessageHandlerRemove(szMessageHandlerType))
			{
				cJSON_AddBoolToObject(lpJsonManagerBasicUnregisterResponse, TSM_MANAGER_UNREGISTER_RESPONSE_RESULT, cJSON_True);
			}
			else
			{
				cJSON_AddBoolToObject(lpJsonManagerBasicUnregisterResponse, TSM_MANAGER_UNREGISTER_RESPONSE_RESULT, cJSON_False);
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szManagerBasicOutput = cJSON_Print(lpJsonManagerBasicOutput);
			if (NULL == szManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicOutput
				);
				__leave;
			}
			dwManagerBasicOutputSize = (DWORD)strlen(szManagerBasicOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szManagerBasicOutput;
			(*lpdwOutputBufferSize) = dwManagerBasicOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szManagerBasicOutput);
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


DWORD WINAPI ManagerBasicCallbackDispatchMessage(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szManagerBasicInput = NULL;
	DWORD	dwManagerBasicInputSize = 0;
	cJSON*	lpJsonManagerBasicInput = NULL;
	cJSON*	lpJsonManagerBasicDispatchRequest = NULL;
	cJSON*	lpJsonManagerBasicDispatchRequestCommand = NULL;
	cJSON*	lpJsonManagerBasicDispatchRequestCommandEntry = NULL;

	CHAR*	szMessageHandlerType = NULL;
	LPBYTE	lpMessageHandlerInput = NULL;
	DWORD	dwMessageHandlerInputSize = 0;
	LPBYTE	lpMessageHandlerOutput = NULL;
	DWORD	dwMessageHandlerOutputSize = 0;

	f_CallbackInterface fpMessageHandler = NULL;

	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;
	cJSON*	lpJsonManagerBasicOutput = NULL;
	cJSON*	lpJsonManagerBasicDispatchResponse = NULL;


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
			szManagerBasicInput = (char*)lpInputBuffer;
			dwManagerBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			//printf("[%s(%d)] szManagerBasicInput:\n%s\n", __FILE__, __LINE__, szManagerBasicInput);
			lpJsonManagerBasicInput = cJSON_Parse(szManagerBasicInput);
			if (NULL == lpJsonManagerBasicInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szManagerBasicInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonManagerBasicOutput = cJSON_Duplicate(lpJsonManagerBasicInput, cJSON_True);
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicOutput = cJSON_Duplicate( %p, %d ) failed.\n"),
					lpJsonManagerBasicInput, cJSON_True
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the command type
			lpJsonManagerBasicDispatchRequestCommand = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicInput, TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_COMMAND);
			if (NULL == lpJsonManagerBasicDispatchRequestCommand)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicInput,
					TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_COMMAND
				);
				__leave;
			}
			if (
				!cJSON_IsString(lpJsonManagerBasicDispatchRequestCommand) ||
				(NULL == lpJsonManagerBasicDispatchRequestCommand->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonManagerBasicDispatchRequestCommand
				);
				__leave;
			}
			szMessageHandlerType = cJSON_GetStringValue(lpJsonManagerBasicDispatchRequestCommand);
			if (NULL == szMessageHandlerType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonManagerBasicDispatchRequestCommand
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szMessageHandlerType:     %S"), szMessageHandlerType);


			// Get the handler function
			DBGPRINT(DEBUG_VERBOSE, TEXT("MessageHandlerFindFirst( %S )"), szMessageHandlerType);
			fpMessageHandler = MessageHandlerFindFirst(szMessageHandlerType);
			if (NULL == fpMessageHandler)
			{
				dwErrorCode = ERROR_UNSUPPORTED_TYPE;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("fpMessageHandler = MessageHandlerFindFirst(%S) failed."),
					szMessageHandlerType
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpMessageHandler:          %p"), fpMessageHandler);


			// Get the command entry
			lpJsonManagerBasicDispatchRequestCommandEntry = cJSON_GetObjectItemCaseSensitive(lpJsonManagerBasicInput, szMessageHandlerType);
			if (NULL == lpJsonManagerBasicDispatchRequestCommandEntry)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerBasicInput,
					szMessageHandlerType
				);
				__leave;
			}
			if (
				!cJSON_IsBytes(lpJsonManagerBasicDispatchRequestCommandEntry) ||
				(NULL == lpJsonManagerBasicDispatchRequestCommandEntry->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonManagerBasicDispatchRequestCommandEntry
				);
				__leave;
			}
			lpMessageHandlerInput = cJSON_GetBytesValue(lpJsonManagerBasicDispatchRequestCommandEntry, (int*)(&dwMessageHandlerInputSize));
			if (NULL == lpMessageHandlerInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonManagerBasicDispatchRequestCommandEntry, &dwMessageHandlerInputSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwMessageHandlerInputSize: %d"), dwMessageHandlerInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpMessageHandlerInput:     %p"), lpMessageHandlerInput);


			// Call the handler
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("fpMessageHandler( %p, %d, %p, %p )"),
				lpMessageHandlerInput, dwMessageHandlerInputSize, &lpMessageHandlerOutput, &dwMessageHandlerOutputSize
			);
			dwErrorCode = fpMessageHandler(lpMessageHandlerInput, dwMessageHandlerInputSize, &lpMessageHandlerOutput, &dwMessageHandlerOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwErrorCode:                %08x"), dwErrorCode);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwMessageHandlerOutputSize: %d"), dwMessageHandlerOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpMessageHandlerOutput:     %p"), lpMessageHandlerOutput);
			printf("[%s(%d)] lpMessageHandlerOutput:\n%s\n", __FILE__, __LINE__, (CHAR*)lpMessageHandlerOutput);


			// Free the input
			if (NULL != lpMessageHandlerInput)
			{
				cJSON_free(lpMessageHandlerInput);
				lpMessageHandlerInput = NULL;
			}


			// Create the MessageHandler response buffer
			lpJsonManagerBasicDispatchResponse = cJSON_CreateBytes(lpMessageHandlerOutput, dwMessageHandlerOutputSize);
			if (NULL == lpJsonManagerBasicDispatchResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicDispatchResponse = cJSON_CreateBytes(%p, %d) failed.\n"),
					lpMessageHandlerOutput, dwMessageHandlerOutputSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonManagerBasicOutput, TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE, lpJsonManagerBasicDispatchResponse);


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szManagerBasicOutput = cJSON_Print(lpJsonManagerBasicOutput);
			if (NULL == szManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicOutput
				);
				__leave;
			}
			dwManagerBasicOutputSize = (DWORD)strlen(szManagerBasicOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szManagerBasicOutput;
			(*lpdwOutputBufferSize) = dwManagerBasicOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szManagerBasicOutput);
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




DWORD WINAPI ManagerBasicCallbackListHandlers(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szManagerBasicInput = NULL;
	DWORD	dwManagerBasicInputSize = 0;
	cJSON*	lpJsonManagerBasicInput = NULL;

	LPBYTE	lpMessageHandlerInput = NULL;
	DWORD	dwMessageHandlerInputSize = 0;
	LPBYTE	lpMessageHandlerOutput = NULL;
	DWORD	dwMessageHandlerOutputSize = 0;

	CHAR*	szManagerBasicOutput = NULL;
	DWORD	dwManagerBasicOutputSize = 0;
	cJSON*	lpJsonManagerBasicOutput = NULL;
	cJSON*	lpJsonManagerBasicOutputHandlers = NULL;


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
			//if (NULL == lpInputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpInputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//szManagerBasicInput = (char*)lpInputBuffer;
			//dwManagerBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			//printf("[%s(%d)] szManagerBasicInput:\n%s\n", __FILE__, __LINE__, szManagerBasicInput);
			//lpJsonManagerBasicInput = cJSON_Parse(szManagerBasicInput);
			//if (NULL == lpJsonManagerBasicInput)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Parse(%p) failed."),
			//		szManagerBasicInput
			//	);
			//	__leave;
			//}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonManagerBasicOutput = cJSON_CreateObject();
			if (NULL == lpJsonManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicOutput = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}

			lpJsonManagerBasicOutputHandlers = cJSON_AddArrayToObject(lpJsonManagerBasicOutput, TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS);
			if (NULL == lpJsonManagerBasicOutputHandlers)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerBasicOutputHandlers = cJSON_CreateArray() failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			for (PMESSAGE_HANDLER_NODE pCurrentHandler = g_ManagerBasic_lpMessageHandlerList; pCurrentHandler != NULL; pCurrentHandler = pCurrentHandler->lpNext)
			{
				cJSON* lpJsonCurrentMessageHandlerType = NULL;
				
				DBGPRINT(DEBUG_VERBOSE, TEXT("pCurrentHandler->szMessageHandlerType:     %S"), pCurrentHandler->szMessageHandlerType);

				lpJsonCurrentMessageHandlerType = cJSON_CreateString(pCurrentHandler->szMessageHandlerType);
				if (NULL == lpJsonCurrentMessageHandlerType)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCurrentMessageHandlerType = cJSON_CreateString(%S) failed.\n"),
						pCurrentHandler->szMessageHandlerType
					);
					__leave;
				}

				cJSON_AddItemToArray(lpJsonManagerBasicOutputHandlers, lpJsonCurrentMessageHandlerType);
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szManagerBasicOutput = cJSON_Print(lpJsonManagerBasicOutput);
			if (NULL == szManagerBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonManagerBasicOutput
				);
				__leave;
			}
			dwManagerBasicOutputSize = (DWORD)strlen(szManagerBasicOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szManagerBasicOutput;
			(*lpdwOutputBufferSize) = dwManagerBasicOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szManagerBasicOutput);
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