#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_SurveyHost.h"
#include "LIB_SurveyHost.h"

// Primary TsunamiWave Configuration
CHAR g_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD dwErrorCode = ERROR_SUCCESS;
	DWORD dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpPackedConfigurationBuffer = NULL;
	DWORD	dwPackedConfigurationBufferSize = 0;
	LPBYTE	lpPackedConfigurationPasswordBuffer = NULL;
	DWORD	dwPackedConfigurationPasswordBufferSize = 0;
	CHAR*	szConfigurationJson = NULL;
	DWORD	dwConfigurationJsonSize = 0;


	cJSON*	lpJsonSurveyHostRequestEntries = NULL;
	cJSON*	lpJsonSurveyHostRequestEntry = NULL;

	CHAR*	szSurveyHostInput = NULL;
	DWORD	dwSurveyHostInputSize = 0;
	CHAR*	szSurveyHostOutput = NULL;
	DWORD	dwSurveyHostOutputSize = 0;

	cJSON*	lpJsonSurveyHostOutput = NULL;
	//cJSON*	lpJsonSurveyHostResponse = NULL;
	//cJSON*	lpJsonSurveyHostResponseResults = NULL;
//	cJSON*	lpJsonSurveyHostOutput = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	//DWORD	dwIndex = 0;

	cJSON*	lpJsonComputerName = NULL;
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

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyHost wmain() start"));

			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_szPackedConfigurationPassword);

			DBGPRINT(DEBUG_INFO, TEXT("Get the configuration JSON from the binary storage"));
			dwErrorCode = GetConfigurationFromBinary(
				lpPackedConfigurationBuffer,
				dwPackedConfigurationBufferSize,
				lpPackedConfigurationPasswordBuffer,
				dwPackedConfigurationPasswordBufferSize,
				(LPBYTE*)&szConfigurationJson,
				&dwConfigurationJsonSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("DecodeDecryptDecompressBuffer( %p, %d, %p, %d, %p, %p ) failed.(%08x)"),
					lpPackedConfigurationBuffer,
					dwPackedConfigurationBufferSize,
					lpPackedConfigurationPasswordBuffer,
					dwPackedConfigurationPasswordBufferSize,
					(LPBYTE*)&szConfigurationJson,
					&dwConfigurationJsonSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwConfigurationJsonSize: %d"), dwConfigurationJsonSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szConfigurationJson:     %p"), szConfigurationJson);
			//printf("szConfigurationJson:\n%s\n", szConfigurationJson);

			// Parse the output string into a JSON object
			lpJsonConfiguration = cJSON_Parse(szConfigurationJson);
			if (NULL == lpJsonConfiguration)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonConfiguration = cJSON_Parse(%p) failed."),
					szConfigurationJson
				);
				__leave;
			}

			// Get the binary id
			lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_BINARY_ID);
			if (NULL == lpJsonBinaryId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					TSM_CONFIGURATION_BINARY_ID
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonBinaryId))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonBinaryId
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %d"), TSM_CONFIGURATION_BINARY_ID, lpJsonBinaryId->valueint);

			// Get the agent id
			lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, TSM_CONFIGURATION_AGENT_ID);
			if (NULL == lpJsonAgentId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					TSM_CONFIGURATION_AGENT_ID
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonAgentId))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonAgentId
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("%S: %d"), TSM_CONFIGURATION_AGENT_ID, lpJsonAgentId->valueint);

			// -------------------------------------------------------------------------
			// Get any other configuration settings
			// -------------------------------------------------------------------------


			/***************************************************************************
				Initialize the SurveyHost plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyHost plugin"));
			dwErrorCode = SurveyHostInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyHostInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the SurveyHostCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the SurveyHostCallbackInterface"));

			szSurveyHostInput = NULL;
			dwSurveyHostInputSize = 0;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyHostInputSize: %d"), dwSurveyHostInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyHostInput:     %p"), szSurveyHostInput);

			// Call the SurveyHostCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = SurveyHostCallbackInterface(
				(LPBYTE)szSurveyHostInput,
				dwSurveyHostInputSize,
				(LPBYTE*)&szSurveyHostOutput,
				&dwSurveyHostOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyHostCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szSurveyHostInput,
					dwSurveyHostInputSize,
					(LPBYTE*)&szSurveyHostOutput,
					&dwSurveyHostOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyHostOutputSize: %d"), dwSurveyHostOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyHostOutput:     %p"), szSurveyHostOutput);
			printf("[%s(%d)] szSurveyHostOutput:\n%s\n", __FILE__, __LINE__, szSurveyHostOutput);

			// Parse the output string into a JSON object
			lpJsonSurveyHostOutput = cJSON_Parse(szSurveyHostOutput);
			if (NULL == lpJsonSurveyHostOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyHostOutput = cJSON_Parse(%p) failed."),
					szSurveyHostOutput
				);
				__leave;
			}

			//// Get the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			//lpJsonSurveyHostResponse = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response);
			//if (NULL == lpJsonSurveyHostResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyHostResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyHostOutput, SurveyHost_Response
			//	);
			//	__leave;
			//}

			//// Get the response results array
			//lpJsonSurveyHostResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response_Results);
			//if (NULL == lpJsonSurveyHostResponseResults)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyHostResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyHostOutput,
			//		SurveyHost_Response_Results
			//	);
			//	__leave;
			//}
			//if (!cJSON_IsArray(lpJsonSurveyHostResponseResults))
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_IsArray(%p) failed."),
			//		lpJsonSurveyHostResponseResults
			//	);
			//	__leave;
			//}
			//// Loop through all the output results objects and display them
			//dwIndex = 0;
			//cJSON_ArrayForEach(lpJsonSurveyHostOutput, lpJsonSurveyHostResponseResults)
			//{


			//DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]:"), SurveyHost_Response_Results, dwIndex);

			// Get and display the SurveyHost_Response_ComputerName
			lpJsonComputerName = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response_ComputerName);
			if (NULL == lpJsonComputerName)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonComputerName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyHostOutput,
					SurveyHost_Response_ComputerName
				);
				__leave;
			}
			if (
				!cJSON_IsString(lpJsonComputerName) ||
				(NULL == lpJsonComputerName->valuestring)
				)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonComputerName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), SurveyHost_Response_ComputerName, lpJsonComputerName->valuestring);

			// Get and display the SurveyHost_Response_VersionInfo
			lpJsonVersionInfo = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response_VersionInfo);
			if (NULL == lpJsonVersionInfo)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfo = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyHostOutput,
					SurveyHost_Response_VersionInfo
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:"), SurveyHost_Response_VersionInfo);

			// Get and display the SurveyHost_Response_VersionInfoMajorVersion
			lpJsonVersionInfoMajorVersion = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoMajorVersion);
			if (NULL == lpJsonVersionInfoMajorVersion)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoMajorVersion = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoMajorVersion
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoMajorVersion))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoMajorVersion
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoMajorVersion, lpJsonVersionInfoMajorVersion->valueint);

			// Get and display the SurveyHost_Response_VersionInfoMinorVersion
			lpJsonVersionInfoMinorVersion = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoMinorVersion);
			if (NULL == lpJsonVersionInfoMinorVersion)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoMinorVersion = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoMinorVersion
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoMinorVersion))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoMinorVersion
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoMinorVersion, lpJsonVersionInfoMinorVersion->valueint);

			// Get and display the SurveyHost_Response_VersionInfoServicePackMajor
			lpJsonVersionInfoServicePackMajor = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoServicePackMajor);
			if (NULL == lpJsonVersionInfoServicePackMajor)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoServicePackMajor = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoServicePackMajor
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoServicePackMajor))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoServicePackMajor
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoServicePackMajor, lpJsonVersionInfoServicePackMajor->valueint);

			// Get and display the SurveyHost_Response_VersionInfoServicePackMinor
			lpJsonVersionInfoServicePackMinor = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoServicePackMinor);
			if (NULL == lpJsonVersionInfoServicePackMinor)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoServicePackMinor = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoServicePackMinor
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoServicePackMinor))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoServicePackMinor
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoServicePackMinor, lpJsonVersionInfoServicePackMinor->valueint);

			// Get and display the SurveyHost_Response_VersionInfoCSDVersion
			lpJsonVersionInfoCSDVersion = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoCSDVersion);
			if (NULL == lpJsonVersionInfoCSDVersion)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoCSDVersion = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoCSDVersion
				);
				__leave;
			}
			if (
				(!cJSON_IsString(lpJsonVersionInfoCSDVersion))  ||
				( NULL == lpJsonVersionInfoCSDVersion->valuestring )
				)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonVersionInfoCSDVersion
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %S"), SurveyHost_Response_VersionInfoCSDVersion, lpJsonVersionInfoCSDVersion->valuestring);


			// Get and display the SurveyHost_Response_VersionInfoSuiteMask
			lpJsonVersionInfoSuiteMask = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoSuiteMask);
			if (NULL == lpJsonVersionInfoSuiteMask)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoSuiteMask = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoSuiteMask
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoSuiteMask))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoSuiteMask
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoSuiteMask, lpJsonVersionInfoSuiteMask->valueint);

			// Get and display the SurveyHost_Response_VersionInfoProductType
			lpJsonVersionInfoProductType = cJSON_GetObjectItemCaseSensitive(lpJsonVersionInfo, SurveyHost_Response_VersionInfoProductType);
			if (NULL == lpJsonVersionInfoProductType)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVersionInfoProductType = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVersionInfo,
					SurveyHost_Response_VersionInfoProductType
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVersionInfoProductType))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVersionInfoProductType
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VersionInfoProductType, lpJsonVersionInfoProductType->valueint);


			// Get and display the SurveyHost_Response_SystemInfo
			lpJsonSystemInfo = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response_SystemInfo);
			if (NULL == lpJsonSystemInfo)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonSystemInfo = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyHostOutput,
					SurveyHost_Response_SystemInfo
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:"), SurveyHost_Response_SystemInfo);

			// Get and display the SurveyHost_Response_SystemInfoProcessorArchitecture
			lpJsonSystemInfoProcessorArchitecture = cJSON_GetObjectItemCaseSensitive(lpJsonSystemInfo, SurveyHost_Response_SystemInfoProcessorArchitecture);
			if (NULL == lpJsonSystemInfoProcessorArchitecture)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonSystemInfoProcessorArchitecture = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSystemInfo,
					SurveyHost_Response_SystemInfoProcessorArchitecture
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonSystemInfoProcessorArchitecture))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonSystemInfoProcessorArchitecture
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_SystemInfoProcessorArchitecture, lpJsonSystemInfoProcessorArchitecture->valueint);


			// Get and display the SurveyHost_Response_VolumeInfo
			lpJsonVolumeInfo = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyHostOutput, SurveyHost_Response_VolumeInfo);
			if (NULL == lpJsonVolumeInfo)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVolumeInfo = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyHostOutput,
					SurveyHost_Response_VolumeInfo
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:"), SurveyHost_Response_VolumeInfo);

			// Get and display the SurveyHost_Response_VolumeInfoRootPathName
			lpJsonVolumeInfoRootPathName = cJSON_GetObjectItemCaseSensitive(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoRootPathName);
			if (NULL == lpJsonVolumeInfoRootPathName)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVolumeInfoRootPathName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVolumeInfo,
					SurveyHost_Response_VolumeInfoRootPathName
				);
				__leave;
			}
			if (
				(!cJSON_IsString(lpJsonVolumeInfoRootPathName)) ||
				(NULL == lpJsonVolumeInfoRootPathName->valuestring)
				)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonVolumeInfoRootPathName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %S"), SurveyHost_Response_VolumeInfoRootPathName, lpJsonVolumeInfoRootPathName->valuestring);

			// Get and display the SurveyHost_Response_VolumeInfoVolumeName
			lpJsonVolumeInfoVolumeName = cJSON_GetObjectItemCaseSensitive(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoVolumeName);
			if (NULL == lpJsonVolumeInfoVolumeName)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVolumeInfoVolumeName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVolumeInfo,
					SurveyHost_Response_VolumeInfoVolumeName
				);
				__leave;
			}
			if (
				(!cJSON_IsString(lpJsonVolumeInfoVolumeName)) ||
				(NULL == lpJsonVolumeInfoVolumeName->valuestring)
				)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonVolumeInfoVolumeName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %S"), SurveyHost_Response_VolumeInfoVolumeName, lpJsonVolumeInfoVolumeName->valuestring);

			// Get and display the SurveyHost_Response_VolumeInfoSerialNumber
			lpJsonVolumeInfoSerialNumber = cJSON_GetObjectItemCaseSensitive(lpJsonVolumeInfo, SurveyHost_Response_VolumeInfoSerialNumber);
			if (NULL == lpJsonVolumeInfoSerialNumber)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonVolumeInfoSerialNumber = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonVolumeInfo,
					SurveyHost_Response_VolumeInfoSerialNumber
				);
				__leave;
			}
			if (!cJSON_IsNumber(lpJsonVolumeInfoSerialNumber))
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonVolumeInfoSerialNumber
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("    ->%S: %d"), SurveyHost_Response_VolumeInfoSerialNumber, lpJsonVolumeInfoSerialNumber->valueint);


			//	// Continue to next lpJsonSurveyHostResponseResults
			//	dwIndex++;

			//} // end cJSON_ArrayForEach(lpJsonSurveyHostOutput, lpJsonSurveyHostResponseResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyHostOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonSurveyHostOutput);
				cJSON_Delete(lpJsonSurveyHostOutput);
				lpJsonSurveyHostOutput = NULL;
			}
			// Free configuration string
			if (NULL != szSurveyHostInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szSurveyHostInput);
				FREE(szSurveyHostInput);
				szSurveyHostInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szSurveyHostOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szSurveyHostOutput);
				cJSON_free(szSurveyHostOutput);
				szSurveyHostOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the SurveyHost plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyHost plugin"));
			dwTempErrorCode = SurveyHostFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyHostFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyHost wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}