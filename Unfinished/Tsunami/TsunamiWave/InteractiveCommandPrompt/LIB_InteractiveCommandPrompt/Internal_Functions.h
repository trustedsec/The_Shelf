#pragma once
#ifndef __LIB_Internal_Functions__
#define __LIB_Internal_Functions__

#include <Windows.h>

/******************************************************************************
	Kernel32 typedefs, defines, functions
******************************************************************************/
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
//
// Use CloseHandle to destroy the snapshot
//

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


#define SZ_InteractiveCommandPrompt_Kernel32 "Kernel32.dll"
static HMODULE g_InteractiveCommandPrompt_hKernel32 = NULL;

#define SZ_InteractiveCommandPrompt_CreateProcessA "CreateProcessA"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_CreateProcessA)(
	__in_opt    LPCSTR lpApplicationName,
	__inout_opt LPSTR lpCommandLine,
	__in_opt    LPSECURITY_ATTRIBUTES lpProcessAttributes,
	__in_opt    LPSECURITY_ATTRIBUTES lpThreadAttributes,
	__in        BOOL bInheritHandles,
	__in        DWORD dwCreationFlags,
	__in_opt    LPVOID lpEnvironment,
	__in_opt    LPCSTR lpCurrentDirectory,
	__in        LPSTARTUPINFOA lpStartupInfo,
	__out       LPPROCESS_INFORMATION lpProcessInformation
);
extern f_InteractiveCommandPrompt_CreateProcessA m_InteractiveCommandPrompt_CreateProcessA;

#define SZ_InteractiveCommandPrompt_TerminateProcess "TerminateProcess"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_TerminateProcess)(
	__in HANDLE hProcess,
	__in UINT uExitCode
);
extern f_InteractiveCommandPrompt_TerminateProcess m_InteractiveCommandPrompt_TerminateProcess;

#define SZ_InteractiveCommandPrompt_CreatePipe "CreatePipe"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_CreatePipe)(
	__out_ecount_full(1) PHANDLE hReadPipe,
	__out_ecount_full(1) PHANDLE hWritePipe,
	__in_opt LPSECURITY_ATTRIBUTES lpPipeAttributes,
	__in     DWORD nSize
	);
extern f_InteractiveCommandPrompt_CreatePipe m_InteractiveCommandPrompt_CreatePipe;

#define SZ_InteractiveCommandPrompt_DuplicateHandle "DuplicateHandle"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_DuplicateHandle)(
	__in        HANDLE hSourceProcessHandle,
	__in        HANDLE hSourceHandle,
	__in        HANDLE hTargetProcessHandle,
	__deref_out LPHANDLE lpTargetHandle,
	__in        DWORD dwDesiredAccess,
	__in        BOOL bInheritHandle,
	__in        DWORD dwOptions
	);
extern f_InteractiveCommandPrompt_DuplicateHandle m_InteractiveCommandPrompt_DuplicateHandle;

#define SZ_InteractiveCommandPrompt_PeekNamedPipe "PeekNamedPipe"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_PeekNamedPipe)(
	__in      HANDLE hNamedPipe,
	__out_bcount_part_opt(nBufferSize, *lpBytesRead) LPVOID lpBuffer,
	__in      DWORD nBufferSize,
	__out_opt LPDWORD lpBytesRead,
	__out_opt LPDWORD lpTotalBytesAvail,
	__out_opt LPDWORD lpBytesLeftThisMessage
	);
extern f_InteractiveCommandPrompt_PeekNamedPipe m_InteractiveCommandPrompt_PeekNamedPipe;

#define SZ_InteractiveCommandPrompt_GetExitCodeProcess "GetExitCodeProcess"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_GetExitCodeProcess)(
	__in  HANDLE hProcess,
	__out LPDWORD lpExitCode
	);
extern f_InteractiveCommandPrompt_GetExitCodeProcess m_InteractiveCommandPrompt_GetExitCodeProcess;

#define SZ_InteractiveCommandPrompt_OpenProcessToken "OpenProcessToken"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_OpenProcessToken)(
	__in        HANDLE ProcessHandle,
	__in        DWORD DesiredAccess,
	__deref_out PHANDLE TokenHandle
	);
extern f_InteractiveCommandPrompt_OpenProcessToken m_InteractiveCommandPrompt_OpenProcessToken;

#define SZ_InteractiveCommandPrompt_InitializeProcThreadAttributeList "InitializeProcThreadAttributeList"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_InitializeProcThreadAttributeList)(
	__out_opt LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	__in DWORD dwAttributeCount,
	__in DWORD dwFlags,
	__out_opt PSIZE_T lpSize
	);
extern f_InteractiveCommandPrompt_InitializeProcThreadAttributeList m_InteractiveCommandPrompt_InitializeProcThreadAttributeList;

#define SZ_InteractiveCommandPrompt_UpdateProcThreadAttribute "UpdateProcThreadAttribute"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_UpdateProcThreadAttribute)(
	__inout LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList,
	__in DWORD dwFlags,
	__in DWORD_PTR Attribute,
	__in_bcount_opt(cbSize) PVOID lpValue,
	__in SIZE_T cbSize,
	__out_bcount_opt(cbSize) PVOID lpPreviousValue,
	__in_opt PSIZE_T lpReturnSize
	);
extern f_InteractiveCommandPrompt_UpdateProcThreadAttribute m_InteractiveCommandPrompt_UpdateProcThreadAttribute;

#define SZ_InteractiveCommandPrompt_DeleteProcThreadAttributeList "DeleteProcThreadAttributeList"
typedef VOID(WINAPI* f_InteractiveCommandPrompt_DeleteProcThreadAttributeList)(
	__inout LPPROC_THREAD_ATTRIBUTE_LIST lpAttributeList
	);
extern f_InteractiveCommandPrompt_DeleteProcThreadAttributeList m_InteractiveCommandPrompt_DeleteProcThreadAttributeList;

#define SZ_InteractiveCommandPrompt_CreateToolhelp32Snapshot "CreateToolhelp32Snapshot"
typedef HANDLE(WINAPI* f_InteractiveCommandPrompt_CreateToolhelp32Snapshot)(
	DWORD dwFlags,
	DWORD th32ProcessID
	);
extern f_InteractiveCommandPrompt_CreateToolhelp32Snapshot m_InteractiveCommandPrompt_CreateToolhelp32Snapshot;

#define SZ_InteractiveCommandPrompt_Process32First "Process32First"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_Process32First)(
	HANDLE hSnapshot,
	LPPROCESSENTRY32 lppe
	);
extern f_InteractiveCommandPrompt_Process32First m_InteractiveCommandPrompt_Process32First;

#define SZ_InteractiveCommandPrompt_Process32Next "Process32Next"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_Process32Next)(
	HANDLE hSnapshot,
	LPPROCESSENTRY32 lppe
	);
extern f_InteractiveCommandPrompt_Process32Next m_InteractiveCommandPrompt_Process32Next;



DWORD Initialize_InteractiveCommandPrompt_Kernel32_Functions();
DWORD Finalize_InteractiveCommandPrompt_Kernel32_Functions();






/******************************************************************************
	Advapi32 typedefs, defines, functions
******************************************************************************/
#define SZ_InteractiveCommandPrompt_Advapi32 "Advapi32.dll"
static HMODULE g_InteractiveCommandPrompt_hAdvapi32 = NULL;

#define SZ_InteractiveCommandPrompt_LookupPrivilegeValueA "LookupPrivilegeValueA"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_LookupPrivilegeValueA)(
	__in_opt LPCSTR lpSystemName,
	__in     LPCSTR lpName,
	__out    PLUID   lpLuid
	);
extern f_InteractiveCommandPrompt_LookupPrivilegeValueA m_InteractiveCommandPrompt_LookupPrivilegeValueA;

#define SZ_InteractiveCommandPrompt_AdjustTokenPrivileges "AdjustTokenPrivileges"
typedef BOOL(WINAPI* f_InteractiveCommandPrompt_AdjustTokenPrivileges)(
	__in      HANDLE TokenHandle,
	__in      BOOL DisableAllPrivileges,
	__in_opt  PTOKEN_PRIVILEGES NewState,
	__in      DWORD BufferLength,
	__out_bcount_part_opt(BufferLength, *ReturnLength) PTOKEN_PRIVILEGES PreviousState,
	__out_opt PDWORD ReturnLength
	);
extern f_InteractiveCommandPrompt_AdjustTokenPrivileges m_InteractiveCommandPrompt_AdjustTokenPrivileges;

DWORD Initialize_InteractiveCommandPrompt_Advapi32_Functions();
DWORD Finalize_InteractiveCommandPrompt_Advapi32_Functions();


#endif
