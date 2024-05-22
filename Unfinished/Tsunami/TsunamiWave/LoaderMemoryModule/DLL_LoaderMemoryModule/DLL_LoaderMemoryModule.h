#pragma once
#ifndef __DLL_LoaderMemoryModule__
#define __DLL_LoaderMemoryModule__

extern "C" __declspec(dllexport) DWORD __cdecl DLL_LoaderMemoryModule_ExportedFunction(void);
extern HANDLE g_LoaderMemoryModule_hThread;

#endif
