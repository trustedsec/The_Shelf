#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_LoaderMemoryModule.h"
#include "LIB_LoaderMemoryModule.h"

// Primary TsunamiWave Configuration
CHAR g_LoaderMemoryModule_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_LoaderMemoryModule_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


#define DEFAULT_FUNC_NAME	"DLL_TEMPLATE_ExportedFunction"
#define DEFAULT_DLL_NAME	"test.dll"
typedef DWORD(__cdecl* f_DLL_TEMPLATE_ExportedFunction)(void);


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

	cJSON*	lpJsonCallProcRequest = NULL;
	cJSON*	lpJsonCallProcRequestFunctionName = NULL;
	cJSON*	lpJsonCallProcRequestMemoryModule = NULL;
	cJSON*	lpJsonCallProcRequestBufferArgument = NULL;
	cJSON*	lpJsonCallProcRequestSizeArgument = NULL;
	cJSON*	lpJsonCallProcResponseFunctionReturn = NULL;
	
	cJSON*	lpJsonFreeLibraryRequest = NULL;
	cJSON*	lpJsonFreeLibraryRequestMemoryModule = NULL;
	cJSON*	lpJsonFreeLibraryResponseResults = NULL;

	CHAR	szFilename[MAX_PATH];
	LPBYTE	lpLibraryContents = NULL;
	DWORD	dwLibraryContentsSize = 0;

	CHAR	szFunctionName[MAX_PATH];
	CHAR	szBufferArgument[MAX_PATH];
	DWORD	dwSizeArgument = 0;

	HANDLE	hMemoryModule = NULL;
	FARPROC	fpProcAddress = NULL;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_LoaderMemoryModule wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			ZeroMemory(szFilename, MAX_PATH);
			if (1 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
				wcstombs(szFilename, argv[1], MAX_PATH);
			}
			else
			{
				strcpy_s(szFilename, MAX_PATH, DEFAULT_DLL_NAME);
			}
			ZeroMemory(szFunctionName, MAX_PATH);
			if (2 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[2]: %s"), argv[2]);
				wcstombs(szFunctionName, argv[2], MAX_PATH);
			}
			else
			{
				strcpy_s(szFunctionName, MAX_PATH, DEFAULT_FUNC_NAME);
			}
			ZeroMemory(szBufferArgument, MAX_PATH);
			if ( 3 < argc )
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[3]: %s"), argv[3]);
				wcstombs(szBufferArgument, argv[3], MAX_PATH);
			}
			else
			{
				strcpy_s(szBufferArgument, MAX_PATH, "");
			}
			if (4 < argc)
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("argv[4]: %s"), argv[4]);
				dwSizeArgument = _wtoi( argv[4] );
			}
			else
			{
				dwSizeArgument = 1;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFilename:       %S"), szFilename);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFunctionName:   %S"), szFunctionName);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szBufferArgument: %S"), szBufferArgument);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSizeArgument:   %d"), dwSizeArgument);


			/********************************************************************************************
				Read DLL into buffer
			********************************************************************************************/
			dwErrorCode = ReadFileIntoBuffer(
				szFilename,
				&lpLibraryContents,
				&dwLibraryContentsSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer( %d, %p ) failed.(%08x)"),
					szFilename,
					&lpLibraryContents,
					&dwLibraryContentsSize,
					dwErrorCode);
				__leave;
			}

			
			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_LoaderMemoryModule_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_LoaderMemoryModule_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_LoaderMemoryModule_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_LoaderMemoryModule_szPackedConfigurationPassword);

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
				Initialize the LoaderMemoryModule plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the LoaderMemoryModule plugin"));
			dwErrorCode = LoaderMemoryModuleInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
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
			//printf("szLoaderMemoryModuleInput: \n%s\n", szLoaderMemoryModuleInput);

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
			//printf("szLoaderMemoryModuleOutput:\n%s\n", szLoaderMemoryModuleOutput);

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



			if (3 > argc)
			{

				/***************************************************************************
					Use the LoaderMemoryModuleGetProc
				***************************************************************************/
				DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleGetProc"));

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
				lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(szFunctionName);
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
				//printf("szLoaderMemoryModuleInput: \n%s\n", szLoaderMemoryModuleInput);

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
				//printf("szLoaderMemoryModuleOutput:\n%s\n", szLoaderMemoryModuleOutput);

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
				fpProcAddress = *((FARPROC*)pBuffer);

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
					Use the fpProcAddress
				***************************************************************************/
				DBGPRINT(DEBUG_INFO, TEXT("Use the fpProcAddress"));

				((f_DLL_TEMPLATE_ExportedFunction)fpProcAddress)();
			}
			else // Argc > 3
			{
				/***************************************************************************
					Use the LoaderMemoryModuleCallProc
				***************************************************************************/
				DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleCallProc"));

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

				// Create the LoaderMemoryModule_CallProc_Request JSON object
				lpJsonCallProcRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_CallProc_Request);
				if (NULL == lpJsonCallProcRequest)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCallProcRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
						lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_CallProc_Request
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonCallProcRequest:    %p"), lpJsonCallProcRequest);

				// Create the LoaderMemoryModule_CallProc_FunctionName input JSON object
				lpJsonCallProcRequestFunctionName = cJSON_CreateString(szFunctionName);
				if (NULL == lpJsonCallProcRequestFunctionName)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCallProcRequestFunctionName = cJSON_CreateString(%S) failed."),
						szFunctionName
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCallProcRequest, LoaderMemoryModule_CallProc_FunctionName, lpJsonCallProcRequestFunctionName);

				// Create the LoaderMemoryModule_CallProc_ModuleHandle input JSON object
				lpJsonCallProcRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hMemoryModule), sizeof(HMODULE));
				if (NULL == lpJsonCallProcRequestMemoryModule)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCallProcRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
						(LPBYTE)(&hMemoryModule),
						sizeof(HMODULE)
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCallProcRequest, LoaderMemoryModule_CallProc_ModuleHandle, lpJsonCallProcRequestMemoryModule);

				// Create the LoaderMemoryModule_CallProc_BufferArgument input JSON object
				lpJsonCallProcRequestBufferArgument = cJSON_CreateBytes((LPBYTE)(&szBufferArgument), (int)strlen(szBufferArgument));
				if (NULL == lpJsonCallProcRequestBufferArgument)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCallProcRequestBufferArgument = cJSON_CreateBytes(%p,%d) failed."),
						(LPBYTE)(&szBufferArgument), strlen(szBufferArgument)
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCallProcRequest, LoaderMemoryModule_CallProc_BufferArgument, lpJsonCallProcRequestBufferArgument);

				// Create the LoaderMemoryModule_CallProc_SizeArgument input JSON object
				lpJsonCallProcRequestSizeArgument = cJSON_CreateNumber(dwSizeArgument);
				if (NULL == lpJsonCallProcRequestSizeArgument)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCallProcRequestSizeArgument = cJSON_CreateNumber(%d) failed."),
						dwSizeArgument
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCallProcRequest, LoaderMemoryModule_CallProc_SizeArgument, lpJsonCallProcRequestSizeArgument);

				// Create the string representation of the LoaderMemoryModule_CallProc_Request
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
				//printf("szLoaderMemoryModuleInput: \n%s\n", szLoaderMemoryModuleInput);

				// Call the LoaderMemoryModuleCallProc passing in the JSON input object and receiving the JSON output object
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("LoaderMemoryModuleCallProc(%p, %d, %p, %p)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize
				);
				dwErrorCode = LoaderMemoryModuleCallProc(
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize
				);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("LoaderMemoryModuleCallProc(%p, %d, %p, %p) failed.(%08x)"),
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
				//printf("szLoaderMemoryModuleOutput:\n%s\n", szLoaderMemoryModuleOutput);

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

				// Get and display the LoaderMemoryModule_CallProc_FunctionReturn
				lpJsonCallProcResponseFunctionReturn = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_CallProc_FunctionReturn);
				if (NULL == lpJsonCallProcResponseFunctionReturn)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonCallProcResponseFunctionReturn = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_CallProc_FunctionReturn
					);
					__leave;
				}
				if (!cJSON_IsNumber(lpJsonCallProcResponseFunctionReturn))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonCallProcResponseFunctionReturn
					);
					__leave;
				}
				dwTempErrorCode = lpJsonCallProcResponseFunctionReturn->valueint;

				DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %d"), LoaderMemoryModule_CallProc_FunctionReturn, dwTempErrorCode);

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
			} // end else use CallProc

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
			//printf("szLoaderMemoryModuleInput: \n%s\n", szLoaderMemoryModuleInput);

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
			//printf("szLoaderMemoryModuleOutput:\n%s\n", szLoaderMemoryModuleOutput);

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
			if ( !cJSON_IsBool(lpJsonFreeLibraryResponseResults) )
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


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_LoaderMemoryModule wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
