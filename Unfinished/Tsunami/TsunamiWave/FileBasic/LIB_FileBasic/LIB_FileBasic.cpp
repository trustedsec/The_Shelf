#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_FileBasic.h"
#include "Internal_Functions.h"


#define SEVEN_DAYS_IN_MILLISECONDS 604800000

// Global variables
cJSON* g_FileBasic_lpJsonConfig = NULL;


DWORD WINAPI FileBasicInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonFileBasicInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonFileBasicInitializeOutput = NULL;
	CHAR*	szFileBasicOutput = NULL;
	DWORD	dwFileBasicOutputSize = 0;

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
			lpJsonFileBasicInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonFileBasicInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonFileBasicInitializeInput:     %p"), lpJsonFileBasicInitializeInput);

			// Set the local module's configuration to the input configuration
			g_FileBasic_lpJsonConfig = lpJsonFileBasicInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonFileBasicInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_FileBasic_Kernel32_Functions()"));
			dwErrorCode = Initialize_FileBasic_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_FileBasic_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/
			

			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Upload_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_FileBasic_lpJsonConfig,
				FileBasic_Upload_Request,
				FileBasicUpload
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_FileBasic_lpJsonConfig,
					FileBasic_Upload_Request,
					FileBasicUpload
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Upload_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Download_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_FileBasic_lpJsonConfig,
				FileBasic_Download_Request,
				FileBasicDownload
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_FileBasic_lpJsonConfig,
					FileBasic_Download_Request,
					FileBasicDownload
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Download_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Delete_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_FileBasic_lpJsonConfig,
				FileBasic_Delete_Request,
				FileBasicDelete
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_FileBasic_lpJsonConfig,
					FileBasic_Delete_Request,
					FileBasicDelete
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register FileBasic_Delete_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szFileBasicOutput = cJSON_Print(lpJsonFileBasicInitializeOutput);
			//if (NULL == szFileBasicOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonFileBasicInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicOutput:     %p"), szFileBasicOutput);
			////printf("szFileBasicOutput:\n%s\n", szFileBasicOutput);
			//dwFileBasicOutputSize = (DWORD)strlen(szFileBasicOutput);

			//(*lppOutputBuffer) = (LPBYTE)szFileBasicOutput;
			//(*lpdwOutputBufferSize) = dwFileBasicOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonFileBasicInitializeInput)
			{
				cJSON_Delete(lpJsonFileBasicInitializeInput);
				lpJsonFileBasicInitializeInput = NULL;
			}
			if (NULL != lpJsonFileBasicInitializeOutput)
			{
				cJSON_Delete(lpJsonFileBasicInitializeOutput);
				lpJsonFileBasicInitializeOutput = NULL;
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


DWORD WINAPI FileBasicFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szFileBasicInput = NULL;
	DWORD	dwFileBasicInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szFileBasicOutput = NULL;
	DWORD	dwFileBasicOutputSize = 0;
	cJSON*	lpJsonFileBasicOutput = NULL;

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
			//szFileBasicInput = (char*)lpInputBuffer;
			//dwFileBasicInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister FileBasic_Upload_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_FileBasic_lpJsonConfig,
				FileBasic_Upload_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_FileBasic_lpJsonConfig,
					FileBasic_Upload_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister FileBasic_Upload_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister FileBasic_Download_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_FileBasic_lpJsonConfig,
				FileBasic_Download_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_FileBasic_lpJsonConfig,
					FileBasic_Download_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister FileBasic_Download_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_FileBasic_Kernel32_Functions()"));
			dwErrorCode = Finalize_FileBasic_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_FileBasic_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_FileBasic_lpJsonConfig)
			{
				cJSON_Delete(g_FileBasic_lpJsonConfig);
				g_FileBasic_lpJsonConfig = NULL;
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


DWORD WINAPI FileBasicUpload(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInput = NULL;
	DWORD	dwInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonFileName = NULL;
	CHAR*	szFileName = NULL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonFileContents = NULL;
	LPBYTE	lpFileContents = NULL;
	DWORD	dwFileContentsSize = 0;


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
			szInput = (char*)lpInputBuffer;
			dwInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonOutput = cJSON_CreateObject();
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the FileBasic_Upload_Request_File_Name
			lpJsonFileName = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Upload_Request_File_Name);
			if (NULL == lpJsonFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Upload_Request_File_Name
				);
				__leave;
			}
				
			
			// Check the FileBasic_Upload_Request_File_Name JSON type
			if (
				!cJSON_IsString(lpJsonFileName) ||
				( NULL == lpJsonFileName->valuestring )
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsString(%p) failed."),
					lpJsonFileName
				);
				__leave;
			}
			// Get the FileBasic_Upload_Request_File_Name value and check for sanity
			szFileName = lpJsonFileName->valuestring;
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileName:        %S"), szFileName);

			dwErrorCode = ReadFileIntoBuffer(szFileName, &lpFileContents, &dwFileContentsSize);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ReadFileIntoBuffer(%S, %p, %d) failed. (%08x)"),
					szFileName, &lpFileContents, &dwFileContentsSize,
					dwErrorCode
				);
				__leave;
			}
			
			// Create the FileBasic_Upload_Response_File_Contents JSON object
			lpJsonFileContents = cJSON_CreateBytes(lpFileContents, (INT)dwFileContentsSize);
			if (NULL == lpJsonFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileContents = cJSON_CreateBytes(%p, %d) failed.\n"),
					lpFileContents, dwFileContentsSize
				);
				__leave;
			}

			// Add the FileBasic_Upload_Response_File_Contents to the output
			cJSON_AddItemToObject(lpJsonOutput, FileBasic_Upload_Response_File_Contents, lpJsonFileContents);



			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szOutput = cJSON_Print(lpJsonOutput);
			if (NULL == szOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonOutput
				);
				__leave;
			}
			dwOutputSize = (DWORD)strlen(szOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szOutput;
			(*lpdwOutputBufferSize) = dwOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpFileContents)
			{
				FREE(lpFileContents);
				lpFileContents = NULL;
			}

			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
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

					cJSON_free((*lppOutputBuffer));
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


DWORD WINAPI FileBasicDownload(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInput = NULL;
	DWORD	dwInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonFileName = NULL;
	CHAR*	szFileName = NULL;
	cJSON*	lpJsonFileContents = NULL;
	LPBYTE	lpFileContents = NULL;
	DWORD	dwFileContentsSize = 0;
	cJSON*	lpCreationDisposition = NULL;
	DWORD	dwCreationDisposition = CREATE_NEW;
	cJSON*	lpCreationAttributes = NULL;
	DWORD	dwCreationAttributes = FILE_ATTRIBUTE_NORMAL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonOutputResponseErrorCode = NULL;


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
			szInput = (char*)lpInputBuffer;
			dwInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonOutput = cJSON_CreateObject();
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the FileBasic_Download_Request_File_Name
			lpJsonFileName = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Download_Request_File_Name);
			if (NULL == lpJsonFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Download_Request_File_Name
				);
				__leave;
			}
			// Get the FileBasic_Download_Request_File_Name value and check for sanity
			szFileName = cJSON_GetStringValue(lpJsonFileName);
			if (NULL == szFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonFileName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileName:        %S"), szFileName);


			// Get the FileBasic_Download_Request_File_Contents
			lpJsonFileContents = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Download_Request_File_Contents);
			if (NULL == lpJsonFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Download_Request_File_Contents
				);
				__leave;
			}
			// Get the FileBasic_Download_Request_File_Contents value and check for sanity
			lpFileContents = cJSON_GetBytesValue(lpJsonFileContents, (INT*)(&dwFileContentsSize));
			if (NULL == lpFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetBytesValue(%p,%d) failed."),
					lpJsonFileContents, (INT*)(&dwFileContentsSize)
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileContentsSize:    %d"), dwFileContentsSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpFileContents:        %p"), lpFileContents);


			// Get the FileBasic_Download_Request_Creation_Disposition
			lpCreationDisposition = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Download_Request_Creation_Disposition);
			if (NULL == lpCreationDisposition)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Download_Request_Creation_Disposition
				);
			}
			else
			{
				// Get the FileBasic_Download_Request_Creation_Disposition value and check for sanity
				if (!cJSON_IsNumber(lpCreationDisposition))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpCreationDisposition
					);
					__leave;
				}
				dwCreationDisposition = lpCreationDisposition->valueint;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCreationDisposition:    %08x"), dwCreationDisposition);


			// Get the FileBasic_Download_Request_Creation_Attributes
			lpCreationAttributes = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Download_Request_Creation_Attributes);
			if (NULL == lpCreationAttributes)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Download_Request_Creation_Attributes
				);
			}
			else
			{
				// Get the FileBasic_Download_Request_Creation_Attributes value and check for sanity
				if (!cJSON_IsNumber(lpCreationAttributes))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpCreationAttributes
					);
					__leave;
				}
				dwCreationAttributes = lpCreationAttributes->valueint;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwCreationAttributes:    %08x"), dwCreationAttributes);


			// Actually write the file out to disk
			dwTempErrorCode = WriteFileOutToDisk(szFileName, lpFileContents, dwFileContentsSize, dwCreationDisposition, dwCreationAttributes );
			if (ERROR_SUCCESS != dwTempErrorCode)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("WriteFileOutToDisk(%S, %p, %d, %d, %d) failed. (%08x)"),
					szFileName, lpFileContents, dwFileContentsSize, dwCreationDisposition, dwCreationAttributes,
					dwTempErrorCode
				);
				//__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwTempErrorCode:    %08x"), dwTempErrorCode);


			// Create the FileBasic_Download_Response_ErrorCode JSON object
			lpJsonOutputResponseErrorCode = cJSON_CreateNumber(dwTempErrorCode);
			if (NULL == lpJsonOutputResponseErrorCode)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutputResponseErrorCode = cJSON_CreateNumber(%d) failed.\n"),
					dwTempErrorCode
				);
				__leave;
			}

			// Add the FileBasic_Download_Response_ErrorCode to the output
			cJSON_AddItemToObject(lpJsonOutput, FileBasic_Download_Response_ErrorCode, lpJsonOutputResponseErrorCode);



			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szOutput = cJSON_Print(lpJsonOutput);
			if (NULL == szOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonOutput
				);
				__leave;
			}
			dwOutputSize = (DWORD)strlen(szOutput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwOutputSize: %d"), dwOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szOutput:     %p"), szOutput);
			//printf("szOutput:\n%s\n", szOutput);

			(*lppOutputBuffer) = (LPBYTE)szOutput;
			(*lpdwOutputBufferSize) = dwOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpFileContents)
			{
				cJSON_free(lpFileContents);
				lpFileContents = NULL;
			}

			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
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

					cJSON_free((*lppOutputBuffer));
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



DWORD WINAPI FileBasicDelete(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szInput = NULL;
	DWORD	dwInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonFileName = NULL;
	CHAR*	szFileName = NULL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonDeleteResults = NULL;
	DWORD	dwDeleteResults = ERROR_SUCCESS;
	BOOL	bDeleteReturn = FALSE;


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
			szInput = (char*)lpInputBuffer;
			dwInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonInput = cJSON_Parse(szInput);
			if (NULL == lpJsonInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					szInput
				);
				__leave;
			}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonOutput = cJSON_CreateObject();
			if (NULL == lpJsonOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonOutput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			// Get the FileBasic_Delete_Request_File_Name
			lpJsonFileName = cJSON_GetObjectItemCaseSensitive(lpJsonInput, FileBasic_Delete_Request_File_Name);
			if (NULL == lpJsonFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, FileBasic_Delete_Request_File_Name
				);
				__leave;
			}

			// Check the FileBasic_Delete_Request_File_Name JSON type
			szFileName = cJSON_GetStringValue(lpJsonFileName);
			if (NULL == szFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetStringValue(%p) failed."),
					lpJsonFileName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileName:        %S"), szFileName);

			bDeleteReturn = m_FileBasic_DeleteFileA(szFileName);
			dwDeleteResults = GetLastError();
			if (FALSE == bDeleteReturn )
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("DeleteFileA(%S) failed. (%08x)"),
					szFileName, dwDeleteResults
				);
			}

			// Create the FileBasic_Delete_Response_Delete_Results JSON object
			lpJsonDeleteResults = cJSON_CreateNumber(dwDeleteResults);
			if (NULL == lpJsonDeleteResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_CreateNumber(%d) failed.\n"),
					dwDeleteResults
				);
				__leave;
			}

			// Add the FileBasic_Delete_Response_Delete_Results to the output
			cJSON_AddItemToObject(lpJsonOutput, FileBasic_Delete_Response_Delete_Results, lpJsonDeleteResults);


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szOutput = cJSON_Print(lpJsonOutput);
			if (NULL == szOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonOutput
				);
				__leave;
			}
			dwOutputSize = (DWORD)strlen(szOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("szJsonOutput:\n%s\n", szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szOutput;
			(*lpdwOutputBufferSize) = dwOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonInput)
			{
				cJSON_Delete(lpJsonInput);
				lpJsonInput = NULL;
			}

			if (NULL != lpJsonOutput)
			{
				cJSON_Delete(lpJsonOutput);
				lpJsonOutput = NULL;
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

					cJSON_free((*lppOutputBuffer));
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
