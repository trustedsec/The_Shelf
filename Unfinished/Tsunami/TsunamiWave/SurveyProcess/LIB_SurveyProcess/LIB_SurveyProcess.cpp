#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_SurveyProcess.h"
#include "Internal_Functions.h"


// Global variables
cJSON* g_SurveyProcess_lpJsonConfiguration = NULL;


PVOID GetPebAddress(HANDLE ProcessHandle)
{
	PVOID lpPEB = NULL;
    PROCESS_BASIC_INFORMATION pbi;
 
	ZeroMemory(&pbi, sizeof(pbi));
    
	if (ERROR_SUCCESS == m_SurveyProcess_NtQueryInformationProcess(ProcessHandle, 0, &pbi, sizeof(pbi), NULL))
	{
		lpPEB = pbi.PebBaseAddress;
	}

	return lpPEB;
}

DWORD WINAPI SurveyProcessInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonSurveyProcessInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonSurveyProcessInitializeOutput = NULL;
	CHAR*	szSurveyProcessOutput = NULL;
	DWORD	dwSurveyProcessOutputSize = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonSurveyProcessInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonSurveyProcessInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}

			// Set the local module's configuration to the input configuration
			g_SurveyProcess_lpJsonConfiguration = lpJsonSurveyProcessInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonSurveyProcessInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_SurveyProcess_Kernel32_Functions()"));
			dwErrorCode = Initialize_SurveyProcess_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_SurveyProcess_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyProcess_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_SurveyProcess_lpJsonConfiguration,
				SurveyProcess_Request,
				SurveyProcessCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_SurveyProcess_lpJsonConfiguration,
					SurveyProcess_Request,
					SurveyProcessCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyProcess_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szSurveyProcessOutput = cJSON_Print(lpJsonSurveyProcessInitializeOutput);
			//if (NULL == szSurveyProcessOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonSurveyProcessInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyProcessOutput:     %p"), szSurveyProcessOutput);
			////printf("szSurveyProcessOutput:\n%s\n", szSurveyProcessOutput);
			//dwSurveyProcessOutputSize = (DWORD)strlen(szSurveyProcessOutput);

			//(*lppOutputBuffer) = (LPBYTE)szSurveyProcessOutput;
			//(*lpdwOutputBufferSize) = dwSurveyProcessOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyProcessInitializeInput)
			{
				cJSON_Delete(lpJsonSurveyProcessInitializeInput);
				lpJsonSurveyProcessInitializeInput = NULL;
			}
			if (NULL != lpJsonSurveyProcessInitializeOutput)
			{
				cJSON_Delete(lpJsonSurveyProcessInitializeOutput);
				lpJsonSurveyProcessInitializeOutput = NULL;
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


DWORD WINAPI SurveyProcessFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyProcessInput = NULL;
	DWORD	dwSurveyProcessInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szSurveyProcessOutput = NULL;
	DWORD	dwSurveyProcessOutputSize = 0;
	cJSON*	lpJsonSurveyProcessOutput = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);

			/********************************************************************************
				Check arguments
			********************************************************************************/
			//if (NULL == lppOutputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lppOutputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//if (NULL == lpInputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpInputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//szSurveyProcessInput = (char*)lpInputBuffer;
			//dwSurveyProcessInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyProcess_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_SurveyProcess_lpJsonConfiguration,
				SurveyProcess_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_SurveyProcess_lpJsonConfiguration,
					SurveyProcess_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyProcess_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_SurveyProcess_Kernel32_Functions()"));
			dwErrorCode = Finalize_SurveyProcess_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_SurveyProcess_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_SurveyProcess_lpJsonConfiguration)
			{
				cJSON_Delete(g_SurveyProcess_lpJsonConfiguration);
				g_SurveyProcess_lpJsonConfiguration = NULL;
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


DWORD WINAPI SurveyProcessCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyProcessInput = NULL;
	DWORD	dwSurveyProcessInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonSurveyProcessRequestEntries = NULL;
	cJSON*	lpJsonSurveyProcessRequestEntry = NULL;

	CHAR*	szSurveyProcessOutput = NULL;
	DWORD	dwSurveyProcessOutputSize = 0;
	cJSON*	lpJsonSurveyProcessOutput = NULL;
	//cJSON*	lpJsonSurveyProcessResponse = NULL;
	cJSON*	lpJsonSurveyProcessResponseResults = NULL;

	DWORD	dwRequestIndex = 0;

	HANDLE hProcessSnap = INVALID_HANDLE_VALUE;
	PROCESSENTRY32 pe32;

	HANDLE hModuleSnap = INVALID_HANDLE_VALUE;
	MODULEENTRY32 me32;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputBuffer:        %p"), lpInputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputBufferSize:    %d"), dwInputBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lppOutputBuffer:      %p"), lppOutputBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpdwOutputBufferSize: %p"), lpdwOutputBufferSize);


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
			szSurveyProcessInput = (char*)lpInputBuffer;
			dwSurveyProcessInputSize = dwInputBufferSize;


			/********************************************************************************
				Check input for new configuration
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szSurveyProcessInput);
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
			// Create the output JSON object
			lpJsonSurveyProcessOutput = cJSON_CreateObject();
			if (NULL == lpJsonSurveyProcessOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyProcessOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the SurveyProcess_Response JSON object
			//lpJsonSurveyProcessResponse = cJSON_AddObjectToObject(lpJsonSurveyProcessOutput, SurveyProcess_Response);
			//if (NULL == lpJsonSurveyProcessResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyProcessResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonSurveyProcessOutput, SurveyProcess_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyProcessResponse:    %p"), lpJsonSurveyProcessResponse);

			// Create an array of SurveyProcess_Registry_Results JSON objects
			lpJsonSurveyProcessResponseResults = cJSON_AddArrayToObject(lpJsonSurveyProcessOutput, SurveyProcess_Response_Results);
			if (NULL == lpJsonSurveyProcessResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyProcessResponseResults = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonSurveyProcessOutput,
					SurveyProcess_Response_Results
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyProcessResponseResults:    %p"), lpJsonSurveyProcessResponseResults);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Take a snapshot of all processes in the system.
			hProcessSnap = m_SurveyProcess_CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
			if (hProcessSnap == INVALID_HANDLE_VALUE)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("f_SurveyProcess_CreateToolhelp32Snapshot( %08x, %08x ) failed.(%08x)"),
					TH32CS_SNAPPROCESS, 0,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hProcessSnap: %p"), hProcessSnap);

			ZeroMemory(&pe32, sizeof(pe32));
			// Set the size of the structure before using it.
			pe32.dwSize = sizeof(PROCESSENTRY32);

			// Retrieve information about the first process, and exit if unsuccessful
			if (FALSE == m_SurveyProcess_Process32First(hProcessSnap, &pe32))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("f_SurveyProcess_Process32First( %p, %p ) failed.(%08x)"),
					hProcessSnap, &pe32,
					dwErrorCode
				);
				__leave;
			}

			do
			{
				// Get the array of SurveyProcess_Registry_Entries
				lpJsonSurveyProcessRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonInput, SurveyProcess_Request_Entries);
				if (NULL == lpJsonSurveyProcessRequestEntries)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonInput,
						SurveyProcess_Request_Entries
					);
					__leave;
				}
				// Loop through each request entry and perform the actual request
				dwRequestIndex = 0;
				cJSON_ArrayForEach(lpJsonSurveyProcessRequestEntry, lpJsonSurveyProcessRequestEntries)
				{
					LPSTR szEntry = NULL;

					// Make sure that the request entry JSON object is a string
					if (
						!cJSON_IsString(lpJsonSurveyProcessRequestEntry) ||
						(NULL == lpJsonSurveyProcessRequestEntry->valuestring)
						)
					{
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_IsString(%p) failed."),
							lpJsonSurveyProcessRequestEntry
						);
						continue;
					}
						
					szEntry = lpJsonSurveyProcessRequestEntry->valuestring;
					//DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]: %S"), SurveyProcess_Request_Entries, dwRequestIndex, szEntry);


					if (
						(0 == _stricmp(szEntry, "*")) ||
						(0 == _stricmp(szEntry, pe32.szExeFile))
						)
					{
						cJSON* lpJsonSurveyProcessResponseResult = NULL;
						cJSON* lpJsonSurveyProcessResponseResultExeFile = NULL;
						cJSON* lpJsonSurveyProcessResponseResultProcessId = NULL;
						cJSON* lpJsonSurveyProcessResponseResultParentProcessId = NULL;

						//DBGPRINT(DEBUG_VERBOSE, TEXT("Found survey entry: \"%S\""), szEntry);

						lpJsonSurveyProcessResponseResult = cJSON_CreateObject();
						if (NULL == lpJsonSurveyProcessResponseResult)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSurveyProcessResponseResult = cJSON_CreateObject() failed.\n")
							);
							__leave;
						}

						lpJsonSurveyProcessResponseResultExeFile = cJSON_CreateString(pe32.szExeFile);
						if (NULL == lpJsonSurveyProcessResponseResultExeFile)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSurveyProcessResponseResultExeFile = cJSON_CreateString failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ExeFile, lpJsonSurveyProcessResponseResultExeFile);

						lpJsonSurveyProcessResponseResultProcessId = cJSON_CreateNumber(pe32.th32ProcessID);
						if (NULL == lpJsonSurveyProcessResponseResultProcessId)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSurveyProcessResponseResultProcessId = cJSON_CreateNumber failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ProcessId, lpJsonSurveyProcessResponseResultProcessId);

						lpJsonSurveyProcessResponseResultParentProcessId = cJSON_CreateNumber(pe32.th32ParentProcessID);
						if (NULL == lpJsonSurveyProcessResponseResultParentProcessId)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSurveyProcessResponseResultParentProcessId = cJSON_CreateNumber failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyProcessResponseResult, SurveyProcess_Response_ParentProcessId, lpJsonSurveyProcessResponseResultParentProcessId);

						// Add current entry to overall results list
						cJSON_AddItemToArray(lpJsonSurveyProcessResponseResults, lpJsonSurveyProcessResponseResult);

					} // end if we are interested in this value

				} // end cJSON_ArrayForEach(lpJsonSurveyProcessRequestEntry, lpJsonSurveyProcessRequestEntries)
			} while (m_SurveyProcess_Process32Next(hProcessSnap, &pe32));

			
			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szSurveyProcessOutput = cJSON_Print(lpJsonSurveyProcessOutput);
			if (NULL == szSurveyProcessOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonSurveyProcessOutput
				);
				__leave;
			}
			dwSurveyProcessOutputSize = (DWORD)strlen(szSurveyProcessOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyProcessOutputSize: %d"), dwSurveyProcessOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyProcessOutput:     %p"), szSurveyProcessOutput);
			//printf("szSurveyProcessOutput:\n%s\n", szSurveyProcessOutput);

			(*lppOutputBuffer) = (LPBYTE)szSurveyProcessOutput;
			(*lpdwOutputBufferSize) = dwSurveyProcessOutputSize;

		} // end try-finally
		__finally
		{
			if (INVALID_HANDLE_VALUE != hModuleSnap)
			{
				CloseHandle(hModuleSnap);
				hModuleSnap = INVALID_HANDLE_VALUE;
			}

			if (INVALID_HANDLE_VALUE != hProcessSnap)
			{
				CloseHandle(hProcessSnap);
				hProcessSnap = INVALID_HANDLE_VALUE;
			}


			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonSurveyProcessOutput)
			{
				cJSON_Delete(lpJsonSurveyProcessOutput);
				lpJsonSurveyProcessOutput = NULL;
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

					cJSON_free(szSurveyProcessOutput);
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
