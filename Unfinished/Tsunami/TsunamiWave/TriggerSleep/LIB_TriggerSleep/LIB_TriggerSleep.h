#pragma once
#ifndef __LIB_TriggerSleep__
#define __LIB_TriggerSleep__

#include <Windows.h>

#define TriggerSleep_Request						"trigger_sleep_request"
#define TriggerSleep_Request_Sleep_Seconds			"sleep_seconds"
#define TriggerSleep_Request_Variation_Seconds		"variation_seconds"
#define TriggerSleep_Response_Wake_Time				"wake_time"


DWORD WINAPI TriggerSleepInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI TriggerSleepFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI TriggerSleepCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
