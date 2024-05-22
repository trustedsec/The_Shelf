#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_LoadLibrary.h"
#include "Internal_Loader.h"


// Global variables
cJSON* g_lpJsonLoadLibraryConfig = NULL;
HMODULE HMODULE_KERNEL32 = NULL;
f_VirtualProtect m_VirtualProtect = NULL;
SHELLCODE_CTX *ctx = NULL;


/*
 * Find library name from given unicode string
 */
int find_string(SHELLCODE_CTX *ctx, UNICODE_STRING *str)
{
	int i, j;

	for (i = 0; i < str->Length; i++)
	{
		for (j = 0; j < ctx->liblen; j++)
		{
			if (str->Buffer[i + j] != ctx->libname[j])
				break;
		}

		/* Match */
		if (j == ctx->liblen)
			return 0;
	}

	return 1;
}



/* NtOpenSection hook */
NTSTATUS NTAPI m_NtOpenSection(
	PHANDLE SectionHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes)
{
	/* Find our context */
	if (!find_string(ctx, ObjectAttributes->ObjectName))
	{
		*SectionHandle = (PHANDLE)ctx->mapped_address;
		return STATUS_SUCCESS;
	}

	return ctx->p_NtOpenSection(SectionHandle, DesiredAccess,
		ObjectAttributes);
}

/* NtQueryAttributesFile hook */
NTSTATUS NTAPI m_NtQueryAttributesFile(
	POBJECT_ATTRIBUTES ObjectAttributes,
	PFILE_BASIC_INFORMATION FileAttributes)
{
	if (!find_string(ctx, ObjectAttributes->ObjectName))
	{
		/*
		 * struct PFILE_BASIC_INFORMATION must be actually filled
		 * with something sane, otherwise it might break something.
		 * The values are defined in libloader.h
		 *
		 */
		FileAttributes->CreationTime.LowPart = LOW_TIME_1;
		FileAttributes->CreationTime.HighPart = HIGH_TIME;
		FileAttributes->LastAccessTime.LowPart = LOW_TIME_2;
		FileAttributes->LastAccessTime.HighPart = HIGH_TIME;
		FileAttributes->LastWriteTime.LowPart = LOW_TIME_1;
		FileAttributes->LastWriteTime.HighPart = HIGH_TIME;
		FileAttributes->ChangeTime.LowPart = LOW_TIME_1;
		FileAttributes->ChangeTime.HighPart = HIGH_TIME;
		FileAttributes->FileAttributes = FILE_ATTRIBUTE_NORMAL;
		return STATUS_SUCCESS;
	}

	return ctx->p_NtQueryAttributesFile(ObjectAttributes, FileAttributes);
}

/* NtOpenFile hook */
void NTAPI m_NtOpenFile(
	PHANDLE FileHandle,
	ACCESS_MASK DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PIO_STATUS_BLOCK IoStatusBlock,
	ULONG ShareAccess,
	ULONG OpenOptions)
{
	if (!find_string(ctx, ObjectAttributes->ObjectName))
	{
		*FileHandle = (PVOID)ctx->mapped_address;
		return;
	}

	ctx->p_NtOpenFile(
		FileHandle,
		DesiredAccess,
		ObjectAttributes,
		IoStatusBlock,
		ShareAccess,
		OpenOptions);

	return;
}

/* NtCreateSection hook */
NTSTATUS NTAPI m_NtCreateSection(
	PHANDLE SectionHandle,
	ULONG DesiredAccess,
	POBJECT_ATTRIBUTES ObjectAttributes,
	PLARGE_INTEGER MaximumSize,
	ULONG PageAttributes,
	ULONG SectionAttributes,
	HANDLE FileHandle)
{
	if (FileHandle == (HANDLE)ctx->mapped_address)
	{
		*SectionHandle = (PVOID)ctx->mapped_address;
		return STATUS_SUCCESS;
	}

	return ctx->p_NtCreateSection(
		SectionHandle,
		DesiredAccess,
		ObjectAttributes,
		MaximumSize,
		PageAttributes,
		SectionAttributes,
		FileHandle);
}


/* NtMapViewOfSection hook */
NTSTATUS NTAPI m_NtMapViewOfSection(
	HANDLE SectionHandle,
	HANDLE ProcessHandle,
	PVOID *BaseAddress,
	ULONG ZeroBits,
	ULONG CommitSize,
	PLARGE_INTEGER SectionOffset,
	PULONG ViewSize,
	SECTION_INHERIT InheritDisposition,
	ULONG AllocationType,
	ULONG Protect)
{
	if (SectionHandle == (HANDLE)ctx->mapped_address)
	{
		*BaseAddress = (PVOID)ctx->mapped_address;
		*ViewSize = ctx->size_map;

		/* We assume that the image must be relocated */
		return STATUS_IMAGE_NOT_AT_BASE;
	}

	return ctx->p_NtMapViewOfSection(
		SectionHandle,
		ProcessHandle,
		BaseAddress,
		ZeroBits,
		CommitSize,
		SectionOffset,
		ViewSize,
		InheritDisposition,
		AllocationType,
		Protect);
}

/* NtClose hook */
NTSTATUS NTAPI m_NtClose(
	HANDLE Handle)
{

	if (Handle == (HANDLE)ctx->mapped_address)
	{
		return STATUS_SUCCESS;
	}

	return ctx->p_NtClose(Handle);
}

/* Patch given function */
void patch_function(
	SHELLCODE_CTX *ctx, 
	DWORD address, 
	unsigned char *stub, 
	unsigned char *hook
)
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD				protect;
	ULONG 				bytes;
	SIZE_T              written;
	MEMORY_BASIC_INFORMATION	mbi_thunk;

	/*
	 * Most native NT functions begin with stub like this:
	 *
	 * 00000000  B82B000000        mov eax,0x2b         ; syscall
	 * 00000005  8D542404          lea edx,[esp+0x4]    ; arguments
	 * 00000009  CD2E              int 0x2e             ; interrupt
	 *
	 * In offset 0, the actual system call is saved in eax. Syscall
	 * is 32 bit number (!) so we can assume 5 bytes of preamble size
	 * for each function.. If there's need to hook other functions,
	 * a complete disassembler is needed for preamble size counting.
	 *
	 */
	bytes = 5;

	/* Create the stub */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("WriteProcessMemory( %p, %p, %p, %d, %p )..."),
		(HANDLE)-1,
		stub,
		(char *)address,
		bytes,
		&written
	);
	if (FALSE == WriteProcessMemory(
		(HANDLE)-1,
		stub,
		(char *)address,
		bytes,
		&written
	)
		)
	{
		dwErrorCode = ERROR_BAD_ARGUMENTS;
		DBGPRINT(
			DEBUG_ERROR,
			TEXT("WriteProcessMemory( %p, %p, %p, %d, %p ) failed (%08x).\n"),
			(HANDLE)-1,
			stub,
			(char *)address,
			bytes,
			&written,
			dwErrorCode
		);
		return;
	}
	*(PBYTE)(stub + bytes) = 0xE9;
	*(DWORD *)(stub + bytes + 1) = (DWORD)address - ((DWORD)stub + 5);


	/* Patch original function */

	/* Fix protection */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("VirtualQuery( %p, %p, %d )..."),
		(char *)address,
		&mbi_thunk,
		sizeof(MEMORY_BASIC_INFORMATION)
	);
	if (FALSE == VirtualQuery(
		(char *)address, 
		&mbi_thunk,
		sizeof(MEMORY_BASIC_INFORMATION)
	)
		)
	{
		dwErrorCode = ERROR_BAD_ARGUMENTS;
		DBGPRINT(
			DEBUG_ERROR,
			TEXT("VirtualQuery( %p, %p, %d ) failed (%08x).\n"),
			(char *)address,
			&mbi_thunk,
			sizeof(MEMORY_BASIC_INFORMATION),
			dwErrorCode
		);
		return;
	}
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("VirtualProtect( %p, %d, %08x, %p )..."),
		mbi_thunk.BaseAddress,
		mbi_thunk.RegionSize,
		PAGE_EXECUTE_READWRITE,
		&mbi_thunk.Protect
	);
	if ( FALSE == VirtualProtect(
		mbi_thunk.BaseAddress, 
		mbi_thunk.RegionSize,
		PAGE_EXECUTE_READWRITE, 
		&mbi_thunk.Protect
	)
		)
	{
		dwErrorCode = ERROR_BAD_ARGUMENTS;
		DBGPRINT(
			DEBUG_ERROR,
			TEXT("VirtualProtect( %p, %d, %08x, %p ) failed (%08x).\n"),
			mbi_thunk.BaseAddress,
			mbi_thunk.RegionSize,
			PAGE_EXECUTE_READWRITE,
			&mbi_thunk.Protect,
			dwErrorCode
		);
		return;
	}

	/* Insert jump */
	*(PBYTE)address = 0xE9;
	*(DWORD *)(address + 1) = (DWORD)hook - ((DWORD)address + 5);


	/* Restore protection */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("VirtualProtect( %p, %d, %08x, %p )..."),
		mbi_thunk.BaseAddress,
		mbi_thunk.RegionSize,
		mbi_thunk.Protect,
		&protect
	);
	if (FALSE == VirtualProtect(
		mbi_thunk.BaseAddress, 
		mbi_thunk.RegionSize,
		mbi_thunk.Protect, 
		&protect
	)
		)
	{
		dwErrorCode = ERROR_BAD_ARGUMENTS;
		DBGPRINT(
			DEBUG_ERROR,
			TEXT("VirtualProtect( %p, %d, %08x, %p ) failed (%08x).\n"),
			mbi_thunk.BaseAddress,
			mbi_thunk.RegionSize,
			mbi_thunk.Protect,
			&protect,
			dwErrorCode
		);
		return;
	}
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("VirtualProtect( %p, %p, %d )..."),
		(HANDLE) - 1,
		mbi_thunk.BaseAddress,
		mbi_thunk.RegionSize
	);
	FlushInstructionCache(
		(HANDLE)-1, 
		mbi_thunk.BaseAddress,
		mbi_thunk.RegionSize
	);

}

/* Install hooks, fix addresses */
void install_hooks(SHELLCODE_CTX *ctx)
{
	f_NtMapViewOfSection lNtMapViewOfSection;
	f_NtQueryAttributesFile lNtQueryAttributesFile;
	f_NtOpenFile lNtOpenFile;
	f_NtCreateSection lNtCreateSection;
	f_NtOpenSection lNtOpenSection;
	f_NtClose lNtClose;
	HMODULE ntdll;

	if (!(ntdll = LoadLibrary(TEXT("ntdll"))))
	{
		return;
	}
	DBGPRINT(DEBUG_VERBOSE, TEXT("ntdll:               %p"), ntdll);


	lNtMapViewOfSection = (f_NtMapViewOfSection)GetProcAddress(ntdll, "NtMapViewOfSection");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtMapViewOfSection:    %p"), lNtMapViewOfSection);
	lNtQueryAttributesFile = (f_NtQueryAttributesFile)GetProcAddress(ntdll, "NtQueryAttributesFile");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtQueryAttributesFile: %p"), lNtQueryAttributesFile);
	lNtOpenFile = (f_NtOpenFile)GetProcAddress(ntdll, "NtOpenFile");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtOpenFile:            %p"), lNtOpenFile);
	lNtCreateSection = (f_NtCreateSection)GetProcAddress(ntdll, "NtCreateSection");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtCreateSection:       %p"), lNtCreateSection);
	lNtOpenSection = (f_NtOpenSection)GetProcAddress(ntdll, "NtOpenSection");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtOpenSection:         %p"), lNtOpenSection);
	lNtClose = (f_NtClose)GetProcAddress(ntdll, "NtClose");
	DBGPRINT(DEBUG_VERBOSE, TEXT("lNtClose:               %p"), lNtClose);

	/* NtMapViewOfSection */
	DBGPRINT(
		DEBUG_VERBOSE, 
		TEXT("patch_function( %p, %08x, %p, %p )..."), 
		ctx,
		(DWORD)lNtMapViewOfSection,
		ctx->s_NtMapViewOfSection,
		(unsigned char *)m_NtMapViewOfSection
	);
	patch_function(
		ctx, 
		(DWORD)lNtMapViewOfSection,
		ctx->s_NtMapViewOfSection,
		(unsigned char *)m_NtMapViewOfSection
	);
	ctx->p_NtMapViewOfSection =
		(f_NtMapViewOfSection)((LPVOID)(ctx->s_NtMapViewOfSection));

	/* NtQueryAttributesFile */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("patch_function( %p, %08x, %p, %p )..."),
		ctx,
		(DWORD)lNtQueryAttributesFile,
		ctx->s_NtQueryAttributesFile,
		(unsigned char *)m_NtQueryAttributesFile
	);
	patch_function(
		ctx, 
		(DWORD)lNtQueryAttributesFile,
		ctx->s_NtQueryAttributesFile,
		(unsigned char *)m_NtQueryAttributesFile
	);
	ctx->p_NtQueryAttributesFile =
		(f_NtQueryAttributesFile)((LPVOID)(ctx->s_NtQueryAttributesFile));

	/* NtOpenFile */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("patch_function( %p, %08x, %p, %p )..."),
		ctx,
		(DWORD)lNtOpenFile,
		ctx->s_NtOpenFile,
		(unsigned char *)m_NtOpenFile
	);
	patch_function(
		ctx, 
		(DWORD)lNtOpenFile, 
		ctx->s_NtOpenFile,
		(unsigned char *)m_NtOpenFile
	);
	ctx->p_NtOpenFile = (f_NtOpenFile)((LPVOID)(ctx->s_NtOpenFile));

	/* NtCreateSection */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("patch_function( %p, %08x, %p, %p )..."),
		ctx,
		(DWORD)lNtCreateSection,
		ctx->s_NtCreateSection,
		(unsigned char *)m_NtCreateSection
	);
	patch_function(
		ctx, 
		(DWORD)lNtCreateSection, 
		ctx->s_NtCreateSection,
		(unsigned char *)m_NtCreateSection
	);
	ctx->p_NtCreateSection = (f_NtCreateSection)((LPVOID)(ctx->s_NtCreateSection));

	/* NtOpenSection */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("patch_function( %p, %08x, %p, %p )..."),
		ctx,
		(DWORD)lNtOpenSection,
		ctx->s_NtOpenSection,
		(unsigned char *)m_NtOpenSection
	);
	patch_function(
		ctx, 
		(DWORD)lNtOpenSection, 
		ctx->s_NtOpenSection,
		(unsigned char *)m_NtOpenSection
	);
	ctx->p_NtOpenSection = (f_NtOpenSection)((LPVOID)(ctx->s_NtOpenSection));

	/* NtClose */
	DBGPRINT(
		DEBUG_VERBOSE,
		TEXT("patch_function( %p, %08x, %p, %p )..."),
		ctx,
		(DWORD)lNtClose,
		ctx->s_NtClose,
		(unsigned char *)m_NtClose
	);
	patch_function(
		ctx, 
		(DWORD)lNtClose, 
		ctx->s_NtClose,
		(unsigned char *)m_NtClose
	);
	ctx->p_NtClose = (f_NtClose)((LPVOID)(ctx->s_NtClose));

}

/* Restore given function */
void restore_function(SHELLCODE_CTX *ctx, DWORD address, unsigned char *stub)
{
	DWORD				protect;
	ULONG 				bytes;
	SIZE_T              written;
	MEMORY_BASIC_INFORMATION	mbi_thunk;

	bytes = 5;

	/* Patch original function */

	/* Fix protection */
	VirtualQuery((char *)address, &mbi_thunk,
		sizeof(MEMORY_BASIC_INFORMATION));
	VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize,
		PAGE_EXECUTE_READWRITE, &mbi_thunk.Protect);

	/* Copy bytes back to function */
	WriteProcessMemory((HANDLE)-1, (char *)address, stub,
		bytes, &written);

	/* Restore protection */
	VirtualProtect(mbi_thunk.BaseAddress, mbi_thunk.RegionSize,
		mbi_thunk.Protect, &protect);
	FlushInstructionCache((HANDLE)-1, mbi_thunk.BaseAddress,
		mbi_thunk.RegionSize);

}

/* Remove hooks */
void remove_hooks(SHELLCODE_CTX *ctx)
{
	f_NtMapViewOfSection lNtMapViewOfSection;
	f_NtQueryAttributesFile lNtQueryAttributesFile;
	f_NtOpenFile lNtOpenFile;
	f_NtCreateSection lNtCreateSection;
	f_NtOpenSection lNtOpenSection;
	f_NtClose lNtClose;
	HMODULE ntdll;

	if (!(ntdll = LoadLibraryA("ntdll")))
	{
		return;
	}

	lNtMapViewOfSection = (f_NtMapViewOfSection)GetProcAddress(ntdll, "NtMapViewOfSection");
	lNtQueryAttributesFile = (f_NtQueryAttributesFile)GetProcAddress(ntdll, "NtQueryAttributesFile");
	lNtOpenFile = (f_NtOpenFile)GetProcAddress(ntdll, "NtOpenFile");
	lNtCreateSection = (f_NtCreateSection)GetProcAddress(ntdll, "NtCreateSection");
	lNtOpenSection = (f_NtOpenSection)GetProcAddress(ntdll, "NtOpenSection");
	lNtClose = (f_NtClose)GetProcAddress(ntdll, "NtClose");

	/* NtMapViewOfSection */
	restore_function(ctx, (DWORD)lNtMapViewOfSection,
		ctx->s_NtMapViewOfSection);

	/* NtQueryAttributesFile */
	restore_function(ctx, (DWORD)lNtQueryAttributesFile,
		ctx->s_NtQueryAttributesFile);

	/* NtOpenFile */
	restore_function(ctx, (DWORD)lNtOpenFile, ctx->s_NtOpenFile);

	/* NtCreateSection */
	restore_function(ctx, (DWORD)lNtCreateSection, ctx->s_NtCreateSection);

	/* NtOpenSection */
	restore_function(ctx, (DWORD)lNtOpenSection, ctx->s_NtOpenSection);

	/* NtClose */
	restore_function(ctx, (DWORD)lNtClose, ctx->s_NtClose);
}

/* Map file in memory as section */
void map_file(SHELLCODE_CTX *ctx)
{
	PIMAGE_NT_HEADERS 	nt;
	PIMAGE_DOS_HEADER 	dos;
	PIMAGE_SECTION_HEADER	sect;
	int			i;

	dos = (PIMAGE_DOS_HEADER)ctx->file_address;
	nt = (PIMAGE_NT_HEADERS)(ctx->file_address + dos->e_lfanew);

	/*
	 * Allocate space for the mapping
	 * First, try to map the file at ImageBase
	 *
	 */
	ctx->mapped_address = (DWORD)VirtualAlloc(
		(PVOID)nt->OptionalHeader.ImageBase,
		nt->OptionalHeader.SizeOfImage,
		MEM_RESERVE | MEM_COMMIT, 
		PAGE_EXECUTE_READWRITE
	);


	/* No success, let the system decide..  */
	if (ctx->mapped_address == 0) 
	{
		ctx->mapped_address = (DWORD)VirtualAlloc(
			(PVOID)NULL,
			nt->OptionalHeader.SizeOfImage,
			MEM_RESERVE | MEM_COMMIT, 
			PAGE_EXECUTE_READWRITE
		);
	}

	ctx->size_map = nt->OptionalHeader.SizeOfImage;

	/* Lock the mapping in memory */
	{
		ULONG(_stdcall *NtLockVirtualMemory)(HANDLE, PVOID *, PULONG, ULONG);

		NtLockVirtualMemory = (ULONG(_stdcall *)(HANDLE, PVOID *, PULONG, ULONG))GetProcAddress(
			GetModuleHandleA("ntdll"),
			"NtLockVirtualMemory"
		);

		if (NtLockVirtualMemory)
		{
			PVOID base = (PVOID)ctx->mapped_address;
			ULONG sz = nt->OptionalHeader.SizeOfImage;

			NtLockVirtualMemory(
				(HANDLE)-1,
				&base,
				&sz,
				1
			);
		}
	}

	/* Write headers */
	WriteProcessMemory(
		(HANDLE)-1, 
		(LPVOID)ctx->mapped_address,
		(LPVOID)ctx->file_address, 
		nt->OptionalHeader.SizeOfHeaders, 
		0
	);

	/* Write sections */
	sect = IMAGE_FIRST_SECTION(nt);
	for (i = 0; i < nt->FileHeader.NumberOfSections; i++) {
		WriteProcessMemory(
			(HANDLE)-1,
			(PCHAR)ctx->mapped_address + sect[i].VirtualAddress,
			(PCHAR)ctx->file_address + sect[i].PointerToRawData,
			sect[i].SizeOfRawData, 
			0
		);
	}

}





DWORD InitializeLoaderFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	FARPROC fpFunctionPointer = NULL;

	__try
	{
		__try
		{
			if (NULL == HMODULE_KERNEL32)
			{
				HMODULE_KERNEL32 = LoadLibraryA(
					"kernel32.dll"
				);
				if (NULL == HMODULE_KERNEL32)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("HMODULE_KERNEL32:            %p"), HMODULE_KERNEL32);

				fpFunctionPointer = GetProcAddress(
					HMODULE_KERNEL32,
					"VirtualProtect"
				);
				if (NULL == fpFunctionPointer)
				{
					dwErrorCode = GetLastError();
					__leave;
				}
				m_VirtualProtect = (f_VirtualProtect)fpFunctionPointer;
				DBGPRINT(DEBUG_VERBOSE, TEXT("m_VirtualProtect:              %p"), m_VirtualProtect);

				
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}

DWORD FinalizeLoaderFunctions()
{
	DWORD	dwErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			if (NULL != HMODULE_KERNEL32)
			{
				FreeLibrary(HMODULE_KERNEL32);
				HMODULE_KERNEL32 = NULL;
			}
		} // end try-finally
		__finally
		{

		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();

	} // end except

	return dwErrorCode;
}


DWORD WINAPI LoadLibraryInitialization(
	IN	FARPROC fpRegisterCallback,
	IN	LPBYTE	lpInitializationBuffer,
	IN	DWORD	dwInitializationBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szJsonInput = NULL;
	DWORD	dwJsonInputSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("fpRegisterCallback:         %p"), fpRegisterCallback);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInitializationBuffer:     %p"), lpInitializationBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitializationBufferSize: %d"), dwInitializationBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (
				(NULL == lpInitializationBuffer) ||
				(0 == dwInitializationBufferSize)
				)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInitializationBuffer is NULL")
				);
				__leave;
			}
			szJsonInput = (char*)lpInitializationBuffer;
			dwJsonInputSize = dwInitializationBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			if (NULL != szJsonInput)
			{
				g_lpJsonLoadLibraryConfig = cJSON_Parse(szJsonInput);
				if (NULL == g_lpJsonLoadLibraryConfig)
				{
					dwErrorCode = ERROR_BAD_ARGUMENTS;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_Parse(%p) failed."),
						lpInitializationBuffer
					);
					__leave;
				}
			}


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeLoaderFunctions()"));
			dwErrorCode = InitializeLoaderFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeLoaderFunctions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			if (NULL != fpRegisterCallback)
			{
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("fpRegisterCallback( %p, %08x )"),
					(FARPROC)LoadLibraryCallbackInterface,
					(DWORD)TSM_LOADER_LOAD_DLL_REQUEST
				);
				dwErrorCode = ((_RegisterCallback)fpRegisterCallback)(
					(FARPROC)LoadLibraryCallbackInterface,
					(DWORD)TSM_LOADER_LOAD_DLL_REQUEST
					);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("fpRegisterCallback( %p, %08x ) failed.(%08x)"),
						(FARPROC)LoadLibraryCallbackInterface,
						(DWORD)TSM_LOADER_LOAD_DLL_REQUEST,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(
					DEBUG_INFO,
					TEXT("fpRegisterCallback( %p, %08x ) was successful"),
					(FARPROC)LoadLibraryCallbackInterface,
					(DWORD)TSM_LOADER_LOAD_DLL_REQUEST
				);
			}
			else
			{
				DBGPRINT(DEBUG_INFO, TEXT("fpRegisterCallback is NULL"));
			}

		} // end try-finally
		__finally
		{

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI LoadLibraryFinalization(
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_lpJsonLoadLibraryConfig)
			{
				cJSON_Delete(g_lpJsonLoadLibraryConfig);
				g_lpJsonLoadLibraryConfig = NULL;
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("FinalizeLoaderFunctions()"));
			dwErrorCode = FinalizeLoaderFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("FinalizeLoaderFunctions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


		} // end try-finally
		__finally
		{


			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}



DWORD WINAPI LoadLibraryCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szJsonInput = NULL;
	DWORD	dwJsonInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;
	cJSON*	lpJsonRequestEntries = NULL;
	cJSON*	lpJsonRequestEntry = NULL;

	BOOL	bUsingLocalConfig = FALSE;

	CHAR*	szJsonOutput = NULL;
	DWORD	dwJsonOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonOutputResults = NULL;
	
	DWORD	dwRequestEntry = 0;
	
	__try
	{
		__try
		{
			/********************************************************************************
				Check arguments
			********************************************************************************/
			if (NULL == lppOutputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lppOutputBuffer is NULL.\n")
				);
				__leave;
			}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}
			szJsonInput = (char*)lpInputBuffer;
			dwJsonInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szJsonInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					lpInputBuffer
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonOutput = cJSON_CreateObject();
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}
			// Create the message type for the JSON object
			if (NULL == cJSON_AddNumberToObject(lpJsonOutput, TSM_MESSAGE_TYPE, TSM_LOADER_LOAD_DLL_RESPONSE))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_AddNumberToObject(%p, %S, %d) failed."),
					lpJsonOutput,
					TSM_MESSAGE_TYPE,
					TSM_LOADER_LOAD_DLL_RESPONSE
				);
				__leave;
			}
			// Create a JSON array of response/result objects
			lpJsonOutputResults = cJSON_AddArrayToObject(lpJsonOutput, LoadLibrary_Response_Results);
			if (NULL == lpJsonOutputResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutputResults = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonOutput,
					LoadLibrary_Response_Results
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the request message type
			lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(lpJsonInput, TSM_MESSAGE_TYPE);
			if (NULL == lpJsonMessageType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonMessageType = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					TSM_MESSAGE_TYPE
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonMessageType))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonMessageType
				);
				__leave;
			}
			// Check the message type for the correct response
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonMessageType: %d (%08x)"), lpJsonMessageType->valueint, lpJsonMessageType->valueint);
			if (TSM_LOADER_LOAD_DLL_REQUEST != lpJsonMessageType->valueint)
			{
				dwErrorCode = ERROR_UNSUPPORTED_TYPE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("Invalid message type: %d (%08x)"),
					lpJsonMessageType->valueint,
					lpJsonMessageType->valueint
				);
				__leave;
			}

			// Get the array of request queries/entries
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S)"),
				lpJsonInput,
				LoadLibrary_Request_Entries
			);
			lpJsonRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonInput, LoadLibrary_Request_Entries);
			if (NULL == lpJsonRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					LoadLibrary_Request_Entries
				);
				__leave;
			}
			// Loop through each request entry and perform the actual request
			cJSON_ArrayForEach(lpJsonRequestEntry, lpJsonRequestEntries)
			{
				cJSON*	lpJsonLibraryBuffer = NULL;
				cJSON*	lpJsonEntry = NULL;
				cJSON*	lpJsonModuleHandle = NULL;
				
				LPBYTE	lpDllFileBuffer = NULL;
				DWORD	dwDllFileBufferSize = 0;

				LPCSTR name = "user32.dll";
				PUCHAR buffer = lpDllFileBuffer;
				DWORD bufferlength = dwDllFileBufferSize;

				LPCSTR shortName = name;
				LPCSTR slash = NULL;
				HMODULE mod = NULL;


				// Get the library buffer
				lpJsonLibraryBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonRequestEntry, LoadLibrary_Request_LibraryBuffer);
				if (NULL == lpJsonLibraryBuffer)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonLibraryBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonRequestEntry,
						LoadLibrary_Request_LibraryBuffer
					);
					__leave;
				}
				if (
					(!cJSON_IsBytes(lpJsonLibraryBuffer)) ||
					(NULL == lpJsonLibraryBuffer->valuestring)
					)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsBytes(%p) failed."),
						lpJsonLibraryBuffer
					);
					__leave;
				}

				lpDllFileBuffer = cJSON_GetBytesValue(lpJsonLibraryBuffer, (int*)(&dwDllFileBufferSize));
				if (NULL == lpDllFileBuffer)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpDllFileBuffer = cJSON_GetBytesValue(%p, %d) failed."),
						lpJsonLibraryBuffer,
						dwDllFileBufferSize
					);
					__leave;
				}


				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]:"), LoadLibrary_Request_Entries, dwRequestEntry );
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:     %p"), LoadLibrary_Request_LibraryBuffer, lpDllFileBuffer);
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%SSize: %d"), LoadLibrary_Request_LibraryBuffer, dwDllFileBufferSize);
				DBGPRINT(
					DEBUG_VERBOSE, 
					TEXT("  ->%S:     %02x %02x %02x %02x"), 
					LoadLibrary_Request_LibraryBuffer, 
					lpDllFileBuffer[0],
					lpDllFileBuffer[1],
					lpDllFileBuffer[2],
					lpDllFileBuffer[3]
				);

				

				if ((slash = strrchr(name, '\\')))
					shortName = slash + 1;

				ctx = (SHELLCODE_CTX *)VirtualAlloc(
					NULL,
					sizeof(SHELLCODE_CTX),
					MEM_COMMIT,
					PAGE_EXECUTE_READWRITE
				);
				if (NULL == ctx)
				{
					dwErrorCode = ERROR_BAD_ARGUMENTS;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("VirtualAlloc( %p, %d, %08x, %08x ) failed (%08x).\n"),
						NULL,
						sizeof(SHELLCODE_CTX),
						MEM_COMMIT,
						PAGE_EXECUTE_READWRITE,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("ctx:        %p"), ctx);

				DBGPRINT(DEBUG_VERBOSE, TEXT("install_hooks(%p)..."), ctx);
				install_hooks(ctx);

				do
				{
					// The name of the library to load it as
					strncpy_s(ctx->libname, sizeof(ctx->libname), shortName, sizeof(ctx->libname) - 1);
					DBGPRINT(DEBUG_VERBOSE, TEXT("ctx->libname: %S"), ctx->libname);
					ctx->liblen = (int)strlen(ctx->libname) + 1;
					DBGPRINT(DEBUG_VERBOSE, TEXT("ctx->liblen:  %d"), ctx->liblen);

					// The address of the raw buffer
					ctx->file_address = (DWORD)buffer;
					DBGPRINT(DEBUG_VERBOSE, TEXT("ctx->libname: %08x"), ctx->file_address);

					// Map the buffer into memory
					DBGPRINT(DEBUG_VERBOSE, TEXT("map_file(%p)..."), ctx);
					map_file(ctx);

					// Load the fake library
					DBGPRINT(DEBUG_VERBOSE, TEXT("LoadLibraryA(%s)..."), ctx->libname);
					if (!(mod = LoadLibraryA(ctx->libname)))
					{
						break;
					}

				} while (0);

				DBGPRINT(DEBUG_VERBOSE, TEXT("remove_hooks(%p)..."), ctx);
				remove_hooks(ctx);

				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("VirtualFree( %p, %d, %08x ) failed (%08x).\n"),
					ctx,
					0,
					MEM_RELEASE,
					dwErrorCode
				);
				VirtualFree(ctx, 0, MEM_RELEASE);

				DBGPRINT(DEBUG_VERBOSE, TEXT("mod:     %p"), mod);

				// Create the JSON results entry
				lpJsonEntry = cJSON_CreateObject();
				if (NULL == lpJsonEntry)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonEntry = cJSON_CreateObject() failed.\n")
					);
					__leave;
				}

				// Create the module handle output
				lpJsonModuleHandle = cJSON_CreateBytes((LPBYTE)(&mod), sizeof(mod));
				if (NULL == lpJsonModuleHandle)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonModuleHandle = cJSON_CreateString failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonEntry, LoadLibrary_Response_ModuleHandle, lpJsonModuleHandle);

				// Add current entry to overall results list
				cJSON_AddItemToArray(lpJsonOutputResults, lpJsonEntry);

				dwRequestEntry++;

			} // end cJSON_ArrayForEach(lpJsonRequestEntry, lpJsonRequestEntries)



			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szJsonOutput = cJSON_Print(lpJsonOutput);
			if (NULL == szJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonOutput
				);
				__leave;
			}
			dwJsonOutputSize = (DWORD)strlen(szJsonOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szJsonOutput;
			(*lpdwOutputBufferSize) = dwJsonOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
			}

			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != (*lppOutputBuffer))
				{
					//dwTempErrorCode = FREE(*lppOutputBuffer);
					//if (ERROR_SUCCESS != dwTempErrorCode)
					//{
					//	DBGPRINT(DEBUG_ERROR, TEXT("FREE failed.(%08x)"), dwTempErrorCode);
					//}

					cJSON_free(szJsonOutput);
					*lppOutputBuffer = NULL;
				}
				*lpdwOutputBufferSize = 0;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


