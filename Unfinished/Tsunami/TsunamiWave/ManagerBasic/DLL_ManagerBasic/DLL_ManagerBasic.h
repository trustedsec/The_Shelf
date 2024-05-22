#pragma once
#ifndef __DLL_ManagerBasic__
#define __DLL_ManagerBasic__

extern "C" __declspec(dllexport) DWORD __cdecl DLL_ManagerBasic_ExportedFunction(void);
extern HANDLE g_ManagerBasic_hThread;

#endif
