#pragma once
#ifndef __LIB_Internal_TriggerSleep_Functions__
#define __LIB_Internal_TriggerSleep_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_TriggerSleep_Kernel32 "Kernel32.dll"
static HMODULE g_TriggerSleep_hKernel32 = NULL;

#define SZ_TriggerSleep_Sleep "Sleep"
typedef VOID(WINAPI* f_TriggerSleep_Sleep)(
	__in        DWORD dwSeconds
	);
extern f_TriggerSleep_Sleep m_TriggerSleep_Sleep;

#define SZ_TriggerSleep_GetSystemTime "GetSystemTime"
typedef VOID(WINAPI* f_TriggerSleep_GetSystemTime)(
	__out       LPSYSTEMTIME lpSystemTime
	);
extern f_TriggerSleep_GetSystemTime m_TriggerSleep_GetSystemTime;


DWORD Initialize_TriggerSleep_Kernel32_Functions();
DWORD Finalize_TriggerSleep_Kernel32_Functions();


#endif
