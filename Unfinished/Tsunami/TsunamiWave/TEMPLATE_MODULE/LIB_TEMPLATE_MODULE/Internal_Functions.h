#pragma once
#ifndef __LIB_Internal_TEMPLATE_MODULE_Functions__
#define __LIB_Internal_TEMPLATE_MODULE_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_TEMPLATE_MODULE_Kernel32 "Kernel32.dll"
static HMODULE g_TEMPLATE_MODULE_hKernel32 = NULL;

#define SZ_TEMPLATE_MODULE_Sleep "Sleep"
typedef VOID(WINAPI* f_TEMPLATE_MODULE_Sleep)(
	__in        DWORD dwMilliseconds
	);
extern f_TEMPLATE_MODULE_Sleep m_TEMPLATE_MODULE_Sleep;

#define SZ_TEMPLATE_MODULE_GetSystemTime "GetSystemTime"
typedef VOID(WINAPI* f_TEMPLATE_MODULE_GetSystemTime)(
	__out       LPSYSTEMTIME lpSystemTime
	);
extern f_TEMPLATE_MODULE_GetSystemTime m_TEMPLATE_MODULE_GetSystemTime;


DWORD Initialize_TEMPLATE_MODULE_Kernel32_Functions();
DWORD Finalize_TEMPLATE_MODULE_Kernel32_Functions();


#endif
