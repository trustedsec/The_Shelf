#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

/******************************************************************************
	Advapi32 typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyRegistry_Advapi32 "Advapi32.dll"
static HMODULE g_SurveyRegistry_hAdvapi32 = NULL;

#define SZ_SurveyRegistry_HKEY_CLASSES_ROOT "HKEY_CLASSES_ROOT"
#define SZ_SurveyRegistry_HKEY_CURRENT_CONFIG "HKEY_CURRENT_CONFIG"
#define SZ_SurveyRegistry_HKEY_CURRENT_USER "HKEY_CURRENT_USER"
#define SZ_SurveyRegistry_HKEY_CURRENT_USER_LOCAL_SETTINGS "HKEY_CURRENT_USER_LOCAL_SETTINGS"
#define SZ_SurveyRegistry_HKEY_LOCAL_MACHINE "HKEY_LOCAL_MACHINE"
#define SZ_SurveyRegistry_HKEY_PERFORMANCE_DATA "HKEY_PERFORMANCE_DATA"
#define SZ_SurveyRegistry_HKEY_PERFORMANCE_NLSTEXT "HKEY_PERFORMANCE_NLSTEXT"
#define SZ_SurveyRegistry_HKEY_PERFORMANCE_TEXT "HKEY_PERFORMANCE_TEXT"
#define SZ_SurveyRegistry_HKEY_USERS "HKEY_USERS"



#define SZ_SurveyRegistry_RegOpenKeyExA "RegOpenKeyExA"
typedef LSTATUS(APIENTRY* f_SurveyRegistry_RegOpenKeyExA)(
	__in HKEY hKey,
	__in_opt LPCSTR lpSubKey,
	__in_opt DWORD ulOptions,
	__in REGSAM samDesired,
	__out PHKEY phkResult
);
extern f_SurveyRegistry_RegOpenKeyExA m_SurveyRegistry_RegOpenKeyExA;


#define SZ_SurveyRegistry_RegEnumValueA "RegEnumValueA"
typedef LSTATUS(APIENTRY* f_SurveyRegistry_RegEnumValueA)(
	__in HKEY hKey,
	__in DWORD dwIndex,
	__out_ecount_part_opt(*lpcchValueName, *lpcchValueName + 1) LPSTR lpValueName,
	__inout LPDWORD lpcchValueName,
	__reserved LPDWORD lpReserved,
	__out_opt LPDWORD lpType,
	__out_bcount_part_opt(*lpcbData, *lpcbData) __out_data_source(REGISTRY) LPBYTE lpData,
	__inout_opt LPDWORD lpcbData
);
extern f_SurveyRegistry_RegEnumValueA m_SurveyRegistry_RegEnumValueA;


#define SZ_SurveyRegistry_RegCloseKey "RegCloseKey"
typedef LSTATUS(APIENTRY* f_SurveyRegistry_RegCloseKey)(
	__in HKEY hKey
);
extern f_SurveyRegistry_RegCloseKey m_SurveyRegistry_RegCloseKey;




DWORD Initialize_SurveyRegistry_Advapi32_Functions();
DWORD Finalize_SurveyRegistry_Advapi32_Functions();





#endif
