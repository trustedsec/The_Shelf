#pragma once
#ifndef __DLL_LoadLibrary__
#define __DLL_LoadLibrary__

extern "C" __declspec(dllexport) DWORD __cdecl DLL_LoadLibrary_ExportedFunction(void);
extern HANDLE g_hThread;

#endif
