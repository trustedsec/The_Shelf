#pragma once
#ifndef __LIB_SurveyRegistry__
#define __LIB_SurveyRegistry__

#include <Windows.h>

#define SurveyRegistry_Request							"survey_registry_request"
#define SurveyRegistry_Request_Entries					"registry_entries"
//#define SurveyRegistry_Response							"survey_registry_response"
#define SurveyRegistry_Response_Results					"registry_results"
#define SurveyRegistry_Response_Results_RootKey			"root_key"
#define SurveyRegistry_Response_Results_SubKey			"sub_key"
#define SurveyRegistry_Response_Results_Value			"value"
#define SurveyRegistry_Response_Results_Type			"type"
#define SurveyRegistry_Response_Results_DataSize		"data_size"
#define SurveyRegistry_Response_Results_Data			"data"
#define SurveyRegistry_Response_Results_Type			"type"



DWORD WINAPI SurveyRegistryInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyRegistryFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyRegistryCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
