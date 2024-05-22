#pragma once
#ifndef __DLL_TriggerSleep__
#define __DLL_TriggerSleep__

extern "C" __declspec(dllexport) DWORD WINAPI Initialization(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

extern "C" __declspec(dllexport) DWORD WINAPI Finalization(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

extern "C" __declspec(dllexport) DWORD WINAPI Callback1(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

#endif
