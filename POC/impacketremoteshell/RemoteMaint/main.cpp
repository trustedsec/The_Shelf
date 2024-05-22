#include "SMBConnection.h"
#include "stringobf.h"
#include <process.h>
#define THREADCOUNT 4
#define MAX_CONCURRENT_CONNECTIONS 16
HANDLE hHaltEvent;

#ifdef SERVICEMAIN
SERVICE_STATUS ServiceStatus;
SERVICE_STATUS_HANDLE hStatus;
char* cmd_line;

void ServiceMain(int argc, char* argv[]);
void ControlHandler(DWORD request);
void start(HINSTANCE handle, char* cmd_line);
VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint);
#endif
unsigned int __stdcall WorkerProc(void* IoPort)
{
    for (;;)
    {
        BOOL Status = 0;
        DWORD NumTransferred = 0;
        ULONG_PTR CompKey = COMPLETION_KEY_NONE;
        LPOVERLAPPED pOver = 0;

        Status = GetQueuedCompletionStatus(reinterpret_cast<HANDLE>(IoPort),
            &NumTransferred, &CompKey, &pOver, INFINITE);

        SMBConnection* pConn = reinterpret_cast<SMBConnection*>(pOver);

        if (FALSE == Status && GetLastError() != ERROR_MORE_DATA)
        {
            // An error occurred; reset to a known state.
#ifndef NDEBUG
            printf("Erorr getting completion status: %d\n", GetLastError());
#endif
            if (pConn)
            {
                pConn->IssueReset();
            }
        }
        else if (COMPLETION_KEY_IO == CompKey)
        {
            pConn->OnIoComplete(NumTransferred);
        }
        else if (COMPLETION_KEY_SHUTDOWN == CompKey)
        {
            SetEvent(hHaltEvent);
        }
        else if (COMPLETION_KEY_EXIT == CompKey)
        {
            break;
        }
    }
    return 0;
}

int StartServer()
{
    HANDLE iocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, THREADCOUNT);
    HANDLE Workers[THREADCOUNT] = { 0 };
    unsigned int WorkerIds[THREADCOUNT] = { 0 };
    SMBConnection* Connections[MAX_CONCURRENT_CONNECTIONS] = { 0 };
    hHaltEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    for (int i = 0; i < THREADCOUNT; i++)
    {
        Workers[i] = (HANDLE)_beginthreadex(NULL, 0, WorkerProc, iocp, 0, &(WorkerIds[i]));
    }
    for (int i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++)
    {
        Connections[i] = new SMBConnection{ iocp };
    }
#ifdef SERVICEMAIN
    ReportSvcStatus(SERVICE_RUNNING, 0, 0);
#endif
    WaitForSingleObject(hHaltEvent, INFINITE);
    for (int i = 0; i < THREADCOUNT; i++)
    {
        PostQueuedCompletionStatus(iocp, 0, COMPLETION_KEY_EXIT, 0);
    }
    WaitForMultipleObjects(THREADCOUNT, Workers, TRUE, INFINITE);
    for (int i = 0; i < THREADCOUNT; i++)
    {
        CloseHandle(Workers[i]);
    }
    for (int i = 0; i < MAX_CONCURRENT_CONNECTIONS; i++)
    {
        delete Connections[i];
    }
    CloseHandle(hHaltEvent);
    CloseHandle(iocp);
#ifdef SERVICEMAIN
    ReportSvcStatus(SERVICE_STOPPED, 0, 0);
#endif
    return 0;
}   


int main(int argc, char** argv)
{
#ifdef SERVICEMAIN
    SERVICE_TABLE_ENTRYA ServiceTable[2] = { { NULL, NULL }, { NULL, NULL } };

    ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)ServiceMain;
    ServiceTable[0].lpServiceName = OBF("maint");
    cmd_line = argv[1];
    StartServiceCtrlDispatcherA(ServiceTable);
    return 0;
#else
    return StartServer();
#endif

}

#ifdef SERVICEMAIN
void ServiceMain(int argc, char* argv[]) {
    ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_PAUSE_CONTINUE;
    ServiceStatus.dwWin32ExitCode = 0;
    ServiceStatus.dwServiceSpecificExitCode = 0;
    ServiceStatus.dwCheckPoint = 0;
    ServiceStatus.dwWaitHint = 0;

    hStatus = RegisterServiceCtrlHandlerA(OBF("maint"), (LPHANDLER_FUNCTION)ControlHandler);
    if (hStatus == (SERVICE_STATUS_HANDLE)NULL)
        return;
    ReportSvcStatus(SERVICE_START_PENDING, 0, 1000);
    StartServer();
}

void ControlHandler(DWORD request) {
    switch (request) {
    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:
        SetEvent(hHaltEvent);
        ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 60000);
        return;

    case SERVICE_CONTROL_PAUSE:
        ServiceStatus.dwCurrentState = SERVICE_PAUSED;
        SetServiceStatus(hStatus, &ServiceStatus);
        return;

    case SERVICE_CONTROL_CONTINUE:
        ServiceStatus.dwCurrentState = SERVICE_RUNNING;
        SetServiceStatus(hStatus, &ServiceStatus);

    default:
        break;
    }

    return;
}

VOID ReportSvcStatus(DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwWaitHint)
{
    static DWORD dwCheckPoint = 1;

    // Fill in the SERVICE_STATUS structure.

    ServiceStatus.dwCurrentState = dwCurrentState;
    ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
    ServiceStatus.dwWaitHint = dwWaitHint;

    if (dwCurrentState == SERVICE_START_PENDING)
        ServiceStatus.dwControlsAccepted = 0;
    else ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;

    if ((dwCurrentState == SERVICE_RUNNING) ||
        (dwCurrentState == SERVICE_STOPPED))
        ServiceStatus.dwCheckPoint = 0;
    else ServiceStatus.dwCheckPoint = dwCheckPoint++;

    // Report the status of the service to the SCM.
    SetServiceStatus(hStatus, &ServiceStatus);
}
#endif