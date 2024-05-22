#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_FileBasic.h"
#include "LIB_FileBasic.h"

// Primary TsunamiWave Configuration
CHAR g_FileBasic_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_FileBasic_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;

#define STR_SYSTEM_COMMAND_FORMAT_STRING "fc /B %s %s"

DWORD wmain(INT argc, LPCWCHAR argv[], LPCWCHAR envp[])
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	LPBYTE	lpPackedConfigurationBuffer = NULL;
	DWORD	dwPackedConfigurationBufferSize = 0;
	LPBYTE	lpPackedConfigurationPasswordBuffer = NULL;
	DWORD	dwPackedConfigurationPasswordBufferSize = 0;
	CHAR*	szConfigurationJson = NULL;
	DWORD	dwConfigurationJsonSize = 0;
	CHAR*	szNewConfigurationJson = NULL;
	DWORD	dwNewConfigurationJsonSize = 0;

	cJSON*	lpJsonFileBasicInput = NULL;
	cJSON*	lpJsonFileBasicInputFileName = NULL;
	cJSON*	lpJsonFileBasicInputFileContents = NULL;
	cJSON*	lpJsonFileBasicInputCreationDisposition = NULL;
	cJSON*	lpJsonFileBasicInputCreationAttributes = NULL;

	CHAR*	szFileBasicInput = NULL;
	DWORD	dwFileBasicInputSize = 0;
	CHAR*	szFileBasicOutput = NULL;
	DWORD	dwFileBasicOutputSize = 0;

	cJSON*	lpJsonFileBasicOutput = NULL;
	cJSON*	lpJsonFileBasicOutputFileContents = NULL;
	cJSON*	lpJsonFileBasicOutputFileOperationResults = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;
	
	CHAR	szInputFilename[MAX_PATH];
	CHAR	szOutputFilename[MAX_PATH];

	LPBYTE	lpInputFileContents = NULL;
	DWORD	dwInputFileContentsSize = 0;

	LPBYTE	lpOutputFileContents = NULL;
	DWORD	dwOutputFileContentsSize = 0;

	CHAR	szSystemCommand[MAX_PATH];

	DWORD	dwFileOperationResults = ERROR_SUCCESS;

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_FileBasic wmain() start"));

			/********************************************************************************************
				Check Arguments
			********************************************************************************************/
			if (3 != argc)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("%s input_filename output_filename"), argv[0]);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("argv[1]: %s"), argv[1]);
			wcstombs(szInputFilename, argv[1], MAX_PATH);
			DBGPRINT(DEBUG_VERBOSE, TEXT("argv[2]: %s"), argv[2]);
			wcstombs(szOutputFilename, argv[2], MAX_PATH);

			DBGPRINT(DEBUG_VERBOSE, TEXT("szInputFilename: %S"), szInputFilename);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szOutputFilename: %S"), szOutputFilename);


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_FileBasic_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_FileBasic_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_FileBasic_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_FileBasic_szPackedConfigurationPassword);

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
				Initialize the FileBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the FileBasic plugin"));
			dwErrorCode = FileBasicInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FileBasicInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Use the FileBasicUpload
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the FileBasicUpload"));

			// Create the input JSON object for the FileBasicUpload
			lpJsonFileBasicInput = cJSON_CreateObject();
			if (NULL == lpJsonFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create FileBasic_Upload_Request_File_Name for the input
			lpJsonFileBasicInputFileName = cJSON_CreateString(szInputFilename);
			if (NULL == lpJsonFileBasicInputFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInputFileName = cJSON_CreateString(%S) failed."),
					szInputFilename
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFileBasicInput, FileBasic_Upload_Request_File_Name, lpJsonFileBasicInputFileName);


			// Create the string representation of the FileBasic_Request
			szFileBasicInput = cJSON_Print(lpJsonFileBasicInput);
			if (NULL == szFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szFileBasicInput = cJSON_Print(%p) failed."),
					szFileBasicInput
				);
				__leave;
			}
			dwFileBasicInputSize = (DWORD)strlen(szFileBasicInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicInputSize: %d"), dwFileBasicInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicInput:     %p"), szFileBasicInput);
			printf("szFileBasicInput: \n%s\n", szFileBasicInput);

			// Call the FileBasicUpload passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = FileBasicUpload(
				(LPBYTE)szFileBasicInput,
				dwFileBasicInputSize,
				(LPBYTE*)&szFileBasicOutput,
				&dwFileBasicOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FileBasicUpload(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szFileBasicInput,
					dwFileBasicInputSize,
					(LPBYTE*)&szFileBasicOutput,
					&dwFileBasicOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicOutputSize: %d"), dwFileBasicOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicOutput:     %p"), szFileBasicOutput);
			//printf("szFileBasicOutput:\n%s\n", szFileBasicOutput);


			// Parse the output string into a JSON object
			lpJsonFileBasicOutput = cJSON_Parse(szFileBasicOutput);
			if (NULL == lpJsonFileBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicOutput = cJSON_Parse(%p) failed."),
					szFileBasicOutput
				);
				__leave;
			}

			// Get the FileBasic_Upload_Response_File_Contents JSON object
			lpJsonFileBasicOutputFileContents = cJSON_GetObjectItemCaseSensitive(lpJsonFileBasicOutput, FileBasic_Upload_Response_File_Contents);
			if (NULL == lpJsonFileBasicOutputFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicOutputFileContents = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonFileBasicOutput, FileBasic_Upload_Response_File_Contents
				);
				__leave;
			}

			// Check the type and get the value of FileBasic_Response_Wake_Time
			lpInputFileContents = cJSON_GetBytesValue(lpJsonFileBasicOutputFileContents, (INT*)(&dwInputFileContentsSize));
			if (NULL == lpInputFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonFileBasicOutputFileContents, (INT*)(&dwInputFileContentsSize)
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInputFileContentsSize: %d"), dwInputFileContentsSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpInputFileContents:     %p"), lpInputFileContents);

			// Delete input and output JSON objects
			if (NULL != lpJsonFileBasicInput)
			{
				cJSON_Delete(lpJsonFileBasicInput);
				lpJsonFileBasicInput = NULL;
			}
			if (NULL != lpJsonFileBasicOutput)
			{
				cJSON_Delete(lpJsonFileBasicOutput);
				lpJsonFileBasicOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szFileBasicInput)
			{
				cJSON_free(szFileBasicInput);
				szFileBasicInput = NULL;
			}
			if (NULL != szFileBasicOutput)
			{
				cJSON_free(szFileBasicOutput);
				szFileBasicOutput = NULL;
			}


			/***************************************************************************
				Use the FileBasicDownload
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the FileBasicDownload"));

			// Create the input JSON object for the FileBasicDownload
			lpJsonFileBasicInput = cJSON_CreateObject();
			if (NULL == lpJsonFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create FileBasic_Download_Request_File_Name for the input
			lpJsonFileBasicInputFileName = cJSON_CreateString(szOutputFilename);
			if (NULL == lpJsonFileBasicInputFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInputFileName = cJSON_CreateString(%S) failed."),
					szOutputFilename
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFileBasicInput, FileBasic_Download_Request_File_Name, lpJsonFileBasicInputFileName);

			// Create FileBasic_Download_Request_File_Contents for the input
			lpJsonFileBasicInputFileContents = cJSON_CreateBytes(lpInputFileContents, (INT)dwInputFileContentsSize);
			if (NULL == lpJsonFileBasicInputFileContents)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInputFileName = cJSON_CreateBytes(%p,%d) failed."),
					lpInputFileContents, (INT)dwInputFileContentsSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFileBasicInput, FileBasic_Download_Request_File_Contents, lpJsonFileBasicInputFileContents);


			// Create the string representation of the FileBasic_Request
			szFileBasicInput = cJSON_Print(lpJsonFileBasicInput);
			if (NULL == szFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szFileBasicInput = cJSON_Print(%p) failed."),
					szFileBasicInput
				);
				__leave;
			}
			dwFileBasicInputSize = (DWORD)strlen(szFileBasicInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicInputSize: %d"), dwFileBasicInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicInput:     %p"), szFileBasicInput);
			printf("szFileBasicInput: \n%s\n", szFileBasicInput);

			// Call the FileBasicUpload passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = FileBasicDownload(
				(LPBYTE)szFileBasicInput,
				dwFileBasicInputSize,
				(LPBYTE*)&szFileBasicOutput,
				&dwFileBasicOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FileBasicDownload(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szFileBasicInput,
					dwFileBasicInputSize,
					(LPBYTE*)&szFileBasicOutput,
					&dwFileBasicOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicOutputSize: %d"), dwFileBasicOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicOutput:     %p"), szFileBasicOutput);
			//printf("szFileBasicOutput:\n%s\n", szFileBasicOutput);


			// Delete input and output JSON objects
			if (NULL != lpJsonFileBasicInput)
			{
				cJSON_Delete(lpJsonFileBasicInput);
				lpJsonFileBasicInput = NULL;
			}
			if (NULL != lpJsonFileBasicOutput)
			{
				cJSON_Delete(lpJsonFileBasicOutput);
				lpJsonFileBasicOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szFileBasicInput)
			{
				cJSON_free(szFileBasicInput);
				szFileBasicInput = NULL;
			}
			if (NULL != szFileBasicOutput)
			{
				cJSON_free(szFileBasicOutput);
				szFileBasicOutput = NULL;
			}


			/***************************************************************************
				Compare the file contents of the uploaded and downloaded file
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Comparing the file contents..."));
			ZeroMemory(szSystemCommand, MAX_PATH);
			sprintf_s(szSystemCommand, MAX_PATH, STR_SYSTEM_COMMAND_FORMAT_STRING, szInputFilename, szOutputFilename);
			system(szSystemCommand);



			/***************************************************************************
				Use the FileBasicDelete
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the FileBasicDelete"));

			// Create the input JSON object for the FileBasicDelete
			lpJsonFileBasicInput = cJSON_CreateObject();
			if (NULL == lpJsonFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create FileBasic_Delete_Request_File_Name for the output
			lpJsonFileBasicInputFileName = cJSON_CreateString(szOutputFilename);
			if (NULL == lpJsonFileBasicInputFileName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicInputFileName = cJSON_CreateString(%S) failed."),
					szOutputFilename
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFileBasicInput, FileBasic_Delete_Request_File_Name, lpJsonFileBasicInputFileName);


			// Create the string representation of the FileBasic_Request
			szFileBasicInput = cJSON_Print(lpJsonFileBasicInput);
			if (NULL == szFileBasicInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szFileBasicInput = cJSON_Print(%p) failed."),
					szFileBasicInput
				);
				__leave;
			}
			dwFileBasicInputSize = (DWORD)strlen(szFileBasicInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicInputSize: %d"), dwFileBasicInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicInput:     %p"), szFileBasicInput);
			printf("szFileBasicInput: \n%s\n", szFileBasicInput);

			// Call the FileBasicDelete passing in a NULL JSON input object and receiving the JSON output object
			dwErrorCode = FileBasicDelete(
				(LPBYTE)szFileBasicInput,
				dwFileBasicInputSize,
				(LPBYTE*)&szFileBasicOutput,
				&dwFileBasicOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FileBasicDelete(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szFileBasicInput,
					dwFileBasicInputSize,
					(LPBYTE*)&szFileBasicOutput,
					&dwFileBasicOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileBasicOutputSize: %d"), dwFileBasicOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFileBasicOutput:     %p"), szFileBasicOutput);
			//printf("szFileBasicOutput:\n%s\n", szFileBasicOutput);


			// Parse the output string into a JSON object
			lpJsonFileBasicOutput = cJSON_Parse(szFileBasicOutput);
			if (NULL == lpJsonFileBasicOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicOutput = cJSON_Parse(%p) failed."),
					szFileBasicOutput
				);
				__leave;
			}

			// Get the FileBasic_Delete_Response_File_Contents JSON object
			lpJsonFileBasicOutputFileOperationResults = cJSON_GetObjectItemCaseSensitive(lpJsonFileBasicOutput, FileBasic_Delete_Response_Delete_Results);
			if (NULL == lpJsonFileBasicOutputFileOperationResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFileBasicOutputFileOperationResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonFileBasicOutput, FileBasic_Delete_Response_Delete_Results
				);
				__leave;
			}

			// Check the type and get the value of FileBasic_Delete_Response_Delete_Results
			if (!cJSON_IsNumber(lpJsonFileBasicOutputFileOperationResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonFileBasicOutputFileOperationResults
				);
				__leave;
			}
			dwFileOperationResults = lpJsonFileBasicOutputFileOperationResults->valueint;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFileOperationResults: %08x (%d)"), dwFileOperationResults, dwFileOperationResults);


			// Delete input and output JSON objects
			if (NULL != lpJsonFileBasicInput)
			{
				cJSON_Delete(lpJsonFileBasicInput);
				lpJsonFileBasicInput = NULL;
			}
			if (NULL != lpJsonFileBasicOutput)
			{
				cJSON_Delete(lpJsonFileBasicOutput);
				lpJsonFileBasicOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szFileBasicInput)
			{
				cJSON_free(szFileBasicInput);
				szFileBasicInput = NULL;
			}
			if (NULL != szFileBasicOutput)
			{
				cJSON_free(szFileBasicOutput);
				szFileBasicOutput = NULL;
			}




		} // end try-finally
		__finally
		{
			// Delete the file contents
			if (NULL != lpInputFileContents)
			{
				cJSON_free(lpInputFileContents);
				lpInputFileContents = NULL;
			}
			if (NULL != lpOutputFileContents)
			{
				cJSON_free(lpOutputFileContents);
				lpOutputFileContents = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonFileBasicInput)
			{
				cJSON_Delete(lpJsonFileBasicInput);
				lpJsonFileBasicInput = NULL;
			}
			if (NULL != lpJsonFileBasicOutput)
			{
				cJSON_Delete(lpJsonFileBasicOutput);
				lpJsonFileBasicOutput = NULL;
			}

			// Free input and output JSON strings
			if (NULL != szFileBasicInput)
			{
				cJSON_free(szFileBasicInput);
				szFileBasicInput = NULL;
			}
			if (NULL != szFileBasicOutput)
			{
				cJSON_free(szFileBasicOutput);
				szFileBasicOutput = NULL;
			}

			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the FileBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the FileBasic plugin"));
			dwTempErrorCode = FileBasicFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("FileBasicFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_FileBasic wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
