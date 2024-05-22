#pragma once
#ifndef __LIB_Internal_FileBasic_Functions__
#define __LIB_Internal_FileBasic_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_FileBasic_Kernel32 "Kernel32.dll"
static HMODULE g_FileBasic_hKernel32 = NULL;

#define SZ_FileBasic_DeleteFileA "DeleteFileA"
typedef BOOL(WINAPI* f_FileBasic_DeleteFileA)(
	__in LPCSTR lpFileName
	);
extern f_FileBasic_DeleteFileA m_FileBasic_DeleteFileA;

DWORD Initialize_FileBasic_Kernel32_Functions();
DWORD Finalize_FileBasic_Kernel32_Functions();


#endif
