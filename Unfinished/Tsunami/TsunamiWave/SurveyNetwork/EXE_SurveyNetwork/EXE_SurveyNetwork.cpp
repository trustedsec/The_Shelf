#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_SurveyNetwork.h"
#include "LIB_SurveyNetwork.h"

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


	cJSON*	lpJsonSurveyNetworkRequestEntries = NULL;
	cJSON*	lpJsonSurveyNetworkRequestEntry = NULL;

	CHAR*	szSurveyNetworkInput = NULL;
	DWORD	dwSurveyNetworkInputSize = 0;
	CHAR*	szSurveyNetworkOutput = NULL;
	DWORD	dwSurveyNetworkOutputSize = 0;

	cJSON*	lpJsonSurveyNetworkOutput = NULL;
	//cJSON*	lpJsonSurveyNetworkResponse = NULL;
	cJSON*	lpJsonSurveyNetworkResponseResults = NULL;
	cJSON*	lpJsonSurveyNetworkResponseResult = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	DWORD	dwIndex = 0;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyNetwork wmain() start"));

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
				Initialize the SurveyNetwork plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyNetwork plugin"));
			dwErrorCode = SurveyNetworkInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyNetworkInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the SurveyNetworkCallbackInterface
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the SurveyNetworkCallbackInterface"));

			szSurveyNetworkInput = NULL;
			dwSurveyNetworkInputSize = 0;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyNetworkInputSize: %d"), dwSurveyNetworkInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyNetworkInput:     %p"), szSurveyNetworkInput);

			// Call the SurveyNetworkCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = SurveyNetworkCallbackInterface(
				(LPBYTE)szSurveyNetworkInput,
				dwSurveyNetworkInputSize,
				(LPBYTE*)&szSurveyNetworkOutput,
				&dwSurveyNetworkOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyNetworkCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szSurveyNetworkInput,
					dwSurveyNetworkInputSize,
					(LPBYTE*)&szSurveyNetworkOutput,
					&dwSurveyNetworkOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyNetworkOutputSize: %d"), dwSurveyNetworkOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyNetworkOutput:     %p"), szSurveyNetworkOutput);
			//printf("szSurveyNetworkOutput:\n%s\n", szSurveyNetworkOutput);

			// Parse the output string into a JSON object
			lpJsonSurveyNetworkOutput = cJSON_Parse(szSurveyNetworkOutput);
			if (NULL == lpJsonSurveyNetworkOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkOutput = cJSON_Parse(%p) failed."),
					szSurveyNetworkOutput
				);
				__leave;
			}

			//// Get the TSM_MANAGER_UNREGISTER_RESPONSE JSON object
			//lpJsonSurveyNetworkResponse = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyNetworkOutput, SurveyNetwork_Response);
			//if (NULL == lpJsonSurveyNetworkResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyNetworkResponse = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
			//		lpJsonSurveyNetworkOutput, SurveyNetwork_Response
			//	);
			//	__leave;
			//}

			// Get the response results array
			lpJsonSurveyNetworkResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyNetworkOutput, SurveyNetwork_Response_Results);
			if (NULL == lpJsonSurveyNetworkResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyNetworkResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonSurveyNetworkOutput,
					SurveyNetwork_Response_Results
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonSurveyNetworkResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonSurveyNetworkResponseResults
				);
				__leave;
			}
			// Loop through all the output results objects and display them
			dwIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyNetworkResponseResult, lpJsonSurveyNetworkResponseResults)
			{
				cJSON*	lpJsonComputerName = NULL;
				cJSON*	lpJsonAdapters = NULL;
				cJSON*	lpJsonAdapter = NULL;
				DWORD	dwAdapterIndex = 0;
				

				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]:"), SurveyNetwork_Response_Results, dwIndex);

				// Get and display the SurveyNetwork_Response_ComputerName
				lpJsonComputerName = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyNetworkResponseResult, SurveyNetwork_Response_ComputerName);
				if (NULL == lpJsonComputerName)
				{
					//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("lpJsonComputerName = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyNetworkResponseResult,
						SurveyNetwork_Response_ComputerName
					);
					continue;
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
					continue;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), SurveyNetwork_Response_ComputerName, lpJsonComputerName->valuestring);

				// Get the adapters array
				lpJsonAdapters = cJSON_GetObjectItemCaseSensitive(lpJsonSurveyNetworkResponseResult, SurveyNetwork_Response_Adapters);
				if (NULL == lpJsonAdapters)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonAdapters = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonSurveyNetworkResponseResult,
						SurveyNetwork_Response_Adapters
					);
					__leave;
				}
				if (!cJSON_IsArray(lpJsonAdapters))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsArray(%p) failed."),
						lpJsonAdapters
					);
					__leave;
				}
				dwAdapterIndex = 0;
				// Loop through all the output results objects and display them
				cJSON_ArrayForEach(lpJsonAdapter, lpJsonAdapters)
				{
					cJSON* lpJsonAdapterInterfaceType = NULL;
					cJSON* lpJsonAdapterFriendlyName = NULL;
					cJSON* lpJsonAdapterUnicastAddresses = NULL;
					cJSON* lpJsonAdapterUnicastAddress = NULL;
					cJSON* lpJsonAdapterDNSSuffix = NULL;
					DWORD	dwAddressIndex = 0;
					
					// Get and display the SurveyNetwork_Response_AdapterInterfaceType
					lpJsonAdapterInterfaceType = cJSON_GetObjectItemCaseSensitive(lpJsonAdapter, SurveyNetwork_Response_AdapterInterfaceType);
					if (NULL == lpJsonAdapterInterfaceType)
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("lpJsonAdapterInterfaceType = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
							lpJsonAdapter,
							SurveyNetwork_Response_AdapterInterfaceType
						);
						continue;
					}
					if (!cJSON_IsNumber(lpJsonAdapterInterfaceType))
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_IsNumber(%p) failed."),
							lpJsonAdapterInterfaceType
						);
						continue;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S[%d].%S: %d"), SurveyNetwork_Response_Adapters, dwAdapterIndex, SurveyNetwork_Response_AdapterInterfaceType, lpJsonAdapterInterfaceType->valueint);

					// Get and display the SurveyFile_Response_FileName
					lpJsonAdapterFriendlyName = cJSON_GetObjectItemCaseSensitive(lpJsonAdapter, SurveyNetwork_Response_AdapterFriendlyName);
					if (NULL == lpJsonAdapterFriendlyName)
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
							lpJsonAdapter,
							SurveyNetwork_Response_AdapterFriendlyName
						);
						continue;
					}
					if (
						!cJSON_IsString(lpJsonAdapterFriendlyName) ||
						(NULL == lpJsonAdapterFriendlyName->valuestring)
						)
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_IsString(%p) failed."),
							lpJsonAdapterFriendlyName
						);
						continue;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S[%d].%S: %S"), SurveyNetwork_Response_Adapters, dwAdapterIndex, SurveyNetwork_Response_AdapterFriendlyName, lpJsonAdapterFriendlyName->valuestring);

					// Get and display the SurveyFile_Response_FileName
					lpJsonAdapterDNSSuffix = cJSON_GetObjectItemCaseSensitive(lpJsonAdapter, SurveyNetwork_Response_AdapterDNSSuffix);
					if (NULL == lpJsonAdapterDNSSuffix)
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("lpJsonValue = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
							lpJsonAdapter,
							SurveyNetwork_Response_AdapterDNSSuffix
						);
						continue;
					}
					if (
						!cJSON_IsString(lpJsonAdapterDNSSuffix) ||
						(NULL == lpJsonAdapterDNSSuffix->valuestring)
						)
					{
						//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_IsString(%p) failed."),
							lpJsonAdapterDNSSuffix
						);
						continue;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S[%d].%S: %S"), SurveyNetwork_Response_Adapters, dwAdapterIndex, SurveyNetwork_Response_AdapterDNSSuffix, lpJsonAdapterDNSSuffix->valuestring);


					// Get the unicast addresses array
					lpJsonAdapterUnicastAddresses = cJSON_GetObjectItemCaseSensitive(lpJsonAdapter, SurveyNetwork_Response_AdapterUnicastAddresses);
					if (NULL == lpJsonAdapterUnicastAddresses)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonAdapterUnicastAddresses = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
							lpJsonAdapter,
							SurveyNetwork_Response_AdapterUnicastAddresses
						);
						__leave;
					}
					if (!cJSON_IsArray(lpJsonAdapterUnicastAddresses))
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("cJSON_IsArray(%p) failed."),
							lpJsonAdapterUnicastAddresses
						);
						__leave;
					}
					dwAddressIndex = 0;
					// Loop through all the output results objects and display them
					cJSON_ArrayForEach(lpJsonAdapterUnicastAddress, lpJsonAdapterUnicastAddresses)
					{
						int nBufferSize = 0;
						unsigned char* lpBuffer = NULL;
						LPSOCKADDR lpSockaddr = NULL;

						// Check to make sure that lpJsonAdapterUnicastAddress is a bytes type
						if (
							!cJSON_IsBytes(lpJsonAdapterUnicastAddress) ||
							(NULL == lpJsonAdapterUnicastAddress->valuestring)
							)
						{
							//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_WARNING,
								TEXT("cJSON_IsBytes(%p) failed."),
								lpJsonAdapterUnicastAddress
							);
							continue;
						}

						// Convert the data into the original bytes
						lpBuffer = cJSON_GetBytesValue(lpJsonAdapterUnicastAddress, &nBufferSize);
						if (NULL == lpBuffer)
						{
							//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_WARNING,
								TEXT("cJSON_GetBytesValue(%p, %p) failed."),
								lpJsonAdapterUnicastAddress,
								&nBufferSize
							);
							continue;
						}
						DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S[%d].%S[%d].iSockaddrLength: %d"), SurveyNetwork_Response_Adapters, dwAdapterIndex, SurveyNetwork_Response_AdapterUnicastAddresses, dwAddressIndex, nBufferSize);
						lpSockaddr = (LPSOCKADDR)lpBuffer;
						DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S[%d].%S[%d].lpSockaddr->sa_family: %04x"), SurveyNetwork_Response_Adapters, dwAdapterIndex, SurveyNetwork_Response_AdapterUnicastAddresses, dwAddressIndex, lpSockaddr->sa_family);
						if (16 == nBufferSize)
						{
							DBGPRINT(
								DEBUG_VERBOSE, 
								TEXT("  ->%S[%d].%S[%d].lpSockaddr->sa_data: %d.%d.%d.%d"), 
								SurveyNetwork_Response_Adapters, dwAdapterIndex, 
								SurveyNetwork_Response_AdapterUnicastAddresses, dwAddressIndex, 
								(BYTE)(lpSockaddr->sa_data[2]),
								(BYTE)(lpSockaddr->sa_data[3]),
								(BYTE)(lpSockaddr->sa_data[4]),
								(BYTE)(lpSockaddr->sa_data[5])
							);
						}

						dwAddressIndex++;

					} // end cJSON_ArrayForEach(lpJsonAdapterUnicastAddress, lpJsonAdapterUnicastAddresses)

					dwAdapterIndex++;

				} // end cJSON_ArrayForEach(lpJsonAdapter, lpJsonAdapters)

				dwIndex++;

			} // end cJSON_ArrayForEach(lpJsonSurveyNetworkResponseResult, lpJsonSurveyNetworkResponseResults)

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyNetworkOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonSurveyNetworkOutput);
				cJSON_Delete(lpJsonSurveyNetworkOutput);
				lpJsonSurveyNetworkOutput = NULL;
			}
			// Free configuration string
			if (NULL != szSurveyNetworkInput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("FREE(%p)"), szSurveyNetworkInput);
				FREE(szSurveyNetworkInput);
				szSurveyNetworkInput = NULL;
			}
			// Free output JSON strings
			if (NULL != szSurveyNetworkOutput)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_free(%p)"), szSurveyNetworkOutput);
				cJSON_free(szSurveyNetworkOutput);
				szSurveyNetworkOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the SurveyNetwork plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyNetwork plugin"));
			dwTempErrorCode = SurveyNetworkFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyNetworkFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_SurveyNetwork wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}