#pragma once
#ifndef __LIB_TEMPLATE_MODULE__
#define __LIB_TEMPLATE_MODULE__

#include <Windows.h>

#define TEMPLATE_MODULE_Request						"trigger_sleep_request"
#define TEMPLATE_MODULE_Request_Sleep_Milliseconds		"sleep_milliseconds"
#define TEMPLATE_MODULE_Response_Wake_Time				"wake_time"


DWORD WINAPI TEMPLATE_MODULEInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI TEMPLATE_MODULEFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI TEMPLATE_MODULECallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
