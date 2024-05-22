#pragma once
#ifndef __LIB_Internal_Loader__
#define __LIB_Internal_Loader__

#include <Windows.h>

typedef BOOL(WINAPI* f_VirtualProtect)(
	__in  LPVOID lpAddress,
	__in  SIZE_T dwSize,
	__in  DWORD flNewProtect,
	__out PDWORD lpflOldProtect
);
 

/* NTSTATUS values */
#define STATUS_SUCCESS 			0x00000000
#define STATUS_IMAGE_NOT_AT_BASE	0x40000003

/* Time values */
#define	HIGH_TIME	0x01C422FA
#define LOW_TIME_1	0x7E275CE0
#define LOW_TIME_2	0x8E275CE0

/* Some defines ripped off from DDK */
typedef struct _FILE_BASIC_INFORMATION {
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef enum _SECTION_INFORMATION_CLASS {
	SectionBasicInformation,
	SectionImageInformation
} SECTION_INFORMATION_CLASS;

typedef LARGE_INTEGER PHYSICAL_ADDRESS, *PPHYSICAL_ADDRESS;

typedef LONG NTSTATUS;
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef struct _IO_STATUS_BLOCK {
	NTSTATUS Status;
	ULONG Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
#ifdef MIDL_PASS
	[size_is(MaximumLength / 2), length_is((Length) / 2)] USHORT * Buffer;
#else
	PWSTR  Buffer;
#endif
} UNICODE_STRING;
typedef UNICODE_STRING *PUNICODE_STRING;

typedef struct _ANSI_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} ANSI_STRING, *PANSI_STRING, STRING, *PSTRING;

typedef enum _SECTION_INHERIT {
	ViewShare = 1,
	ViewUnmap = 2
} SECTION_INHERIT;

typedef struct _OBJECT_ATTRIBUTES {
	ULONG Length;
	HANDLE RootDirectory;
	PUNICODE_STRING ObjectName;
	ULONG Attributes;
	PVOID SecurityDescriptor;
	PVOID SecurityQualityOfService;
} OBJECT_ATTRIBUTES;
typedef OBJECT_ATTRIBUTES *POBJECT_ATTRIBUTES;

typedef NTSTATUS(NTAPI *f_NtOpenSection)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES);
typedef NTSTATUS(NTAPI *f_NtQueryAttributesFile)(POBJECT_ATTRIBUTES, PFILE_BASIC_INFORMATION);
typedef void (NTAPI *f_NtOpenFile)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES,
	PIO_STATUS_BLOCK, ULONG ShareAccess, ULONG);
typedef NTSTATUS(NTAPI *f_NtCreateSection)(PHANDLE, ULONG, POBJECT_ATTRIBUTES, PLARGE_INTEGER,
	ULONG, ULONG, HANDLE);
typedef NTSTATUS(NTAPI *f_NtMapViewOfSection)(HANDLE, HANDLE, PVOID *, ULONG, ULONG,
	PLARGE_INTEGER, PULONG, SECTION_INHERIT, ULONG, ULONG);
typedef NTSTATUS(NTAPI *f_NtClose)(HANDLE);

typedef struct _SHELLCODE_CTX {

	/* Library name */
	char				libname[256];
	int				liblen;
	/* Global offset */
	DWORD				offset;
	/* Allocated memory sections */
	DWORD				file_address;
	DWORD				mapped_address;
	DWORD				size_map;

	/* Hook stub functions */
	unsigned char			s_NtOpenSection[10];
	unsigned char			s_NtQueryAttributesFile[10];
	unsigned char			s_NtOpenFile[10];
	unsigned char			s_NtCreateSection[10];
	unsigned char			s_NtMapViewOfSection[10];
	unsigned char			s_NtClose[10];

	/* Hooked functions */
	DWORD				NtOpenSection;
	DWORD				NtQueryAttributesFile;
	DWORD				NtOpenFile;
	DWORD				NtCreateSection;
	DWORD				NtMapViewOfSection;
	DWORD				NtClose;

	f_NtOpenSection				p_NtOpenSection;
	f_NtQueryAttributesFile		p_NtQueryAttributesFile;
	f_NtOpenFile				p_NtOpenFile;
	f_NtCreateSection			p_NtCreateSection;
	f_NtMapViewOfSection		p_NtMapViewOfSection;
	f_NtClose					p_NtClose;

} SHELLCODE_CTX;

#endif