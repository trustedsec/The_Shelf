#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <winsock2.h>
#include <iphlpapi.h>

#include <Windows.h>



/******************************************************************************
	Ws2_32 typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyNetwork_Ws2_32 "Ws2_32.dll"
extern HMODULE g_SurveyNetwork_hWs2_32;

#define SZ_SurveyNetwork_WSAStartup "WSAStartup"
typedef
int
(WSAAPI*
f_SurveyNetwork_WSAStartup)(
	__in WORD wVersionRequested,
	__out LPWSADATA lpWSAData
);
extern f_SurveyNetwork_WSAStartup m_SurveyNetwork_WSAStartup;

#define SZ_SurveyNetwork_WSACleanup "WSACleanup"
typedef
int
(WSAAPI*
f_SurveyNetwork_WSACleanup)(
	void
);
extern f_SurveyNetwork_WSACleanup m_SurveyNetwork_WSACleanup;

#define SZ_SurveyNetwork_gethostname "gethostname"
typedef
int
(WSAAPI*
f_SurveyNetwork_gethostname)(
	__out_bcount(namelen) char FAR * name,
	__in int namelen
);
extern f_SurveyNetwork_gethostname m_SurveyNetwork_gethostname;


DWORD Initialize_SurveyNetwork_Ws2_32_Functions();
DWORD Finalize_SurveyNetwork_Ws2_32_Functions();



/******************************************************************************
	Iphlpapi typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyNetwork_Iphlpapi "Iphlpapi.dll"
extern HMODULE g_hIphlpapi;

#define SZ_SurveyNetwork_GetAdaptersAddresses "GetAdaptersAddresses"
typedef
ULONG
(WINAPI*
f_SurveyNetwork_GetAdaptersAddresses)(
	__in ULONG Family,
	__in ULONG Flags,
	__reserved PVOID Reserved,
	__out_bcount_opt(*SizePointer) PIP_ADAPTER_ADDRESSES AdapterAddresses,
	__inout PULONG SizePointer
);
extern f_SurveyNetwork_GetAdaptersAddresses m_SurveyNetwork_GetAdaptersAddresses;


DWORD InitializeIphlpapiFunctions();
DWORD FinalizeIphlpapiFunctions();




#endif
