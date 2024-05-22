#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <winsock2.h>
#include <iphlpapi.h>

#include <Windows.h>



/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyHost_Kernel32 "Kernel32.dll"
extern HMODULE g_SurveyHost_hKernel32;

#define SZ_SurveyHost_GetComputerNameExA "GetComputerNameExA"
typedef
BOOL
(WINAPI*
f_SurveyHost_GetComputerNameExA)(
	__in    COMPUTER_NAME_FORMAT NameType,
	__out_ecount_part_opt(*nSize, *nSize + 1) LPSTR lpBuffer,
	__inout LPDWORD nSize
);
extern f_SurveyHost_GetComputerNameExA m_SurveyHost_GetComputerNameExA;

#define SZ_SurveyHost_GetVersionExA "GetVersionExA"
typedef
BOOL
(WINAPI*
f_SurveyHost_GetVersionExA)(
	__inout LPOSVERSIONINFOA lpVersionInformation
);
extern f_SurveyHost_GetVersionExA m_SurveyHost_GetVersionExA;

#define SZ_SurveyHost_GetNativeSystemInfo "GetNativeSystemInfo"
typedef
VOID
(WINAPI*
f_SurveyHost_GetNativeSystemInfo)(
	__out LPSYSTEM_INFO lpSystemInfo
);
extern f_SurveyHost_GetNativeSystemInfo m_SurveyHost_GetNativeSystemInfo;

#define SZ_SurveyHost_ExpandEnvironmentStringsA "ExpandEnvironmentStringsA"
typedef
DWORD
(WINAPI*
f_SurveyHost_ExpandEnvironmentStringsA)(
	__in LPCSTR lpSrc,
	__out_ecount_part_opt(nSize, return) LPSTR lpDst,
	__in DWORD nSize
);
extern f_SurveyHost_ExpandEnvironmentStringsA m_SurveyHost_ExpandEnvironmentStringsA;

#define SZ_SurveyHost_GetVolumeInformationA "GetVolumeInformationA"
typedef
BOOL
(WINAPI*
f_SurveyHost_GetVolumeInformationA)(
	__in_opt  LPCSTR lpRootPathName,
	__out_ecount_opt(nVolumeNameSize) LPSTR lpVolumeNameBuffer,
	__in      DWORD nVolumeNameSize,
	__out_opt LPDWORD lpVolumeSerialNumber,
	__out_opt LPDWORD lpMaximumComponentLength,
	__out_opt LPDWORD lpFileSystemFlags,
	__out_ecount_opt(nFileSystemNameSize) LPSTR lpFileSystemNameBuffer,
	__in      DWORD nFileSystemNameSize
);
extern f_SurveyHost_GetVolumeInformationA m_SurveyHost_GetVolumeInformationA;



DWORD Initialize_SurveyHost_Kernel32_Functions();
DWORD Finalize_SurveyHost_Kernel32_Functions();




#endif