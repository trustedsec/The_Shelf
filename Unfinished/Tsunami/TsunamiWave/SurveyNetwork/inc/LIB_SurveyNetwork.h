#pragma once
#ifndef __LIB_SurveyNetwork__
#define __LIB_SurveyNetwork__

#include <Windows.h>

#define SurveyNetwork_Request							"survey_network_request"
//#define SurveyNetwork_Response							"survey_network_response"
#define SurveyNetwork_Response_Results					"network_results"
#define SurveyNetwork_Response_ComputerName				"computer_name"
#define SurveyNetwork_Response_Adapters					"adapters"
#define SurveyNetwork_Response_AdapterInterfaceType		"interface_type"
#define SurveyNetwork_Response_AdapterInterfaceIndex	"interface_type"
#define SurveyNetwork_Response_AdapterOperatingStatus	"operating_status"
#define SurveyNetwork_Response_AdapterFriendlyName		"friendly_name"
#define SurveyNetwork_Response_AdapterDescription		"description"
#define SurveyNetwork_Response_AdapterPhysicalAddress	"physical_address"
#define SurveyNetwork_Response_AdapterUnicastAddresses	"unicast_addresses"
#define SurveyNetwork_Response_AdapterDNSSuffix			"dns_suffix"
#define SurveyNetwork_Response_AdapterDNSAddresses		"dns_server_addresses"


DWORD WINAPI SurveyNetworkInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyNetworkFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI SurveyNetworkCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
