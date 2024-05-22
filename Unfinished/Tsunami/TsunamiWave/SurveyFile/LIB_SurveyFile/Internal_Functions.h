#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyFile_Kernel32 "Kernel32.dll"
extern HMODULE g_SurveyFile_hKernel32;

#define SZ_SurveyFile_GetFullPathNameA "GetFullPathNameA"
typedef 
DWORD
(WINAPI*
f_SurveyFile_GetFullPathNameA)(
	__in            LPCSTR lpFileName,
	__in            DWORD nBufferLength,
	__out_ecount_part_opt(nBufferLength, return +1) LPSTR lpBuffer,
	__deref_opt_out LPSTR *lpFilePart
);
extern f_SurveyFile_GetFullPathNameA m_SurveyFile_GetFullPathNameA;

#define SZ_SurveyFile_FindFirstFileA "FindFirstFileA"
typedef
__out
HANDLE
(WINAPI*
f_SurveyFile_FindFirstFileA)(
	__in  LPCSTR lpFileName,
	__out LPWIN32_FIND_DATAA lpFindFileData
);
extern f_SurveyFile_FindFirstFileA m_SurveyFile_FindFirstFileA;

#define SZ_SurveyFile_FindNextFileA "FindNextFileA"
typedef
BOOL
(WINAPI*
f_SurveyFile_FindNextFileA)(
	__in  HANDLE hFindFile,
	__out LPWIN32_FIND_DATAA lpFindFileData
);
extern f_SurveyFile_FindNextFileA m_SurveyFile_FindNextFileA;

#define SZ_SurveyFile_FindClose "FindClose"
typedef
BOOL
(WINAPI*
f_SurveyFile_FindClose)(
	__inout HANDLE hFindFile
);
extern f_SurveyFile_FindClose m_SurveyFile_FindClose;

/******************************************************************************
	Private functions to look up and resolve functions
******************************************************************************/
DWORD Initialize_SurveyFile_Kernel32_Functions();
DWORD Finalize_SurveyFile_Kernel32_Functions();





#endif
