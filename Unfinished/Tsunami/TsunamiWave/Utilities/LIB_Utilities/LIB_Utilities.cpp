#include "LIB_Utilities.h"
#include <stdio.h>
#include <wincrypt.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
//#define DEBUG_LEVEL DEBUG_VERBOSE
#define DEBUG_LEVEL DEBUG_INFO
#include "DebugOutput.h"




LPCSTR	__szMixTextKey = "{8DED3F71-05BD-48F6-B729-7457A0E137B9}";



LPSTR __MixTextDecrypt(LPCSTR szInputString, LPSTR szOutputString, DWORD dwOutputStringSize)
{
	DWORD	dwKeyIndex = 0;
	DWORD	dwInputIndex = 0;
	CHAR	cTemp = 0;
	char*	szDecryptedString = szOutputString;
	DWORD	dwInputBufferSize = 0;
	LPBYTE	lpInputBuffer = NULL;
	LPBYTE  lpRollingByte = NULL;

	//printf("strlen(__szMixTextKey): %d\n", strlen(__szMixTextKey));
	//printf("__szMixTextKey: %s\n", __szMixTextKey);
	//printf("__szMixTextKey: ");
	//for (DWORD i = 0; __szMixTextKey[i]!=0; i++)
	//	printf("%02x ", __szMixTextKey[i]);
	//printf("\n");


	// Zero out the global decrypted string buffer
	memset(szDecryptedString, 0, dwOutputStringSize);

	// Get size of the input string
	lpInputBuffer = (LPBYTE)szInputString;
	memcpy(&dwInputBufferSize, lpInputBuffer, sizeof(DWORD));
	lpInputBuffer = lpInputBuffer + sizeof(DWORD) + 1; // skip a byte for the random rolling xor seed
	lpRollingByte = lpInputBuffer - 1; // point to the random rolling xor seed
	

	//printf("dwInputBufferSize: %d (%08x)\n", dwInputBufferSize, dwInputBufferSize);
	//printf("lpRollingByte: %02x\n", lpRollingByte[0]);
	//printf("lpInputBuffer: ");
	//for (DWORD i = 0; i < dwInputBufferSize; i++)
	//	printf("%02x ", lpInputBuffer[i]);
	//printf("\n");

	// Loop through each character in the index and decrypt with the key
	for (dwInputIndex = 0; dwInputIndex< dwInputBufferSize; dwInputIndex++)
	{
		// Xor the string with the key
		//printf("i %d: in:%02x key:%02x rolling:%02x\n", dwInputIndex, lpInputBuffer[dwInputIndex], __szMixTextKey[dwKeyIndex], lpRollingByte[dwInputIndex]);
		cTemp = lpInputBuffer[dwInputIndex] ^ __szMixTextKey[dwKeyIndex] ^ lpRollingByte[dwInputIndex];
		//printf("output_char: %02x\n", cTemp);

		szDecryptedString[dwInputIndex] = cTemp;

		// Iterate through the mix key
		dwKeyIndex++;
		if (0 == __szMixTextKey[dwKeyIndex])
			dwKeyIndex = 0;
	}

	// Make sure there is a null terminator
	szDecryptedString[dwInputIndex] = '\0'; //

	//printf("szDecryptedString: ");
	//for (DWORD i = 0; szDecryptedString[i] != 0; i++)
	//	printf("%02x ", szDecryptedString[i]);
	//printf("\n");

	// Return a pointer to the decrypted mix text buffer
	return szDecryptedString;
}




BOOL RegisterModuleMessageHandler(
	const cJSON* lpJsonConfiguration, 
	const CHAR* szMessageHandlerType, 
	const f_CallbackInterface fpMessageHandler
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	BOOL	bReturn = FALSE;
	
	cJSON*	lpJsonRegister = NULL;
	f_CallbackInterface fpRegisterFunction = NULL;
	
	cJSON*	lpJsonRegisterInput = NULL;
	cJSON*	lpJsonManagerRegisterRequest = NULL;
	CHAR*	szRegisterInput = NULL;
	DWORD	dwRegisterInputSize = 0;
	
	CHAR*	szRegisterOutput = NULL;
	DWORD	dwRegisterOutputSize = 0;
	cJSON*	lpJsonRegisterOutput = NULL;
	cJSON*	lpJsonManagerRegisterResponse = NULL;
	cJSON*	lpJsonManagerRegisterResponseResult = NULL;
		
	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;
	
		
	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonConfiguration:  %p"), lpJsonConfiguration);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szMessageHandlerType: %p"), szMessageHandlerType);
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpMessageHandler:     %p"), fpMessageHandler);
	
			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (NULL == lpJsonConfiguration)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonConfiguration is NULL.\n")
				);
				__leave;
			}
			if (NULL == szMessageHandlerType)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szMessageHandlerType is NULL.\n")
				);
				__leave;
			}
			if (NULL == fpMessageHandler)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("fpMessageHandler is NULL.\n")
				);
				__leave;
			}
	
	
			/********************************************************************************
				Get the configuration's registration function
			********************************************************************************/
			lpJsonRegister = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_REGISTER);
			if (NULL == lpJsonRegister)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonRegister = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration, TSM_CONFIGURATION_REGISTER
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
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %p"), TSM_CONFIGURATION_REGISTER, pBuffer);
			fpRegisterFunction = *((f_CallbackInterface*)pBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %p"), TSM_CONFIGURATION_REGISTER, fpRegisterFunction);
			// Free the byte buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;
	
	
			/***************************************************************************
				Create the TSM_CONFIGURATION_REGISTER input JSON
			***************************************************************************/
			// Create the input JSON object
			lpJsonRegisterInput = cJSON_CreateObject();
			if (NULL == lpJsonRegisterInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonRegisterInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}
	
			// Create the TSM_MANAGER_REGISTER_REQUEST JSON object
			lpJsonManagerRegisterRequest = cJSON_AddObjectToObject(lpJsonRegisterInput, TSM_MANAGER_REGISTER_REQUEST);
			if (NULL == lpJsonManagerRegisterRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerRegisterRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonRegisterInput, TSM_MANAGER_REGISTER_REQUEST
				);
				__leave;
			}
	
			// Add the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			if (NULL == cJSON_AddStringToObject(lpJsonManagerRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddStringToObject(%p, %S, %p) failed.\n"),
					lpJsonManagerRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType
				);
				__leave;
			}
	
			// Add the TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER
			if (
				NULL == cJSON_AddBytesToObject(
					lpJsonManagerRegisterRequest, 
					TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER, 
					(LPBYTE)(&fpMessageHandler), 
					sizeof(fpMessageHandler)
				)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddBytesToObject(%p, %S, %d) failed.\n"),
					lpJsonManagerRegisterRequest, TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER, (LPBYTE)fpMessageHandler, sizeof(fpMessageHandler)
				);
				__leave;
			}
				
	
			/********************************************************************************
				Create the TSM_CONFIGURATION_REGISTER input string
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create registration function input message..."));
			szRegisterInput = cJSON_Print(lpJsonRegisterInput);
			if (NULL == szRegisterInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonRegisterInput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szRegisterInput:     %p"), szRegisterInput);
			//printf("szRegisterInput:\n%s\n", szRegisterInput);
			dwRegisterInputSize = (DWORD)strlen(szRegisterInput);
	
	
			/***************************************************************************
				Call the TSM_CONFIGURATION_REGISTER function
			***************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpRegisterFunction:     %p"), fpRegisterFunction);
			DBGPRINT(
				DEBUG_VERBOSE, 
				TEXT("fpRegisterFunction( %p, %d, %p, %p )..."),
				(LPBYTE)szRegisterInput,
				dwRegisterInputSize,
				(LPBYTE*)(&szRegisterOutput),
				&dwRegisterOutputSize
			);
			dwErrorCode = fpRegisterFunction(
				(LPBYTE)szRegisterInput,
				dwRegisterInputSize,
				(LPBYTE*)(&szRegisterOutput),
				&dwRegisterOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("fpRegisterFunction( %p, %d, %p, %p ) failed.(%08x)"),
					(LPBYTE)szRegisterInput,
					dwRegisterInputSize,
					(LPBYTE*)(&szRegisterOutput),
					&dwRegisterOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpRegisterFunction() was successful"));
	
	
			/***************************************************************************
				Check the TSM_CONFIGURATION_REGISTER function output
			***************************************************************************/
			// Parse the output string into a JSON object
			lpJsonRegisterOutput = cJSON_Parse(szRegisterOutput);
			if (NULL == lpJsonRegisterOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonRegisterOutput = cJSON_Parse(%p) failed."),
					szRegisterOutput
				);
				__leave;
			}
	
			// Get the TSM_MANAGER_REGISTER_RESPONSE JSON object
			lpJsonManagerRegisterResponse = cJSON_GetObjectItemCaseSensitive(lpJsonRegisterOutput, TSM_MANAGER_REGISTER_RESPONSE);
			if (NULL == lpJsonManagerRegisterResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerRegisterResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonRegisterOutput, TSM_MANAGER_REGISTER_RESPONSE
				);
				__leave;
			}
	
			// Get the TSM_MANAGER_REGISTER_RESPONSE result
			lpJsonManagerRegisterResponseResult = cJSON_GetObjectItemCaseSensitive(lpJsonManagerRegisterResponse, TSM_MANAGER_REGISTER_RESPONSE_RESULT);
			if (NULL == lpJsonManagerRegisterResponseResult)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerRegisterResponseResult = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerRegisterResponse, TSM_MANAGER_REGISTER_RESPONSE_RESULT
				);
				__leave;
			}
				
			// Check if the current result type
			if ( !cJSON_IsBool(lpJsonManagerRegisterResponseResult)	)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBool(%p) failed."),
					lpJsonManagerRegisterResponseResult
				);
				bReturn = FALSE;
				__leave;
			}
	
			if (cJSON_IsFalse(lpJsonManagerRegisterResponseResult))
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("\"%S\": false"), TSM_MANAGER_REGISTER_RESPONSE_RESULT);
				bReturn = FALSE;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("\"%S\": true"), TSM_MANAGER_REGISTER_RESPONSE_RESULT);
				bReturn = TRUE;
			}
	
	
		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonRegisterInput)
			{
				cJSON_Delete(lpJsonRegisterInput);
				lpJsonRegisterInput = NULL;
			}
			if (NULL != lpJsonRegisterOutput)
			{
				cJSON_Delete(lpJsonRegisterOutput);
				lpJsonRegisterOutput = NULL;
			}
			// Free input JSON strings
			if (NULL != szRegisterInput)
			{
				cJSON_free(szRegisterInput);
				szRegisterInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szRegisterOutput)
			{
				cJSON_free(szRegisterOutput);
				szRegisterOutput = NULL;
			}
	
			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except
	
	return bReturn;
}
	
	
BOOL UnregisterModuleMessageHandler(
	const cJSON* lpJsonConfiguration, 
	const CHAR* szMessageHandlerType
)
	
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	BOOL	bReturn = FALSE;
	
	cJSON*	lpJsonUnregister = NULL;
	f_CallbackInterface fpUnregisterFunction = NULL;
	
	cJSON*	lpJsonUnregisterInput = NULL;
	cJSON*	lpJsonManagerUnregisterRequest = NULL;
	CHAR*	szUnregisterInput = NULL;
	DWORD	dwUnregisterInputSize = 0;
	
	CHAR*	szUnregisterOutput = NULL;
	DWORD	dwUnregisterOutputSize = 0;
	cJSON*	lpJsonUnregisterOutput = NULL;
	cJSON*	lpJsonManagerUnregisterResponse = NULL;
	cJSON*	lpJsonManagerUnregisterResponseResult = NULL;
	
	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;
	
	
	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonConfiguration:  %p"), lpJsonConfiguration);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szMessageHandlerType: %p"), szMessageHandlerType);
	
			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (NULL == lpJsonConfiguration)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonConfiguration is NULL.\n")
				);
				__leave;
			}
			if (NULL == szMessageHandlerType)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szMessageHandlerType is NULL.\n")
				);
				__leave;
			}
	
	
			/********************************************************************************
				Get the configuration's registration function
			********************************************************************************/
			lpJsonUnregister = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_UNREGISTER);
			if (NULL == lpJsonUnregister)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonUnregister = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration, TSM_CONFIGURATION_UNREGISTER
				);
				__leave;
			}
			// Check the TSM_CONFIGURATION_UNREGISTER JSON type
			if (
				!cJSON_IsBytes(lpJsonUnregister) ||
				(NULL == lpJsonUnregister->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonUnregister
				);
				__leave;
			}
			// Get the TSM_CONFIGURATION_UNREGISTER byte buffer
			pBuffer = cJSON_GetBytesValue(lpJsonUnregister, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonUnregister,
					&nBufferSize
				);
				__leave;
			}
			// Convert the byte buffer into the HandlerFunction
			fpUnregisterFunction = *((f_CallbackInterface*)pBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %p"), TSM_CONFIGURATION_UNREGISTER, fpUnregisterFunction);
			// Free the byte buffer
			cJSON_free(pBuffer);
			pBuffer = NULL;
	
	
	
			/***************************************************************************
				Create the TSM_CONFIGURATION_UNREGISTER input JSON
			***************************************************************************/
			// Create the input JSON object
			lpJsonUnregisterInput = cJSON_CreateObject();
			if (NULL == lpJsonUnregisterInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonUnregisterInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}
	
			// Create the TSM_MANAGER_REGISTER_REQUEST JSON object
			lpJsonManagerUnregisterRequest = cJSON_AddObjectToObject(lpJsonUnregisterInput, TSM_MANAGER_UNREGISTER_REQUEST);
			if (NULL == lpJsonManagerUnregisterRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerUnregisterRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonUnregisterInput, TSM_MANAGER_UNREGISTER_REQUEST
				);
				__leave;
			}
	
			// Add the TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE
			if (NULL == cJSON_AddStringToObject(lpJsonManagerUnregisterRequest, TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddStringToObject(%p, %S, %p) failed.\n"),
					lpJsonManagerUnregisterRequest, TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE, szMessageHandlerType
				);
				__leave;
			}
	
				
			/********************************************************************************
				Create the TSM_CONFIGURATION_UNREGISTER input string
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create registration function input message..."));
			szUnregisterInput = cJSON_Print(lpJsonUnregisterInput);
			if (NULL == szUnregisterInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonUnregisterInput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szUnregisterInput:     %p"), szUnregisterInput);
			//printf("szUnregisterInput:\n%s\n", szUnregisterInput);
			dwUnregisterInputSize = (DWORD)strlen(szUnregisterInput);
	
			/***************************************************************************
				Send message to the TSM_CONFIGURATION_UNREGISTER function
			***************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpUnregisterCallback()..."));
			dwErrorCode = fpUnregisterFunction(
				(LPBYTE)szUnregisterInput,
				dwUnregisterInputSize,
				(LPBYTE*)(&szUnregisterOutput),
				&dwUnregisterOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("fpUnregisterCallback( %p, %d, %p, %p ) failed.(%08x)"),
					(LPBYTE)szUnregisterInput,
					dwUnregisterInputSize,
					(LPBYTE*)(&szUnregisterOutput),
					&dwUnregisterOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpUnregisterCallback() was successful"));
	
	
			/***************************************************************************
				Check the TSM_CONFIGURATION_UNREGISTER function output
			***************************************************************************/
			// Parse the output string into a JSON object
			lpJsonUnregisterOutput = cJSON_Parse(szUnregisterOutput);
			if (NULL == lpJsonUnregisterOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonUnregisterOutput = cJSON_Parse(%p) failed."),
					szUnregisterOutput
				);
				__leave;
			}
	
			// Get the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			lpJsonManagerUnregisterResponse = cJSON_GetObjectItemCaseSensitive(lpJsonUnregisterOutput, TSM_MANAGER_UNREGISTER_RESPONSE);
			if (NULL == lpJsonManagerUnregisterResponse)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerUnregisterResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonUnregisterOutput,	TSM_MANAGER_UNREGISTER_RESPONSE
				);
				__leave;
			}
	
			// Get the TSM_MANAGER_UNREGISTER_RESPONSE result
			lpJsonManagerUnregisterResponseResult = cJSON_GetObjectItemCaseSensitive(lpJsonManagerUnregisterResponse, TSM_MANAGER_UNREGISTER_RESPONSE_RESULT);
			if (NULL == lpJsonManagerUnregisterResponseResult)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonManagerUnregisterResponseResult = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonManagerUnregisterResponse,
					TSM_MANAGER_UNREGISTER_RESPONSE_RESULT
				);
				__leave;
			}
	
			// Check if the current result type
			if (!cJSON_IsBool(lpJsonManagerUnregisterResponseResult))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBool(%p) failed."),
					lpJsonManagerUnregisterResponseResult
				);
				bReturn = FALSE;
				__leave;
			}
	
			if (cJSON_IsFalse(lpJsonManagerUnregisterResponseResult))
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("\"%S\": false"), TSM_MANAGER_UNREGISTER_RESPONSE_RESULT);
				bReturn = FALSE;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("\"%S\": true"), TSM_MANAGER_UNREGISTER_RESPONSE_RESULT);
				bReturn = TRUE;
			}
	
		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonUnregisterInput)
			{
				cJSON_Delete(lpJsonUnregisterInput);
				lpJsonUnregisterInput = NULL;
			}
			if (NULL != lpJsonUnregisterOutput)
			{
				cJSON_Delete(lpJsonUnregisterOutput);
				lpJsonUnregisterOutput = NULL;
			}
			// Free input JSON strings
			if (NULL != szUnregisterInput)
			{
				cJSON_free(szUnregisterInput);
				szUnregisterInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szUnregisterOutput)
			{
				cJSON_free(szUnregisterOutput);
				szUnregisterOutput = NULL;
			}
	
			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except
	
	return bReturn;
}









HMODULE GetCurrentModule()
{ // NB: XP+ solution!
	HMODULE hModule = NULL;
	GetModuleHandleEx(
		GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
		(LPCTSTR)GetCurrentModule,
		&hModule
	);

	return hModule;
}



#define AES_BLOCK_LEN 128

#define COMPRESSION_FORMAT_LZNT1		(0x0002)
#define COMPRESSION_ENGINE_STANDARD		(0x0000)
#define COMPRESSION_ENGINE_MAXIMUM		(0x0100)
#define COMPRESSION_CHUNK_SIZE			4096
#define STATUS_BAD_COMPRESSION_BUFFER	(0xC0000242)

typedef NTSTATUS(NTAPI* _RtlCompressBuffer)
(
	IN	ULONG	CompressionFormatAndEngine,
	IN	PVOID	UncompressedBuffer,
	IN	ULONG	UncompressedBufferLength,
	OUT	PVOID	CompressedBuffer,
	IN	ULONG	CompressedBufferLength,
	IN	ULONG	UncompressedChunkSize,
	OUT	PULONG	FinalCompressedSize,
	IN	PVOID	WorkspaceBuffer
	);

typedef NTSTATUS(NTAPI* _RtlDecompressBuffer)
(
	IN	ULONG	CompressionFormat,
	OUT PVOID	UncompressedBuffer,
	IN	ULONG	UncompressedBufferLength,
	IN	PVOID	CompressedBuffer,
	IN	ULONG	CompressedBufferLength,
	OUT PULONG	FinalUncompressedSize
	);

typedef NTSTATUS(NTAPI* _RtlGetCompressionWorkSpaceSize)
(
	IN	USHORT	CompressionFormatAndEngine,
	OUT PULONG	CompressBufferWorkSpaceSize,
	OUT PULONG	CompressFragmentWorkSpaceSize
	);


_RtlCompressBuffer RtlCompressBuffer = NULL;
_RtlDecompressBuffer RtlDecompressBuffer = NULL;
_RtlGetCompressionWorkSpaceSize RtlGetCompressionWorkSpaceSize = NULL;
HMODULE HMODULE_NTDLL = NULL;


DWORD InitializeNtDllFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == HMODULE_NTDLL)
			{
				HMODULE_NTDLL = LoadLibraryA(
					"ntdll"
				);
				if (NULL == HMODULE_NTDLL)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("HMODULE_NTDLL:                  %p"), HMODULE_NTDLL);

				fpFunctionPointer = GetProcAddress(
					HMODULE_NTDLL,
					"RtlCompressBuffer"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				RtlCompressBuffer = (_RtlCompressBuffer)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("RtlCompressBuffer:              %p"), RtlCompressBuffer);

				fpFunctionPointer = GetProcAddress(
					HMODULE_NTDLL,
					"RtlDecompressBuffer"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				RtlDecompressBuffer = (_RtlDecompressBuffer)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("RtlDecompressBuffer:            %p"), RtlDecompressBuffer);

				fpFunctionPointer = GetProcAddress(
					HMODULE_NTDLL,
					"RtlGetCompressionWorkSpaceSize"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				RtlGetCompressionWorkSpaceSize = (_RtlGetCompressionWorkSpaceSize)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("RtlGetCompressionWorkSpaceSize: %p"), RtlGetCompressionWorkSpaceSize);
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD FinalizeNtDllFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != HMODULE_NTDLL)
			{
				FreeLibrary(HMODULE_NTDLL);
				HMODULE_NTDLL = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}



typedef BOOL (WINAPI* __CryptAcquireContextW)
(
	OUT		HCRYPTPROV  *phProv,
	IN OPTIONAL    LPCWSTR    szContainer,
	IN OPTIONAL    LPCWSTR    szProvider,
	IN		DWORD       dwProvType,
	IN		DWORD       dwFlags
	);
typedef BOOL (WINAPI* __CryptCreateHash)
(
	IN		HCRYPTPROV  hProv,
	IN		ALG_ID      Algid,
	IN		HCRYPTKEY   hKey,
	IN		DWORD       dwFlags,
	OUT		HCRYPTHASH  *phHash
	);
typedef BOOL(WINAPI* __CryptHashData)
(
	IN		HCRYPTHASH  hHash,
	IN		CONST BYTE  *pbData,
	IN		DWORD   dwDataLen,
	IN		DWORD   dwFlags
	);
typedef BOOL(WINAPI* __CryptDeriveKey)
(
	IN		HCRYPTPROV  hProv,
	IN		ALG_ID      Algid,
	IN		HCRYPTHASH  hBaseData,
	IN		DWORD       dwFlags,
	OUT		HCRYPTKEY   *phKey
	);
typedef BOOL(WINAPI* __CryptDecrypt)
(
	IN		HCRYPTKEY   hKey,
	IN		HCRYPTHASH  hHash,
	IN		BOOL        Final,
	IN		DWORD       dwFlags,
	IN OUT	BYTE        *pbData,
	IN OUT	DWORD       *pdwDataLen
	);
typedef BOOL(WINAPI* __CryptEncrypt)
(
	IN		HCRYPTKEY   hKey,
	IN		HCRYPTHASH  hHash,
	IN		BOOL    Final,
	IN		DWORD   dwFlags,
	IN OUT	BYTE    *pbData,
	IN OUT	DWORD   *pdwDataLen,
	IN		DWORD   dwBufLen
	);
typedef BOOL(WINAPI* __CryptReleaseContext)
(
	IN    HCRYPTPROV  hProv,
	IN    DWORD       dwFlags
	);
typedef BOOL(WINAPI* __CryptDestroyKey)
(
	IN    HCRYPTKEY   hKey
	);
typedef BOOL(WINAPI* __CryptDestroyHash)
(
	IN    HCRYPTHASH  hHash
	);


__CryptAcquireContextW _CryptAcquireContextW = NULL;
__CryptCreateHash _CryptCreateHash = NULL;
__CryptHashData _CryptHashData = NULL;
__CryptDeriveKey _CryptDeriveKey = NULL;
__CryptDecrypt _CryptDecrypt = NULL;
__CryptEncrypt _CryptEncrypt = NULL;
__CryptReleaseContext _CryptReleaseContext = NULL;
__CryptDestroyKey _CryptDestroyKey = NULL;
__CryptDestroyHash _CryptDestroyHash = NULL;
HMODULE HMODULE_ADVAPI32 = NULL;



DWORD InitializeAdvapiFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == HMODULE_ADVAPI32)
			{
				HMODULE_ADVAPI32 = LoadLibraryA(
					"Advapi32.dll"
				);
				if (NULL == HMODULE_ADVAPI32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("HMODULE_ADVAPI32: %p"), HMODULE_ADVAPI32);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptAcquireContextW"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptAcquireContextW = (__CryptAcquireContextW)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptAcquireContextW: %p"), _CryptAcquireContextW);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptCreateHash"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptCreateHash = (__CryptCreateHash)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptCreateHash: %p"), _CryptCreateHash);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptHashData"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptHashData = (__CryptHashData)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptHashData: %p"), _CryptHashData);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptDeriveKey"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptDeriveKey = (__CryptDeriveKey)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptDeriveKey: %p"), _CryptDeriveKey);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptDecrypt"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptDecrypt = (__CryptDecrypt)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptDecrypt: %p"), _CryptDecrypt);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptEncrypt"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptEncrypt = (__CryptEncrypt)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptEncrypt: %p"), _CryptEncrypt);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptReleaseContext"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptReleaseContext = (__CryptReleaseContext)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptReleaseContext: %p"), _CryptReleaseContext);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptDestroyKey"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptDestroyKey = (__CryptDestroyKey)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptDestroyKey: %p"), _CryptDestroyKey);

				fpFunctionPointer = GetProcAddress(
					HMODULE_ADVAPI32,
					"CryptDestroyHash"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptDestroyHash = (__CryptDestroyHash)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptDestroyHash: %p"), _CryptDestroyHash);

			}

		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD FinalizeAdvapiFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != HMODULE_ADVAPI32)
			{
				FreeLibrary(HMODULE_ADVAPI32);
				HMODULE_ADVAPI32 = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}



typedef BOOL(WINAPI* __CryptBinaryToStringA)
(
	IN CONST	BYTE *pbBinary,
	IN			DWORD cbBinary,
	IN			DWORD dwFlags,
	OUT			LPSTR pszString,
	IN OUT		DWORD *pcchString
	);
typedef BOOL(WINAPI* __CryptStringToBinaryA)
(
	IN			 LPCSTR pszString,
	IN			 DWORD cchString,
	IN			 DWORD dwFlags,
	OUT OPTIONAL BYTE *pbBinary,
	IN OUT		 DWORD  *pcbBinary,
	OUT OPTIONAL DWORD *pdwSkip,
	OUT OPTIONAL DWORD *pdwFlags
	);

__CryptBinaryToStringA _CryptBinaryToStringA = NULL;
__CryptStringToBinaryA _CryptStringToBinaryA = NULL;
HMODULE HMODULE_CRYPT32 = NULL;

DWORD InitializeCryptFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == HMODULE_CRYPT32)
			{
				HMODULE_CRYPT32 = LoadLibraryA(
					"Crypt32.dll"
				);
				if (NULL == HMODULE_CRYPT32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("HMODULE_CRYPT32: %p"), HMODULE_CRYPT32);

				fpFunctionPointer = GetProcAddress(
					HMODULE_CRYPT32,
					"CryptBinaryToStringA"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptBinaryToStringA = (__CryptBinaryToStringA)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptBinaryToStringA: %p"), _CryptBinaryToStringA);

				fpFunctionPointer = GetProcAddress(
					HMODULE_CRYPT32,
					"CryptStringToBinaryA"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				_CryptStringToBinaryA = (__CryptStringToBinaryA)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("_CryptStringToBinaryA: %p"), _CryptStringToBinaryA);
			}

		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD FinalizeCryptFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != HMODULE_CRYPT32)
			{
				FreeLibrary(HMODULE_CRYPT32);
				HMODULE_CRYPT32 = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}






DWORD ALLOC(IN DWORD dwSize, OUT LPVOID* lppBuffer)
{
	DWORD dwErrorCode = ERROR_SUCCESS;

	*lppBuffer = VirtualAllocEx(
		GetCurrentProcess(),
		NULL,
		(SIZE_T)dwSize,
		MEM_COMMIT | MEM_RESERVE,
		PAGE_READWRITE
	);
	if (NULL == *lppBuffer)
	{
		dwErrorCode = GetLastError();
	}

	return dwErrorCode;
}

DWORD FREE(IN LPVOID lpBuffer)
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	MEMORY_BASIC_INFORMATION mbi;
	ZeroMemory(&mbi, sizeof(mbi));
	if (0 == VirtualQueryEx(
		GetCurrentProcess(),
		lpBuffer,
		&mbi,
		sizeof(mbi)
	)
		)
	{
		dwErrorCode = GetLastError();
	}
	else
	{
		ZeroMemory(lpBuffer, mbi.RegionSize);
	}
	ZeroMemory(&mbi, sizeof(mbi));

	if (FALSE == VirtualFreeEx(
		GetCurrentProcess(),
		lpBuffer,
		0,
		MEM_RELEASE
	)
		)
	{
		dwErrorCode = GetLastError();
	}

	return dwErrorCode;
}

DWORD CompressBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	PBYTE	lpBuffer = NULL;
	DWORD	dwBufferSize = 0;


	DWORD	dwCompressBufferWorkSpaceSize = 0;
	DWORD	dwCompressFragmentWorkSpaceSize = 0;
	LPBYTE	lpWorkspaceBuffer = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);

			// Initialize/resolve the NT dll functions
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeNtDllFunctions()"));
			dwErrorCode = InitializeNtDllFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeNtDllFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Get the compression workspace size for the LZNT1 compression engine
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("RtlGetCompressionWorkSpaceSize( %04x, %p, %p)"),
				COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD,
				&dwCompressBufferWorkSpaceSize,
				&dwCompressFragmentWorkSpaceSize
			);
			dwErrorCode = RtlGetCompressionWorkSpaceSize(
				COMPRESSION_FORMAT_LZNT1,
				&dwCompressBufferWorkSpaceSize,
				&dwCompressFragmentWorkSpaceSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("RtlGetCompressionWorkSpaceSize failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCompressBufferWorkSpaceSize:     %08x"), dwCompressBufferWorkSpaceSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCompressFragmentWorkSpaceSize:   %08x"), dwCompressFragmentWorkSpaceSize);


			// Allocated a compression workspace buffer
			dwErrorCode = ALLOC(dwCompressBufferWorkSpaceSize, (LPVOID*)&lpWorkspaceBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpWorkspaceBuffer:     %p"), lpWorkspaceBuffer);


			// Allocate a buffer to hold the destination/compressed buffer
			dwBufferSize = 13 * dwSourceBufferSize;
			dwErrorCode = ALLOC(dwBufferSize, (LPVOID*)&lpBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpBuffer:     %p"), lpBuffer);


			// Attempt to compress the buffer using LZNT1
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("RtlCompressBuffer( %04x, %p, %08x, %p, %08x, %08x, %p, %p)"),
				COMPRESSION_FORMAT_LZNT1 | COMPRESSION_ENGINE_STANDARD,
				lpSourceBuffer,
				dwSourceBufferSize,
				lpBuffer,
				dwBufferSize,
				COMPRESSION_CHUNK_SIZE,
				lpdwDestinationBufferSize,
				lpWorkspaceBuffer
			);
			dwErrorCode = RtlCompressBuffer(
				COMPRESSION_FORMAT_LZNT1,
				lpSourceBuffer,
				dwSourceBufferSize,
				lpBuffer,
				dwBufferSize,
				COMPRESSION_CHUNK_SIZE,
				lpdwDestinationBufferSize,
				lpWorkspaceBuffer
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("RtlCompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Allocate a buffer exactly sized to hold the destination buffer
			dwErrorCode = ALLOC(*lpdwDestinationBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}


			CopyMemory((*lppDestinationBuffer), lpBuffer, *lpdwDestinationBufferSize);

		} // end try-finally
		__finally
		{
			if (NULL != lpWorkspaceBuffer)
			{
				dwTempErrorCode = FREE(lpWorkspaceBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpWorkspaceBuffer = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					lpBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}

			if (NULL != lpBuffer)
			{
				dwTempErrorCode = FREE(lpBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpBuffer = NULL;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD DecompressBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	PBYTE	lpBuffer = NULL;
	DWORD	dwBufferSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);

			// Initialize/resolve the NT dll functions
			dwErrorCode = InitializeNtDllFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeNtDllFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			dwBufferSize = dwSourceBufferSize;
			do
			{
				// Allocate a buffer to hold the destination/compressed buffer
				dwBufferSize = 4 * dwBufferSize;
				if (NULL != lpBuffer)
				{
					FREE(lpBuffer);
				}
				lpBuffer = NULL;
				dwErrorCode = ALLOC(dwBufferSize, (LPVOID*)&lpBuffer);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
					__leave;
				}

				// Attempt to compress the buffer using LZNT1
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("RtlDecompressBuffer( %04x, %p, %08x, %p, %08x, %p)"),
					COMPRESSION_FORMAT_LZNT1,
					lpBuffer,
					dwBufferSize,
					lpSourceBuffer,
					dwSourceBufferSize,
					lpdwDestinationBufferSize
				);
				dwErrorCode = RtlDecompressBuffer(
					COMPRESSION_FORMAT_LZNT1,
					lpBuffer,
					dwBufferSize,
					lpSourceBuffer,
					dwSourceBufferSize,
					lpdwDestinationBufferSize
				);
			} while (STATUS_BAD_COMPRESSION_BUFFER == dwErrorCode);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("RtlDecompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}

			// Allocate a buffer exactly sized to hold the destination buffer
			dwErrorCode = ALLOC(*lpdwDestinationBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}


			CopyMemory((*lppDestinationBuffer), lpBuffer, *lpdwDestinationBufferSize);

		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					lpBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD EncryptBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize, PBYTE lpPasswordBuffer, DWORD dwPasswordBufferSize, DWORD dwAlgorithmID)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	BYTE	lpBuffer[AES_BLOCK_LEN];
	//DWORD	dwBufferSize = AES_BLOCK_LEN;
	DWORD	dwBufferLength = 0;

	HCRYPTPROV	hProv = NULL;

	DWORD		dwFlags = 0;
	HCRYPTKEY	hKey = NULL;
	HCRYPTHASH	hHash = NULL;
	BOOL		bFinal = FALSE;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:        %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize:    %08x"), dwPasswordBufferSize);


			// Initialize/resolve the Advapi32 functions
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeAdvapiFunctions()"));
			dwErrorCode = InitializeAdvapiFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeAdvapiFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Get a handle to the Microsoft Enhanced RSA and AES Cryptographic Provider
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptAcquireContextW( %p, %p, %p, %08x, %08x )"),
				&hProv,
				NULL,
				MS_ENH_RSA_AES_PROV_W,
				PROV_RSA_AES,
				CRYPT_VERIFYCONTEXT
			);
			if (FALSE == _CryptAcquireContextW(
				&hProv,
				NULL,
				MS_ENH_RSA_AES_PROV_W,
				PROV_RSA_AES,
				CRYPT_VERIFYCONTEXT
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptAcquireContextW( %p, %p, %p, %08x, %08x ) failed. (%08x)"),
					&hProv,
					NULL,
					MS_ENH_RSA_AES_PROV_W,
					PROV_RSA_AES,
					CRYPT_VERIFYCONTEXT,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hProv:     %p"), hProv);


			// Get a handle to a SHA256 hashing object/provider
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptCreateHash( %p, %08x, %p, %08x, %p )"),
				hProv,
				CALG_SHA1,
				NULL,
				dwFlags,
				&hHash
			);
			if (FALSE == _CryptCreateHash(
				hProv,
				CALG_SHA1,
				NULL,
				dwFlags,
				&hHash
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptCreateHash( %p, %08x, %p, %08x, %p ) failed. (%08x)"),
					hProv,
					CALG_SHA1,
					NULL,
					dwFlags,
					&hHash,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hHash:     %p"), hHash);



			// Hash the password which is used to derive the key used for encrypting/decrypting the data
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptHashData( %p, %p, %08x, %08x )"),
				hHash,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				dwFlags
			);
			if (FALSE == _CryptHashData(
				hHash,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				dwFlags
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptHashData( %p, %p, %08x, %08x ) failed. (%08x)"),
					hHash,
					lpPasswordBuffer,
					dwPasswordBufferSize,
					dwFlags,
					dwErrorCode
				);
				__leave;
			}



			// Derive the key from the hashed password
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptDeriveKey( %p, %08x, %p, %08x, %p )"),
				hProv,
				dwAlgorithmID,
				hHash,
				dwFlags,
				&hKey
			);
			if (FALSE == _CryptDeriveKey(
				hProv,
				dwAlgorithmID,
				hHash,
				dwFlags,
				&hKey
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptDeriveKey( %p, %08x, %p, %08x, %p ) failed. (%08x)"),
					hProv,
					dwAlgorithmID,
					hHash,
					dwFlags,
					&hKey,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hKey:     %p"), hKey);


			// Allocate a buffer to hold the encrypted buffer
			*lpdwDestinationBufferSize = (dwSourceBufferSize / AES_BLOCK_LEN)*AES_BLOCK_LEN;
			if (dwSourceBufferSize % AES_BLOCK_LEN)
				*lpdwDestinationBufferSize += AES_BLOCK_LEN;
			dwErrorCode = ALLOC(*lpdwDestinationBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppDestinationBuffer:     %p"), *lppDestinationBuffer);



			// Loop through encrypting each block
			DWORD dwCurrentOffset = 0;
			for (dwCurrentOffset = 0; dwCurrentOffset < dwSourceBufferSize; dwCurrentOffset += dwBufferLength)
			{
				ZeroMemory(lpBuffer, AES_BLOCK_LEN);
				dwBufferLength = AES_BLOCK_LEN;
				if ((dwSourceBufferSize - dwCurrentOffset) < AES_BLOCK_LEN)
				{
					dwBufferLength = (dwSourceBufferSize - dwCurrentOffset);
					bFinal = TRUE;
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("CopyMemory( %p, %p, %08x )"), lpBuffer, lpSourceBuffer + dwCurrentOffset, dwBufferLength);
				CopyMemory(lpBuffer, lpSourceBuffer + dwCurrentOffset, dwBufferLength);

				// Encrypt the current block
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("_CryptEncrypt( %p, %p, %08x, %08x, %p, %p, %08x )"),
					hKey,
					NULL,
					bFinal,
					dwFlags,
					lpBuffer,
					&dwBufferLength,
					AES_BLOCK_LEN
				);
				if (FALSE == _CryptEncrypt(
					hKey,
					NULL,
					bFinal,
					dwFlags,
					lpBuffer,
					&dwBufferLength,
					AES_BLOCK_LEN
				)
					)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("_CryptEncrypt( %p, %p, %08x, %08x, %p, %p, %08x ) failed. (%08x)"),
						hKey,
						NULL,
						bFinal,
						dwFlags,
						lpBuffer,
						&dwBufferLength,
						AES_BLOCK_LEN,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwBufferLength:     %08x"), dwBufferLength);
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpBuffer:           %p"), lpBuffer);
				//for (DWORD i = 0; i < dwBufferLength; i++)
				//{
				//	DBGPRINT(DEBUG_VERBOSE, TEXT("lpBuffer[%d]: %02x"), i, lpBuffer[i]);
				//}

				DBGPRINT(DEBUG_VERBOSE, TEXT("CopyMemory( %p, %p, %08x )"), (*lppDestinationBuffer) + dwCurrentOffset, lpBuffer, dwBufferLength);
				CopyMemory((*lppDestinationBuffer) + dwCurrentOffset, lpBuffer, dwBufferLength);

			} // end loop through source buffer encrypting blocks



			*lpdwDestinationBufferSize = dwCurrentOffset;


		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					*lppDestinationBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}

			
			if (NULL != hKey)
			{
				if (FALSE == _CryptDestroyKey(hKey))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptDestroyKey failed.(%08x)"), dwTempErrorCode);
				}
				hKey = NULL;
			}

			if (NULL != hHash)
			{
				if (FALSE == _CryptDestroyHash(hHash))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptDestroyHash failed.(%08x)"), dwTempErrorCode);
				}
				hHash = NULL;
			}

			if (NULL != hProv)
			{
				if (FALSE == _CryptReleaseContext(hProv, 0))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptReleaseContext failed.(%08x)"), dwTempErrorCode);
				}
				hProv = NULL;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD DecryptBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize, PBYTE lpPasswordBuffer, DWORD dwPasswordBufferSize, DWORD dwAlgorithmID)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	BYTE	lpBuffer[AES_BLOCK_LEN];
	//DWORD	dwBufferSize = AES_BLOCK_LEN;
	DWORD	dwBufferLength = 0;

	HCRYPTPROV	hProv = NULL;

	DWORD		dwFlags = 0;
	HCRYPTKEY	hKey = NULL;
	HCRYPTHASH	hHash = NULL;
	BOOL		bFinal = FALSE;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:        %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize:    %08x"), dwPasswordBufferSize);


			// Initialize/resolve the Advapi32 functions
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeAdvapiFunctions()"));
			dwErrorCode = InitializeAdvapiFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeAdvapiFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Get a handle to the Microsoft Enhanced RSA and AES Cryptographic Provider
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptAcquireContextW( %p, %p, %p, %08x, %08x )"),
				&hProv,
				NULL,
				MS_ENH_RSA_AES_PROV_W,
				PROV_RSA_AES,
				CRYPT_VERIFYCONTEXT
			);
			if (FALSE == _CryptAcquireContextW(
				&hProv,
				NULL,
				MS_ENH_RSA_AES_PROV_W,
				PROV_RSA_AES,
				CRYPT_VERIFYCONTEXT
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptAcquireContextW( %p, %p, %p, %08x, %08x ) failed. (%08x)"),
					&hProv,
					NULL,
					MS_ENH_RSA_AES_PROV_W,
					PROV_RSA_AES,
					CRYPT_VERIFYCONTEXT,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hProv:     %p"), hProv);


			// Get a handle to a SHA256 hashing object/provider
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptCreateHash( %p, %08x, %p, %08x, %p )"),
				hProv,
				CALG_SHA1,
				NULL,
				dwFlags,
				&hHash
			);
			if (FALSE == _CryptCreateHash(
				hProv,
				CALG_SHA1,
				NULL,
				dwFlags,
				&hHash
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptCreateHash( %p, %08x, %p, %08x, %p ) failed. (%08x)"),
					hProv,
					CALG_SHA1,
					NULL,
					dwFlags,
					&hHash,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hHash:     %p"), hHash);



			// Hash the password which is used to derive the key used for encrypting/decrypting the data
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptHashData( %p, %p, %08x, %08x )"),
				hHash,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				dwFlags
			);
			if (FALSE == _CryptHashData(
				hHash,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				dwFlags
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptHashData( %p, %p, %08x, %08x ) failed. (%08x)"),
					hHash,
					lpPasswordBuffer,
					dwPasswordBufferSize,
					dwFlags,
					dwErrorCode
				);
				__leave;
			}



			// Derive the key from the hashed password
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptDeriveKey( %p, %08x, %p, %08x, %p )"),
				hProv,
				dwAlgorithmID,
				hHash,
				dwFlags,
				&hKey
			);
			if (FALSE == _CryptDeriveKey(
				hProv,
				dwAlgorithmID,
				hHash,
				dwFlags,
				&hKey
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptDeriveKey( %p, %08x, %p, %08x, %p ) failed. (%08x)"),
					hProv,
					dwAlgorithmID,
					hHash,
					dwFlags,
					&hKey,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hKey:     %p"), hKey);


			// Allocate a buffer to hold the decrypted buffer
			*lpdwDestinationBufferSize = 0;
			dwErrorCode = ALLOC(dwSourceBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppDestinationBuffer:     %p"), *lppDestinationBuffer);



			// Loop through decrypting each block
			DWORD dwCurrentOffset = 0;
			for (dwCurrentOffset = 0; dwCurrentOffset < dwSourceBufferSize; dwCurrentOffset += dwBufferLength)
			{
				ZeroMemory(lpBuffer, AES_BLOCK_LEN);
				dwBufferLength = AES_BLOCK_LEN;
				if ((dwSourceBufferSize - dwCurrentOffset) == 0)
				{
					//dwBufferLength = (dwSourceBufferSize - dwCurrentOffset);
					bFinal = TRUE;
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("CopyMemory( %p, %p, %08x )"), lpBuffer, lpSourceBuffer + dwCurrentOffset, dwBufferLength);
				CopyMemory(lpBuffer, lpSourceBuffer + dwCurrentOffset, dwBufferLength);

				// Encrypt the current block
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("_CryptDecrypt( %p, %p, %08x, %08x, %p, %p )"),
					hKey,
					NULL,
					bFinal,
					dwFlags,
					lpBuffer,
					&dwBufferLength
				);
				if (FALSE == _CryptDecrypt(
					hKey,
					NULL,
					bFinal,
					dwFlags,
					lpBuffer,
					&dwBufferLength
				)
					)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("_CryptDecrypt( %p, %p, %08x, %08x, %p, %p ) failed. (%08x)"),
						hKey,
						NULL,
						bFinal,
						dwFlags,
						lpBuffer,
						&dwBufferLength,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwBufferLength:     %08x"), dwBufferLength);

				DBGPRINT(DEBUG_VERBOSE, TEXT("CopyMemory( %p, %p, %08x )"), (*lppDestinationBuffer) + dwCurrentOffset, lpBuffer, dwBufferLength);
				CopyMemory((*lppDestinationBuffer) + dwCurrentOffset, lpBuffer, dwBufferLength);

			} // end loop through source buffer encrypting blocks


			*lpdwDestinationBufferSize = dwCurrentOffset;



		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					*lppDestinationBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}

			if (NULL != hKey)
			{
				if (FALSE == _CryptDestroyKey(hKey))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptDestroyKey failed.(%08x)"), dwTempErrorCode);
				}
				hKey = NULL;
			}

			if (NULL != hHash)
			{
				if (FALSE == _CryptDestroyHash(hHash))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptDestroyHash failed.(%08x)"), dwTempErrorCode);
				}
				hHash = NULL;
			}

			if (NULL != hProv)
			{
				if (FALSE == _CryptReleaseContext(hProv, 0))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("_CryptReleaseContext failed.(%08x)"), dwTempErrorCode);
				}
				hProv = NULL;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD EncodeBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize, DWORD dwEncodingType)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);


			// Initialize/resolve the Crypt32 functions
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeCryptFunctions()"));
			dwErrorCode = InitializeCryptFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeCryptFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Call the encoding function once to determine the size in bytes required for the output
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptBinaryToStringA( %p, %d, %08x, %p, %p )"),
				lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				NULL,
				lpdwDestinationBufferSize
			);
			_CryptBinaryToStringA(
				lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				NULL,
				lpdwDestinationBufferSize
			);
			DBGPRINT(DEBUG_VERBOSE, TEXT("*lpdwDestinationBufferSize:  %d (%08x)"), *lpdwDestinationBufferSize, *lpdwDestinationBufferSize);


			// Allocate the destination buffer
			dwErrorCode = ALLOC(*lpdwDestinationBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppDestinationBuffer:        %p"), *lppDestinationBuffer);


			// Actually encode the source buffer into the newly allocated destination buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptBinaryToStringA( %p, %d, %08x, %p, %p )"),
				lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				(LPSTR)(*lppDestinationBuffer),
				lpdwDestinationBufferSize
			);
			if (FALSE == _CryptBinaryToStringA(
				lpSourceBuffer,
				dwSourceBufferSize, 
				dwEncodingType, 
				(LPSTR)(*lppDestinationBuffer), 
				lpdwDestinationBufferSize 
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptBinaryToStringA( %p, %d, %08x, %p, %p ) failed. (%08x)"),
					lpSourceBuffer,
					dwSourceBufferSize,
					dwEncodingType,
					(LPSTR)(*lppDestinationBuffer),
					lpdwDestinationBufferSize,
					dwErrorCode
				);
				__leave;
			}

		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					*lppDestinationBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD DecodeBuffer(PBYTE lpSourceBuffer, DWORD dwSourceBufferSize, PBYTE* lppDestinationBuffer, PDWORD lpdwDestinationBufferSize, DWORD dwEncodingType)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpSourceBuffer:     %p"), lpSourceBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSourceBufferSize: %08x"), dwSourceBufferSize);


			// Initialize/resolve the Crypt32 functions
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeCryptFunctions()"));
			dwErrorCode = InitializeCryptFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeCryptFunctions failed.(%08x)"), dwErrorCode);
				__leave;
			}


			// Call the encoding function once to determine the size in bytes required for the output
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptStringToBinaryA( %p, %d, %08x, %p, %p, %p, %p )"),
				(LPSTR)lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				NULL,
				lpdwDestinationBufferSize,
				NULL,
				NULL
			);
			_CryptStringToBinaryA(
				(LPSTR)lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				NULL,
				lpdwDestinationBufferSize,
				NULL,
				NULL
			);
			DBGPRINT(DEBUG_VERBOSE, TEXT("*lpdwDestinationBufferSize:  %d (%08x)"), *lpdwDestinationBufferSize, *lpdwDestinationBufferSize);


			// Allocate the destination buffer
			dwErrorCode = ALLOC(*lpdwDestinationBufferSize, (LPVOID*)lppDestinationBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppDestinationBuffer:        %p"), *lppDestinationBuffer);


			// Actually encode the source buffer into the newly allocated destination buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("_CryptStringToBinaryA( %p, %d, %08x, %p, %p, %p, %p )"),
				(LPSTR)lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				(*lppDestinationBuffer),
				lpdwDestinationBufferSize,
				NULL,
				NULL
			);
			if (FALSE == _CryptStringToBinaryA(
				(LPSTR)lpSourceBuffer,
				dwSourceBufferSize,
				dwEncodingType,
				(*lppDestinationBuffer),
				lpdwDestinationBufferSize,
				NULL,
				NULL
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_CryptStringToBinaryA( %p, %d, %08x, %p, %p, %p, %p ) failed. (%08x)"),
					(LPSTR)lpSourceBuffer,
					dwSourceBufferSize,
					dwEncodingType,
					(*lppDestinationBuffer),
					lpdwDestinationBufferSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}

		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppDestinationBuffer)
				{
					dwTempErrorCode = FREE(*lppDestinationBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
					*lppDestinationBuffer = NULL;
				}
				*lppDestinationBuffer = NULL;
				*lpdwDestinationBufferSize = 0;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}



DWORD DecodeDecryptDecompressBuffer(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpEncryptedBuffer = NULL;
	DWORD	dwEncryptedBufferSize = 0;

	LPBYTE	lpCompressedBuffer = NULL;
	DWORD	dwCompressedBufferSize = 0;
	DWORD	dwCompressedBufferLength = 0;




	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d (%08x)"), dwInputBufferSize, dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:     %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize: %d (%08x)"), dwPasswordBufferSize, dwPasswordBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			// Decode the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("DecodeBuffer( %p, %d, %p, %p )"),
				lpInputBuffer,
				dwInputBufferSize,
				&lpEncryptedBuffer,
				&dwEncryptedBufferSize
			);
			dwErrorCode = DecodeBuffer(
				lpInputBuffer, 
				dwInputBufferSize, 
				&lpEncryptedBuffer, 
				&dwEncryptedBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("DecodeBuffer( %p, %d, %p, %p ) failed.(%08x)"),
					lpInputBuffer,
					dwInputBufferSize,
					&lpEncryptedBuffer,
					&dwEncryptedBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncryptedBufferSize:  %d (%08x)"), dwEncryptedBufferSize, dwEncryptedBufferSize);
			//for (DWORD i = 0; i < dwEncryptedBufferSize; i++)
			//{
			//	DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncryptedBuffer[%d]: %02x"), i, lpEncryptedBuffer[i]);
			//}


			// Decrypt the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("DecryptBuffer( %p, %d, %p, %p, %p, %d )"),
				lpEncryptedBuffer,
				dwEncryptedBufferSize,
				&lpCompressedBuffer,
				&dwCompressedBufferSize,
				lpPasswordBuffer,
				dwPasswordBufferSize
			);
			dwErrorCode = DecryptBuffer(
				lpEncryptedBuffer, 
				dwEncryptedBufferSize, 
				&lpCompressedBuffer, 
				&dwCompressedBufferSize, 
				lpPasswordBuffer, 
				dwPasswordBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("DecryptBuffer( %p, %d, %p, %p, %p, %d ) failed.(%08x)"),
					lpEncryptedBuffer,
					dwEncryptedBufferSize,
					&lpCompressedBuffer,
					&dwCompressedBufferSize,
					lpPasswordBuffer,
					dwPasswordBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCompressedBufferSize:  %d (%08x)"), dwCompressedBufferSize, dwCompressedBufferSize);
			//for (DWORD i = 0; i < dwCompressedBufferSize; i++)
			//{
			//	DBGPRINT(DEBUG_VERBOSE, TEXT("lpCompressedBuffer[%d]: %02x"), i, lpCompressedBuffer[i]);
			//}


			// Get the compressed buffer length
			CopyMemory( &dwCompressedBufferLength, lpCompressedBuffer, sizeof(DWORD) );
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCompressedBufferLength:  %d (%08x)"), dwCompressedBufferLength, dwCompressedBufferLength);
			if (dwCompressedBufferLength > (dwCompressedBufferSize - sizeof(DWORD)))
			{
				dwErrorCode = ERROR_BAD_COMPRESSION_BUFFER;
				DBGPRINT(DEBUG_ERROR, TEXT("Compressed buffer length is larger than the compressed buffer ( %d > %d )"), dwCompressedBufferLength, (dwCompressedBufferSize - sizeof(DWORD)));
				__leave;
			}


			// Decompress the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("DecompressBuffer( %p, %d, %p, %p )"),
				(lpCompressedBuffer + sizeof(DWORD)),
				dwCompressedBufferLength,
				lppOutputBuffer,
				lpdwOutputBufferSize
			);
			dwErrorCode = DecompressBuffer(
				(lpCompressedBuffer + sizeof(DWORD)),
				dwCompressedBufferLength,
				lppOutputBuffer,
				lpdwOutputBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("DecompressBuffer( %p, %d, %p, %p ) failed.(%08x)"),
					(lpCompressedBuffer + sizeof(DWORD)),
					dwCompressedBufferLength,
					lppOutputBuffer,
					lpdwOutputBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("(*lpdwOutputBufferSize):  %d (%08x)"), (*lpdwOutputBufferSize), (*lpdwOutputBufferSize));
			//for (DWORD i = 0; i < (*lpdwOutputBufferSize); i++)
			//{
			//	DBGPRINT(DEBUG_VERBOSE, TEXT("(*lppOutputBuffer)[%d]: %02x"), i, (*lppOutputBuffer)[i]);
			//}




		} // end try-finally
		__finally
		{
			if (NULL != lpEncryptedBuffer)
			{
				dwTempErrorCode = FREE(lpEncryptedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpEncryptedBuffer = NULL;
			}

			if (NULL != lpCompressedBuffer)
			{
				dwTempErrorCode = FREE(lpCompressedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpCompressedBuffer = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppOutputBuffer)
				{
					dwTempErrorCode = FREE(*lppOutputBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
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


DWORD CompressEncryptEncodeBuffer(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpDecryptedBuffer = NULL;
	DWORD	dwDecryptedBufferSize = 0;

	LPBYTE	lpEncryptedBuffer = NULL;
	DWORD	dwEncryptedBufferSize = 0;

	LPBYTE	lpCompressedBuffer = NULL;
	DWORD	dwCompressedBufferSize = 0;




	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d (%08x)"), dwInputBufferSize, dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:     %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize: %d (%08x)"), dwPasswordBufferSize, dwPasswordBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			//printf("[%s(%d) lpInputBuffer:\n", __FILE__, __LINE__);
			//for (DWORD i = 0; i < dwInputBufferSize; i++)
			//	printf("%02x ", lpInputBuffer[i]);
			//printf("\n");

			// Compress the buffer
			DBGPRINT(
				DEBUG_VERBOSE, 
				TEXT("CompressBuffer( %p, %d, %p, %p )"),
				lpInputBuffer,
				dwInputBufferSize,
				&lpCompressedBuffer,
				&dwCompressedBufferSize
			);
			dwErrorCode = CompressBuffer(
				lpInputBuffer, 
				dwInputBufferSize, 
				&lpCompressedBuffer, 
				&dwCompressedBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("CompressBuffer( %p, %d, %p, %p ) failed.(%08x)"),
					lpInputBuffer,
					dwInputBufferSize,
					&lpCompressedBuffer,
					&dwCompressedBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCompressedBufferSize:  %d (%08x)"), dwCompressedBufferSize, dwCompressedBufferSize);
			//printf("[%s(%d) lpCompressedBuffer:\n", __FILE__, __LINE__);
			//for (DWORD i = 0; i < dwCompressedBufferSize; i++)
			//	printf("%02x ", lpCompressedBuffer[i]);
			//printf("\n");


			// Prepend compressed buffer with size because encryption will pad the buffer to block_size
			dwDecryptedBufferSize = dwCompressedBufferSize + sizeof(DWORD);
			dwErrorCode = ALLOC( dwDecryptedBufferSize, (LPVOID*)(&lpDecryptedBuffer) );
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("ALLOC failed.(%08x)"), dwErrorCode);
				__leave;
			}
			CopyMemory( lpDecryptedBuffer, &dwCompressedBufferSize, sizeof(DWORD) );
			CopyMemory( lpDecryptedBuffer + sizeof(DWORD), lpCompressedBuffer, dwCompressedBufferSize );
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecryptedBufferSize:  %d (%08x)"), dwDecryptedBufferSize, dwDecryptedBufferSize);
			//printf("[%s(%d) lpDecryptedBuffer:\n", __FILE__, __LINE__);
			//for (DWORD i = 0; i < dwDecryptedBufferSize; i++)
			//	printf("%02x ", lpDecryptedBuffer[i]);
			//printf("\n");


			// Encrypt the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("EncryptBuffer( %p, %d, %p, %p, %p, %d )"),
				lpDecryptedBuffer,
				dwDecryptedBufferSize,
				&lpEncryptedBuffer,
				&dwEncryptedBufferSize,
				lpPasswordBuffer,
				dwPasswordBufferSize
			);
			dwErrorCode = EncryptBuffer(
				lpDecryptedBuffer, 
				dwDecryptedBufferSize, 
				&lpEncryptedBuffer, 
				&dwEncryptedBufferSize, 
				lpPasswordBuffer, 
				dwPasswordBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("EncryptBuffer( %p, %d, %p, %p, %p, %d ) failed.(%08x)"),
					lpDecryptedBuffer,
					dwDecryptedBufferSize,
					&lpEncryptedBuffer,
					&dwEncryptedBufferSize,
					lpPasswordBuffer,
					dwPasswordBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncryptedBufferSize:  %d (%08x)"), dwEncryptedBufferSize, dwEncryptedBufferSize);
			//printf("[%s(%d) lpEncryptedBuffer:\n", __FILE__, __LINE__);
			//for (DWORD i = 0; i < dwEncryptedBufferSize; i++)
			//	printf("%02x ", lpEncryptedBuffer[i]);
			//printf("\n");


			// Encode the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("EncodeBuffer( %p, %d, %p, %p )"),
				lpEncryptedBuffer,
				dwEncryptedBufferSize,
				lppOutputBuffer,
				lpdwOutputBufferSize
			);
			dwErrorCode = EncodeBuffer(
				lpEncryptedBuffer, 
				dwEncryptedBufferSize, 
				lppOutputBuffer, 
				lpdwOutputBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("EncodeBuffer( %p, %d, %p, %p ) failed.(%08x)"),
					lpEncryptedBuffer,
					dwEncryptedBufferSize,
					lppOutputBuffer,
					lpdwOutputBufferSize, 
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("(*lpdwOutputBufferSize):  %d (%08x)"), (*lpdwOutputBufferSize), (*lpdwOutputBufferSize));
			//printf("[%s(%d) (*lppOutputBuffer):\n", __FILE__, __LINE__);
			//for (DWORD i = 0; i < (*lpdwOutputBufferSize); i++)
			//	printf("%02x ", (*lppOutputBuffer)[i]);
			//printf("\n");


		} // end try-finally
		__finally
		{
			if (NULL != lpEncryptedBuffer)
			{
				dwTempErrorCode = FREE(lpEncryptedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpEncryptedBuffer = NULL;
			}

			if (NULL != lpDecryptedBuffer)
			{
				dwTempErrorCode = FREE(lpDecryptedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpDecryptedBuffer = NULL;
			}

			if (NULL != lpCompressedBuffer)
			{
				dwTempErrorCode = FREE(lpCompressedBuffer);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
				}
				lpCompressedBuffer = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppOutputBuffer)
				{
					dwTempErrorCode = FREE(*lppOutputBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					}
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



DWORD ReadFileIntoBuffer(
	IN	LPSTR	lpszFileName,
	OUT	LPBYTE*	lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	HANDLE	hFile = INVALID_HANDLE_VALUE;

	DWORD	dwBytesRead = 0;

	__try
	{
		__try
		{
			if (NULL == lpszFileName)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("lpszFileName is NULL"));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpszFileName:     %S"), lpszFileName);

			// Opening the existing file
			hFile = CreateFileA(
				lpszFileName,		// file to open
				GENERIC_READ,		// open for reading
				FILE_SHARE_READ,	// share for reading
				NULL,				// default security
				OPEN_EXISTING,		// existing file only
				FILE_ATTRIBUTE_NORMAL,// normal file
				NULL				// no attribute template
			);
			if ( INVALID_HANDLE_VALUE == hFile )
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("CreateFileA( %p, %08x, %08x, %p, %08x, %08x, %p ) failed. (%08x)"), 
					lpszFileName, 
					GENERIC_READ,		
					FILE_SHARE_READ,	
					NULL,				
					OPEN_EXISTING,		
					FILE_ATTRIBUTE_NORMAL,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			(*lpdwOutputBufferSize) = GetFileSize(hFile, NULL);
			DBGPRINT(DEBUG_VERBOSE, TEXT("(*lpdwOutputBufferSize):     %d"), (*lpdwOutputBufferSize));

			if ( 0 == (*lpdwOutputBufferSize))
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("lpszFileName is zero bytes"));
				__leave;
			}
			
			if (INVALID_FILE_SIZE == (*lpdwOutputBufferSize))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("GetFileSize( %p, %08x ) failed. (%08x)"),
					hFile, 
					NULL,
					dwErrorCode
				);
				__leave;
			}

			dwErrorCode = ALLOC((*lpdwOutputBufferSize), (LPVOID *)lppOutputBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ALLOC( %d, %p ) failed. (%08x)"),
					(*lpdwOutputBufferSize), 
					(LPVOID *)lppOutputBuffer,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("(*lppOutputBuffer):     %p"), (*lppOutputBuffer));

			if (
				FALSE == ReadFile(
					hFile,
					(*lppOutputBuffer),
					(*lpdwOutputBufferSize),
					&dwBytesRead,
					NULL
				)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFile( %p, %p, %d, %p, %p ) failed. (%08x)"),
					hFile,
					(*lppOutputBuffer),
					(*lpdwOutputBufferSize),
					&dwBytesRead,
					NULL,
					dwErrorCode
				);
				__leave;
			}

			if (dwBytesRead != (*lpdwOutputBufferSize))
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("Could not read all of the data from %S"), lpszFileName);
				__leave;
			}

		} // end try-finally
		__finally
		{
			if (INVALID_HANDLE_VALUE != hFile)
			{
				if (FALSE == CloseHandle(hFile))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("CloseHandle(%p) failed. (%08x)"), hFile, dwTempErrorCode);
				}
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppOutputBuffer)
				{
					dwTempErrorCode = FREE(*lppOutputBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed. (%08x)"), dwTempErrorCode);
					}
					*lppOutputBuffer = NULL;
				}
				*lppOutputBuffer = NULL;
				*lpdwOutputBufferSize = 0;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}


DWORD WriteFileOutToDisk(
	IN	LPSTR	lpszFileName,
	IN	LPBYTE	lpInputBuffer,
	IN  DWORD   dwInputBufferSize,
	IN OPTIONAL	DWORD	dwDisposition = CREATE_NEW,
	IN OPTIONAL	DWORD	dwAttributes = FILE_ATTRIBUTE_NORMAL
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;
	DWORD	dwCreateFileErrorCode = ERROR_SUCCESS;

	HANDLE	hFile = INVALID_HANDLE_VALUE;

	__try
	{
		__try
		{
			if (NULL == lpszFileName)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("lpszFileName is NULL"));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpszFileName:     %S"), lpszFileName);

			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("lpInputBuffer is NULL"));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:     %p"), lpInputBuffer);
			if (0 == dwInputBufferSize)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(DEBUG_ERROR, TEXT("dwInputBufferSize is 0"));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize: %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDisposition:     %d"), dwDisposition);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwAttributes:      %d"), dwAttributes);

			// Opening the existing file
			hFile = CreateFileA(
				lpszFileName,		// file to open
				GENERIC_WRITE,		// open for writing
				0,					// do not share
				NULL,				// default security
				dwDisposition,		// use specified disposition
				dwAttributes,		// use specified attributes
				NULL				// no attribute template
			);
			if (INVALID_HANDLE_VALUE == hFile)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("CreateFileA( %p, %08x, %08x, %p, %08x, %08x, %p ) failed. (%08x)"),
					lpszFileName,
					GENERIC_WRITE,
					0,
					NULL,
					dwDisposition,
					dwAttributes,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			dwCreateFileErrorCode = GetLastError();
			
			if (
				FALSE == WriteFile(
					hFile,
					lpInputBuffer,
					dwInputBufferSize,
					NULL,
					NULL
				)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("WriteFile( %p, %p, %d, %p, %p ) failed. (%08x)"),
					hFile,
					lpInputBuffer,
					dwInputBufferSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


		} // end try-finally
		__finally
		{
			if (INVALID_HANDLE_VALUE != hFile)
			{
				if (FALSE == CloseHandle(hFile))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(DEBUG_ERROR, TEXT("CloseHandle(%p) failed. (%08x)"), hFile, dwTempErrorCode);
				}
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}


DWORD GetConfigurationFromBinary(
		IN LPBYTE	lpInputBuffer,
		IN DWORD	dwInputBufferSize,
		IN LPBYTE	lpPasswordBuffer,
		IN DWORD	dwPasswordBufferSize,
		OUT LPBYTE* lppOutputBuffer,
		OUT LPDWORD lpdwOutputBufferSize
)
{
	return DecodeDecryptDecompressBuffer(
		lpInputBuffer,
		dwInputBufferSize,
		lpPasswordBuffer,
		dwPasswordBufferSize,
		lppOutputBuffer,
		lpdwOutputBufferSize
		);
}


DWORD GetConfigurationFromResource(
	IN DWORD	dwResourceType,
	IN DWORD	dwResourceName,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpDecryptedBuffer = NULL;
	DWORD	dwDecryptedBufferSize = 0;

	LPBYTE	lpEncodedBuffer = NULL;
	DWORD	dwEncodedBufferSize = 0;

	HRSRC	hResource = NULL;
	HMODULE hCurrentModule = NULL;
	HGLOBAL hLoaddedResource = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			DBGPRINT(DEBUG_VERBOSE, TEXT("dwResourceType:        %d (%08x)"), dwResourceType, dwResourceType);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwResourceName:        %d (%08x)"), dwResourceName, dwResourceName);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:      %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize:  %d (%08x)"), dwPasswordBufferSize, dwPasswordBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:       %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize:  %p"), lpdwOutputBufferSize);

			// Get a handle to the current module (dll/exe)
			hCurrentModule = GetCurrentModule();
			if (NULL == hCurrentModule)
			{
				dwErrorCode = ERROR_INVALID_HANDLE;
				DBGPRINT(DEBUG_ERROR, TEXT("GetCurrentModule failed."));
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hCurrentModule: %p"), hCurrentModule);

			// Find the default resource in the current module 
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("FindResourceA( %p, %p, %p )"),
				hCurrentModule,						// hModule
				MAKEINTRESOURCEA(dwResourceName),	// lpName
				MAKEINTRESOURCEA(dwResourceType)	// lpType
			);
			hResource = FindResourceA(
				hCurrentModule,						// hModule
				MAKEINTRESOURCEA(dwResourceName),	// lpName
				MAKEINTRESOURCEA(dwResourceType)	// lpType
			);
			if (NULL == hResource)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FindResourceA( %p, %p, %p ) failed. (%08x)"),
					hCurrentModule,									// hModule
					MAKEINTRESOURCEA(dwResourceName),	// lpName
					MAKEINTRESOURCEA(dwResourceType),	// lpType
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hResource: %p"), hResource);

			// Load the default resource in the current module
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoadResource( %p, %p )"),
				hCurrentModule,
				hResource
			);
			hLoaddedResource = LoadResource(hCurrentModule, hResource);
			if (NULL == hLoaddedResource)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoadResource( %p, %p ) failed. (%08x)"),
					hCurrentModule,
					hResource,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hLoaddedResource: %p"), hLoaddedResource);

			// Lock the default resource in the current module
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LockResource( %p )"),
				hLoaddedResource
			);
			lpEncodedBuffer = (LPBYTE)LockResource(hLoaddedResource);
			if (NULL == lpEncodedBuffer)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LockResource( %p ) failed. (%08x)"),
					hLoaddedResource,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer: %p"), lpEncodedBuffer);


			// Get the size of the default resource in the current module
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("SizeofResource( %p, %p )"),
				hCurrentModule,
				hResource
			);
			dwEncodedBufferSize = SizeofResource(hCurrentModule, hResource);
			if (0 == dwEncodedBufferSize)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SizeofResource( %p, %p ) failed. (%08x)"),
					hCurrentModule,
					hResource,
					dwErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("lpEncodedBuffer:        %p"), lpEncodedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwEncodedBufferSize:    %d (%08x)"), dwEncodedBufferSize, dwEncodedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPasswordBuffer:       %p"), lpPasswordBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPasswordBufferSize:   %d (%08x)"), dwPasswordBufferSize, dwPasswordBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecryptedBuffer:      %p"), &lpDecryptedBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecryptedBufferSize:  %p"), &dwDecryptedBufferSize);

			// Decode/Decrypt/Decompress the buffer
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("DecodeDecryptDecompressBuffer( %p, %d, %p, %d, %p, %p )"),
				lpEncodedBuffer,
				dwEncodedBufferSize,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				&lpDecryptedBuffer,
				&dwDecryptedBufferSize
			);
			dwErrorCode = DecodeDecryptDecompressBuffer(
				lpEncodedBuffer,
				dwEncodedBufferSize,
				lpPasswordBuffer,
				dwPasswordBufferSize,
				&lpDecryptedBuffer,
				&dwDecryptedBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("DecodeDecryptDecompressBuffer failed.(%08x)"), dwErrorCode);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecryptedBufferSize:  %d (%08x)"), dwDecryptedBufferSize, dwDecryptedBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecryptedBuffer:      %p"), lpDecryptedBuffer);

			(*lppOutputBuffer) = lpDecryptedBuffer;
			(*lpdwOutputBufferSize) = dwDecryptedBufferSize;

		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != *lppOutputBuffer)
				{
					dwTempErrorCode = FREE(*lppOutputBuffer);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(DEBUG_ERROR, TEXT("FREE failed. (%08x)"), dwTempErrorCode);
					}
					*lppOutputBuffer = NULL;
				}
				*lppOutputBuffer = NULL;
				*lpdwOutputBufferSize = 0;
			}

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}