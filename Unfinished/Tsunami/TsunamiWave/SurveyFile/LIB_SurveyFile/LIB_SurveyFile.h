#pragma once
#ifndef __LIB_SurveyFile__
#define __LIB_SurveyFile__

#include <Windows.h>

#define SurveyFile_Request						"survey_file_request"
//#define SurveyFile_Response						"survey_file_response"
#define SurveyFile_Request_Entries				"file_entries"
#define SurveyFile_Response_Results				"file_results"
#define SurveyFile_Response_DirectoryName		"directory_name"
#define SurveyFile_Response_FileName			"file_name"
#define SurveyFile_Response_FileAttributes		"file_attributes"
#define SurveyFile_Response_CreationTimeLow		"creation_time_low"
#define SurveyFile_Response_CreationTimeHigh	"creation_time_high"
#define SurveyFile_Response_LastWriteTimeLow	"last_write_time_low"
#define SurveyFile_Response_LastWriteTimeHigh	"last_write_time_high"
#define SurveyFile_Response_LastAccessTimeLow	"last_access_time_low"
#define SurveyFile_Response_LastAccessTimeHigh	"last_access_time_high"
#define SurveyFile_Response_FileSizeLow			"file_size_low"
#define SurveyFile_Response_FileSizeHigh		"file_size_high"


DWORD WINAPI SurveyFileInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyFileFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyFileCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
