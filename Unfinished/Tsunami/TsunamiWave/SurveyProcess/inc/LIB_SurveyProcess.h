#pragma once
#ifndef __LIB_SurveyProcess__
#define __LIB_SurveyProcess__

#include <Windows.h>

#define SurveyProcess_Request						"survey_process_request"
#define SurveyProcess_Request_Entries				"process_entries"
#define SurveyProcess_Response						"survey_process_response"
#define SurveyProcess_Response_Results				"process_results"
#define SurveyProcess_Response_ExeFile				"exe_file"
#define SurveyProcess_Response_ProcessId			"process_id"
#define SurveyProcess_Response_ParentProcessId		"parent_process_id"


DWORD WINAPI SurveyProcessInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyProcessFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyProcessCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
