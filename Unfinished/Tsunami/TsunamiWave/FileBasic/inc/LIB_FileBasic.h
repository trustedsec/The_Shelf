#pragma once
#ifndef __LIB_FileBasic__
#define __LIB_FileBasic__

#include <Windows.h>

#define FileBasic_Upload_Request						"file_basic_upload_request"
#define FileBasic_Upload_Request_File_Name				"file_name"
#define FileBasic_Upload_Response_File_Contents			"file_contents"

#define FileBasic_Download_Request						"file_basic_download_request"
#define FileBasic_Download_Request_File_Name			"file_name"
#define FileBasic_Download_Request_File_Contents		"file_contents"
#define FileBasic_Download_Request_Creation_Disposition	"creation_disposition"
#define FileBasic_Download_Request_Creation_Attributes	"creation_attributes"
#define FileBasic_Download_Response_ErrorCode			"file_write_error_code"

#define FileBasic_Delete_Request						"file_basic_delete_request"
#define FileBasic_Delete_Request_File_Name				"file_name"
#define FileBasic_Delete_Response_Delete_Results		"delete_results"


DWORD WINAPI FileBasicInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI FileBasicFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI FileBasicDownload(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI FileBasicUpload(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI FileBasicDelete(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
