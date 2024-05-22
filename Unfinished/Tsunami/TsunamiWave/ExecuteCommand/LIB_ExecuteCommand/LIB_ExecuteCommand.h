#pragma once
#ifndef __LIB_ExecuteCommand__
#define __LIB_ExecuteCommand__

#include <Windows.h>

#define ExecuteCommand_Request							"execute_command_request"
#define ExecuteCommand_Request_CommandLine				"command_line"
#define ExecuteCommand_Response_ProcessId				"process_id"


DWORD WINAPI ExecuteCommandInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI ExecuteCommandFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI ExecuteCommandCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
