#pragma once
#ifndef __LIB_InternetConnect__
#define __LIB_InternetConnect__

#include <Windows.h>

//#define InternetConnect_Configuration					"internet_connect_configuration"
#define InternetConnect_Configuration_CallbackUrl		"callback_url"
#define InternetConnect_Configuration_UserAgent			"user_agent"
//#define InternetConnect_Configuration_C2_Password		"c2_password"

#define InternetConnect_Send_Request					"internet_connect_send_request"
#define InternetConnect_Send_Request_Buffer				"request_buffer"
//#define InternetConnect_Send_Response					"internet_connect_send_response"
#define InternetConnect_Send_Response_Buffer			"response_buffer"


DWORD WINAPI InternetConnectInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI InternetConnectFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI InternetConnectSendRequest(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);



#endif
