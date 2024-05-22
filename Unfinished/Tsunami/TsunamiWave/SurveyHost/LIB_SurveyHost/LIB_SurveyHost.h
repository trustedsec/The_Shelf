#pragma once
#ifndef __LIB_SurveyHost__
#define __LIB_SurveyHost__

#include <Windows.h>

#define SurveyHost_Request								"survey_host_request"
//#define SurveyHost_Response								"survey_host_response"
#define SurveyHost_Response_Results						"host_results"
#define SurveyHost_Response_ComputerName				"computer_name"
#define SurveyHost_Response_VersionInfo					"version_info"
#define SurveyHost_Response_VersionInfoMajorVersion		"major_version"
#define SurveyHost_Response_VersionInfoMinorVersion		"minor_version"
#define SurveyHost_Response_VersionInfoServicePackMajor	"service_pack_major"
#define SurveyHost_Response_VersionInfoServicePackMinor	"service_pack_minor"
#define SurveyHost_Response_VersionInfoCSDVersion		"csd_version"
#define SurveyHost_Response_VersionInfoSuiteMask		"suite_mask"
#define SurveyHost_Response_VersionInfoProductType		"product_type"
#define SurveyHost_Response_SystemInfo					"system_info"
#define SurveyHost_Response_SystemInfoProcessorArchitecture		"processor_architecture"
#define SurveyHost_Response_VolumeInfo					"volume_info"
#define SurveyHost_Response_VolumeInfoRootPathName		"root_path_name"
#define SurveyHost_Response_VolumeInfoVolumeName		"volume_name"
#define SurveyHost_Response_VolumeInfoSerialNumber		"serial_number"
#define SurveyHost_Response_VolumeInfoFileSystemFlags	"file_system_flags"
#define SurveyHost_Response_VolumeInfoFileSystemName	"file_system_name"


DWORD WINAPI SurveyHostInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyHostFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyHostCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);
#endif
