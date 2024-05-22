#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyProcess_Kernel32 "Kernel32.dll"
extern HMODULE g_SurveyProcess_hKernel32;


#define SZ_SurveyProcess_OpenProcess "OpenProcess"
typedef
HANDLE
(WINAPI*
f_SurveyProcess_OpenProcess)(
	__in DWORD dwDesiredAccess,
	__in BOOL bInheritHandle,
	__in DWORD dwProcessId
);
extern f_SurveyProcess_OpenProcess m_SurveyProcess_OpenProcess;


#define MAX_MODULE_NAME32 255

#define SZ_SurveyProcess_CreateToolhelp32Snapshot "CreateToolhelp32Snapshot"
typedef
HANDLE
(WINAPI*
f_SurveyProcess_CreateToolhelp32Snapshot)(
	DWORD dwFlags,
	DWORD th32ProcessID
);
extern f_SurveyProcess_CreateToolhelp32Snapshot m_SurveyProcess_CreateToolhelp32Snapshot;

//
// The th32ProcessID argument is only used if TH32CS_SNAPHEAPLIST or
// TH32CS_SNAPMODULE is specified. th32ProcessID == 0 means the current
// process.
//
// NOTE that all of the snapshots are global except for the heap and module
//      lists which are process specific. To enumerate the heap or module
//      state for all WIN32 processes call with TH32CS_SNAPALL and the
//      current process. Then for each process in the TH32CS_SNAPPROCESS
//      list that isn't the current process, do a call with just
//      TH32CS_SNAPHEAPLIST and/or TH32CS_SNAPMODULE.
//
// dwFlags
//
#define TH32CS_SNAPHEAPLIST 0x00000001
#define TH32CS_SNAPPROCESS  0x00000002
#define TH32CS_SNAPTHREAD   0x00000004
#define TH32CS_SNAPMODULE   0x00000008
#define TH32CS_SNAPMODULE32 0x00000010
#define TH32CS_SNAPALL      (TH32CS_SNAPHEAPLIST | TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD | TH32CS_SNAPMODULE)
#define TH32CS_INHERIT      0x80000000




typedef struct tagPROCESSENTRY32
{
	DWORD   dwSize;
	DWORD   cntUsage;
	DWORD   th32ProcessID;          // this process
	ULONG_PTR th32DefaultHeapID;
	DWORD   th32ModuleID;           // associated exe
	DWORD   cntThreads;
	DWORD   th32ParentProcessID;    // this process's parent process
	LONG    pcPriClassBase;         // Base priority of process's threads
	DWORD   dwFlags;
	CHAR    szExeFile[MAX_PATH];    // Path
} PROCESSENTRY32;
typedef PROCESSENTRY32 *  PPROCESSENTRY32;
typedef PROCESSENTRY32 *  LPPROCESSENTRY32;

#define SZ_SurveyProcess_Process32First "Process32First"
typedef
BOOL
(WINAPI*
f_SurveyProcess_Process32First)(
	HANDLE hSnapshot,
	LPPROCESSENTRY32 lppe
);
extern f_SurveyProcess_Process32First m_SurveyProcess_Process32First;

#define SZ_SurveyProcess_Process32Next "Process32Next"
typedef
BOOL
(WINAPI*
f_SurveyProcess_Process32Next)(
	HANDLE hSnapshot,
	LPPROCESSENTRY32 lppe
);
extern f_SurveyProcess_Process32Next m_SurveyProcess_Process32Next;

typedef struct tagMODULEENTRY32
{
	DWORD   dwSize;
	DWORD   th32ModuleID;       // This module
	DWORD   th32ProcessID;      // owning process
	DWORD   GlblcntUsage;       // Global usage count on the module
	DWORD   ProccntUsage;       // Module usage count in th32ProcessID's context
	BYTE  * modBaseAddr;        // Base address of module in th32ProcessID's context
	DWORD   modBaseSize;        // Size in bytes of module starting at modBaseAddr
	HMODULE hModule;            // The hModule of this module in th32ProcessID's context
	char    szModule[MAX_MODULE_NAME32 + 1];
	char    szExePath[MAX_PATH];
} MODULEENTRY32;
typedef MODULEENTRY32 *  PMODULEENTRY32;
typedef MODULEENTRY32 *  LPMODULEENTRY32;

#define SZ_SurveyProcess_Module32First "Module32First"
typedef
BOOL
(WINAPI*
f_SurveyProcess_Module32First)(
	HANDLE hSnapshot,
	LPMODULEENTRY32 lpme
);
extern f_SurveyProcess_Module32First m_SurveyProcess_Module32First;

#define SZ_SurveyProcess_Module32Next "Module32Next"
typedef
BOOL
(WINAPI*
f_SurveyProcess_Module32Next)(
	HANDLE hSnapshot,
	LPMODULEENTRY32 lpme
);
extern f_SurveyProcess_Module32Next m_SurveyProcess_Module32Next;


/******************************************************************************
	Internal functions to resolve imports from Kernel32
******************************************************************************/
DWORD Initialize_SurveyProcess_Kernel32_Functions();
DWORD Finalize_SurveyProcess_Kernel32_Functions();



/******************************************************************************
	ntdll typedefs, defines, functions
******************************************************************************/
#define SZ_SurveyProcess_ntdll "ntdll.dll"
extern HMODULE g_SurveyProcess_hntdll;

typedef struct _UNICODE_STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _PROCESS_BASIC_INFORMATION
{
	LONG ExitStatus;
	PVOID PebBaseAddress;
	ULONG_PTR AffinityMask;
	LONG BasePriority;
	ULONG_PTR UniqueProcessId;
	ULONG_PTR ParentProcessId;
} PROCESS_BASIC_INFORMATION, *PPROCESS_BASIC_INFORMATION;


#define SZ_SurveyProcess_NtQueryInformationProcess "NtQueryInformationProcess"
typedef 
NTSTATUS
(NTAPI*
f_SurveyProcess_NtQueryInformationProcess)(
	HANDLE ProcessHandle,
	DWORD ProcessInformationClass,
	PVOID ProcessInformation,
	DWORD ProcessInformationLength,
	PDWORD ReturnLength
);
extern f_SurveyProcess_NtQueryInformationProcess m_SurveyProcess_NtQueryInformationProcess;


/******************************************************************************
	Internal functions to resolve imports from ntdll
******************************************************************************/
DWORD Initialize_SurveyProcess_Ntdll_Functions();
DWORD Finalize_SurveyProcess_Ntdll_Functions();

#endif
