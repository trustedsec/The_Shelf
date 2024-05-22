#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_ExecuteCommand_Kernel32 "Kernel32.dll"
static HMODULE g_ExecuteCommand_hKernel32 = NULL;

#define SZ_ExecuteCommand_CreateProcessA "CreateProcessA"
typedef BOOL(WINAPI* f_ExecuteCommand_CreateProcessA)(
	__in_opt    LPCSTR lpApplicationName,
	__inout_opt LPSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOA lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
);
extern f_ExecuteCommand_CreateProcessA m_ExecuteCommand_CreateProcessA;


DWORD Initialize_ExecuteCommand_Kernel32_Functions();
DWORD Finalize_ExecuteCommand_Kernel32_Functions();





#endif
