#pragma once
#ifndef __LIB_LoaderMemoryModule__
#define __LIB_LoaderMemoryModule__

#include <Windows.h>

#define LoaderMemoryModule_LoadLibrary_Request				"loadermemorymodule_loadlibrary_request"
#define LoaderMemoryModule_LoadLibrary_LibraryBuffer		"library_buffer"
#define LoaderMemoryModule_LoadLibrary_ModuleHandle			"module_handle"

#define LoaderMemoryModule_GetProcAddr_Request				"loadermemorymodule_getprocaddr_request"
#define LoaderMemoryModule_GetProcAddr_FunctionName			"function_name"
#define LoaderMemoryModule_GetProcAddr_ModuleHandle			"module_handle"
#define LoaderMemoryModule_GetProcAddr_FunctionPointer		"function_pointer"

#define LoaderMemoryModule_CallProc_Request					"loadermemorymodule_callproc_request"
#define LoaderMemoryModule_CallProc_FunctionName			"function_name"
#define LoaderMemoryModule_CallProc_ModuleHandle			"module_handle"
#define LoaderMemoryModule_CallProc_BufferArgument			"buffer_argument"
#define LoaderMemoryModule_CallProc_SizeArgument			"size_argument"
#define LoaderMemoryModule_CallProc_FunctionReturn			"function_return"

#define LoaderMemoryModule_FreeLibrary_Request				"loadermemorymodule_freeibrary_request"
#define LoaderMemoryModule_FreeLibrary_ModuleHandle			"module_handle"
#define LoaderMemoryModule_FreeLibrary_Results				"free_library_results"

typedef DWORD (WINAPI *CALLPROC_0)();
typedef DWORD(WINAPI *CALLPROC_1)(LPVOID);
typedef DWORD(WINAPI *CALLPROC_2)(DWORD);
typedef DWORD(WINAPI *CALLPROC_3)(LPVOID,DWORD);



DWORD WINAPI LoaderMemoryModuleInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI LoaderMemoryModuleFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI LoaderMemoryModuleLoadLibrary(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI LoaderMemoryModuleFreeLibrary(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI LoaderMemoryModuleGetProcAddr(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI LoaderMemoryModuleCallProc(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
