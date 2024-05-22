#pragma once
#ifndef __LIB_LoadLibrary__
#define __LIB_LoadLibrary__

#include <Windows.h>

#define LoadLibrary_Request_Entries					"load_library_entries"
#define LoadLibrary_Request_LibraryBuffer			"library_buffer"
#define LoadLibrary_Response_Results				"load_library_results"
#define LoadLibrary_Response_ModuleHandle			"loaded_module_pointer"


DWORD WINAPI LoadLibraryInitialization(
	IN	FARPROC fpRegisterCallback,
	IN	LPBYTE	lpInitializationBuffer,
	IN	DWORD	dwInitializationBufferSize
);

DWORD WINAPI LoadLibraryFinalization(
);

DWORD WINAPI LoadLibraryCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
);

#endif
