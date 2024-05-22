#pragma once
#ifndef __DLL_Utilities__
#define __DLL_Utilities__

//#include "LIB_Utilities.h"

//extern "C" __declspec(dllexport) DWORD __cdecl DLL_Utilities_ExportedFunction(void);
extern "C" __declspec(dllexport) DWORD __cdecl ALLOC(IN DWORD dwSize, OUT LPVOID* lpBuffer);

#endif
