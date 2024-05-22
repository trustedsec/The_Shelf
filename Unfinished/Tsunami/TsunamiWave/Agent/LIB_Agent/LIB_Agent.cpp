#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"
#include "LIB_LoaderMemoryModule.h"
#include "LIB_ManagerBasic.h"
#include "LIB_InternetConnect.h"
#include "LIB_SurveyFile.h"
#include "LIB_SurveyHost.h"
#include "LIB_SurveyNetwork.h"
#include "LIB_SurveyProcess.h"
#include "LIB_SurveyRegistry.h"
#include "LIB_ExecuteCommand.h"
#include "LIB_TriggerSleep.h"
#include "LIB_FileBasic.h"

#include "LIB_Agent.h"
#include "Internal_Functions.h"


// Global variables
cJSON*	g_Agent_lpJsonInitializationConfig = NULL;
cJSON*	g_Agent_lpJsonConfigurationSettings = NULL;



DWORD WINAPI AgentInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonAgentInitializeInput = NULL;

	CHAR*	szConfigurationJson = NULL;
	DWORD	dwConfigurationJsonSize = 0;
	CHAR*	szNewConfigurationJson = NULL;
	DWORD	dwNewConfigurationJsonSize = 0;

	BOOL	bRegisterResult = FALSE;

	//f_CallbackInterface fpRegisterFunction = NULL;
	//cJSON*	lpJsonRegister = NULL;
	//f_CallbackInterface fpUnregisterFunction = NULL;
	//cJSON*	lpJsonUnregister = NULL;

	cJSON*	lpJsonAgentInitializeOutput = NULL;
	CHAR*	szAgentOutput = NULL;
	DWORD	dwAgentOutputSize = 0;



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
			

			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			szConfigurationJson = (CHAR*)lpInputBuffer;
			dwConfigurationJsonSize = dwInputBufferSize;
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwConfigurationJsonSize: %d"), dwConfigurationJsonSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szConfigurationJson:     %p"), szConfigurationJson);
			//printf("[%s(%d)] szConfigurationJson:\n%s\n", __FILE__, __LINE__, szConfigurationJson);
			g_Agent_lpJsonConfigurationSettings = cJSON_Parse(szConfigurationJson);
			if (NULL == g_Agent_lpJsonConfigurationSettings)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)szConfigurationJson
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_Agent_lpJsonConfigurationSettings:     %p"), g_Agent_lpJsonConfigurationSettings);

			/********************************************************************************
				Initialize internal functions
			********************************************************************************/


			/***************************************************************************
				Initialize the ManagerBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the ManagerBasic plugin"));
			dwErrorCode = ManagerBasicInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				(LPBYTE*)(&szNewConfigurationJson),
				&dwNewConfigurationJsonSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ManagerBasicInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					(LPBYTE*)(&szNewConfigurationJson),
					&dwNewConfigurationJsonSize,
					dwErrorCode
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwNewConfigurationJsonSize: %d"), dwNewConfigurationJsonSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szNewConfigurationJson:     %p"), szNewConfigurationJson);
			//printf("[%s(%d)] szNewConfigurationJson:\n%s\n", __FILE__, __LINE__, szNewConfigurationJson);


			/***************************************************************************
				Update the configuration string and JSON
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Update the configuration string and JSON"));
			// Set the configuration string to the new configuration string
			// which has the register and unregister functions filled in
			szConfigurationJson = szNewConfigurationJson;
			szNewConfigurationJson = NULL;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			lpJsonAgentInitializeInput = cJSON_Parse(szConfigurationJson);
			if (NULL == lpJsonAgentInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonAgentInitializeInput:     %p"), lpJsonAgentInitializeInput);

			// Set the local plugin's configuration to the input configuration
			g_Agent_lpJsonInitializationConfig = lpJsonAgentInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonAgentInitializeInput = NULL;
			

			/***************************************************************************
				Initialize the LoaderMemoryModule plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the LoaderMemoryModule plugin"));
			dwErrorCode = LoaderMemoryModuleInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Initialize the InternetConnect plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the InternetConnect plugin"));
			dwErrorCode = InternetConnectInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InternetConnectInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Initialize the SurveyHost plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyHost plugin"));
			dwErrorCode = SurveyHostInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyHostInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Initialize the SurveyFile plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyFile plugin"));
			dwErrorCode = SurveyFileInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyFileInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


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
				Initialize the SurveyProcess plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyProcess plugin"));
			dwErrorCode = SurveyProcessInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyProcessInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Initialize the SurveyRegistry plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the SurveyRegistry plugin"));
			dwErrorCode = SurveyRegistryInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyRegistryInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}

			/***************************************************************************
				Initialize the ExecuteCommand plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the ExecuteCommand plugin"));
			dwErrorCode = ExecuteCommandInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ExecuteCommandInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}

			/***************************************************************************
				Initialize the TriggerSleep plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the TriggerSleep plugin"));
			dwErrorCode = TriggerSleepInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TriggerSleepInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}

			/***************************************************************************
				Initialize the FileBasic pluginRegisterModuleMessageHandler
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


			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_GetSettings_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_GetSettings_Request,
				AgentGetSettings
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_GetSettings_Request,
					AgentGetSettings
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_GetSettings_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_SetSettings_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_SetSettings_Request,
				AgentSetSettings
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_SetSettings_Request,
					AgentSetSettings
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_SetSettings_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_LoadPlugin_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_LoadPlugin_Request,
				AgentLoadPlugin
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_LoadPlugin_Request,
					AgentLoadPlugin
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_LoadPlugin_Request was successful."));
			}
	
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_UnloadPlugin_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_UnloadPlugin_Request,
				AgentUnloadPlugin
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_UnloadPlugin_Request,
					AgentUnloadPlugin
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_UnloadPlugin_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_ListPlugins_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_ListPlugins_Request,
				AgentListPlugins
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_ListPlugins_Request,
					AgentListPlugins
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register Agent_ListPlugins_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szAgentOutput = szConfigurationJson;
			if (NULL == szAgentOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonAgentInitializeOutput
				);
				__leave;
			}
			dwAgentOutputSize = (DWORD)strlen(szAgentOutput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwAgentOutputSize: %d"), dwAgentOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szAgentOutput:     %p"), szAgentOutput);
			//printf("szAgentOutput:\n%s\n", szAgentOutput);

			(*lppOutputBuffer) = (LPBYTE)szAgentOutput;
			(*lpdwOutputBufferSize) = dwAgentOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonAgentInitializeInput)
			{
				cJSON_Delete(lpJsonAgentInitializeInput);
				lpJsonAgentInitializeInput = NULL;
			}
			if (NULL != lpJsonAgentInitializeOutput)
			{
				cJSON_Delete(lpJsonAgentInitializeOutput);
				lpJsonAgentInitializeOutput = NULL;
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


DWORD WINAPI AgentFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	BOOL	bRegisterResult = FALSE;

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
			//szAgentInput = (char*)lpInputBuffer;
			//dwAgentInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister Agent_GetSettings_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_GetSettings_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_GetSettings_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister Agent_GetSettings_Request was successful."));
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister Agent_SetSettings_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_Agent_lpJsonInitializationConfig,
				Agent_SetSettings_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_Agent_lpJsonInitializationConfig,
					Agent_SetSettings_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister Agent_SetSettings_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/

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


			/***************************************************************************
				Finalize the TriggerSleep plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the TriggerSleep plugin"));
			dwTempErrorCode = TriggerSleepFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("TriggerSleepFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			
			/***************************************************************************
				Finalize the ExecuteCommand plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the ExecuteCommand plugin"));
			dwTempErrorCode = ExecuteCommandFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ExecuteCommandFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the SurveyHost plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyHost plugin"));
			dwTempErrorCode = SurveyHostFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyHostFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the SurveyFile plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyFile plugin"));
			dwTempErrorCode = SurveyFileFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyFileFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
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


			/***************************************************************************
				Finalize the SurveyProcess plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyProcess plugin"));
			dwTempErrorCode = SurveyProcessFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyProcessFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the SurveyRegistry plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the SurveyRegistry plugin"));
			dwTempErrorCode = SurveyRegistryFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("SurveyRegistryFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the InternetConnect plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the InternetConnect plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("InternetConnectFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = InternetConnectFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("InternetConnectFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/***************************************************************************
				Finalize the LoaderMemoryModule plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the LoaderMemoryModule plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = LoaderMemoryModuleFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			
			/***************************************************************************
				Finalize the ManagerBasic plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the ManagerBasic plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("ManagerBasicFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = ManagerBasicFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("ManagerBasicFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("FinalizeAdvapi32Functions()"));
			//dwTempErrorCode = FinalizeAdvapi32Functions();
			//if (ERROR_SUCCESS != dwTempErrorCode)
			//{
			//	DBGPRINT(DEBUG_ERROR, TEXT("FinalizeAdvapi32Functions() failed.(%08x)"), dwTempErrorCode);
			//	//__leave;
			//}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_Agent_lpJsonInitializationConfig)
			{
				cJSON_Delete(g_Agent_lpJsonInitializationConfig);
				g_Agent_lpJsonInitializationConfig = NULL;
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


DWORD WINAPI AgentCallback(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szAgentInput = NULL;
	DWORD	dwAgentInputSize = 0;
	cJSON*	lpJsonAgentInput = NULL;

	cJSON*	lpJsonBinaryId = NULL;
	DWORD	dwBinaryId = 0;
	cJSON*	lpJsonAgentId = NULL;
	DWORD	dwAgentId = 0;

	cJSON*	lpJsonAgentOutput = NULL;
	CHAR*	szAgentOutput = NULL;
	DWORD	dwAgentOutputSize = 0;


	// Initial messages
	cJSON*	lpJsonInitialMessages = NULL;
	cJSON*	lpJsonInitialMessage = NULL;


	// Periodic messages stuff
	DWORD	dwPeriodicCount = 0;
	cJSON*	lpJsonPeriodicMessages = NULL;
	cJSON*	lpJsonPeriodicMessage = NULL;



	cJSON*	lpJsonMessagesToSend = NULL;


	// InternetConnect stuff
	cJSON*	lpJsonSendRequest = NULL;
	cJSON*	lpJsonSendRequestBuffer = NULL;
	cJSON*	lpJsonSendResponseBuffer = NULL;

	cJSON*	lpJsonInternetConnectInput = NULL;
	CHAR*	szInternetConnectInput = NULL;
	DWORD	dwInternetConnectInputSize = 0;

	cJSON*	lpJsonInternetConnectOutput = NULL;
	CHAR*	szInternetConnectOutput = NULL;
	DWORD	dwInternetConnectOutputSize = 0;

	CHAR*	szRequestBuffer = NULL;
	DWORD	dwRequestBufferSize = 0;

	CHAR*	szResponseBuffer = NULL;
	DWORD	dwResponseBufferSize = 0;


	__try
	{
		__try
		{
			DBGPRINT(DEBUG_INFO, TEXT("===================================================================================================================================="));

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
			//szAgentInput = (char*)lpInputBuffer;
			//dwAgentInputSize = dwInputBufferSize;


			/********************************************************************************
				Parse configuration
			********************************************************************************/
			// Get the binary id
			lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(g_Agent_lpJsonConfigurationSettings, TSM_CONFIGURATION_BINARY_ID);
			if (NULL == lpJsonBinaryId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonBinaryId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_Agent_lpJsonConfigurationSettings,
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
			dwBinaryId = lpJsonBinaryId->valueint;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwBinaryId: %d"), dwBinaryId );

			// Get the agent id
			lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(g_Agent_lpJsonConfigurationSettings, TSM_CONFIGURATION_AGENT_ID);
			if (NULL == lpJsonAgentId)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentId = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_Agent_lpJsonConfigurationSettings,
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
			dwAgentId = lpJsonAgentId->valueint;
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwAgentId: %d"), dwAgentId);

			// Get the Agent_Initial_Messages
			lpJsonInitialMessages = cJSON_GetObjectItemCaseSensitive(g_Agent_lpJsonConfigurationSettings, Agent_Initial_Messages);
			if (NULL == lpJsonInitialMessages)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInitialMessages = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					g_Agent_lpJsonConfigurationSettings,
					Agent_Initial_Messages
				);
				__leave;
			}
			if (!cJSON_IsArray(lpJsonInitialMessages))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsArray(%p) failed."),
					lpJsonInitialMessages
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonInitialMessages: %p"), lpJsonInitialMessages);

			


			/********************************************************************************
				Perform initial actions 
			********************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Performing initial actions..."));
			printf("[%s(%d)] Performing initial actions...\n", __FILE__, __LINE__);

			// Create array of messages to send
			if (NULL == lpJsonMessagesToSend)
			{
				lpJsonMessagesToSend = cJSON_CreateArray();
				if (NULL == lpJsonMessagesToSend)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonMessagesToSend = cJSON_CreateArray failed.")
					);
					__leave;
				}
			}

			// Loop through each initial message and run the command using ManagerBasicDispatchMessage
			cJSON_ArrayForEach(lpJsonInitialMessage, lpJsonInitialMessages)
			{
				cJSON*	lpJsonInitialMessageOutput = NULL;
				CHAR*	szInitialMessageInput = NULL;
				DWORD	dwInitialMessageInputSize = 0;
				CHAR*	szInitialMessageOutput = NULL;
				DWORD	dwInitialMessageOutputSize = 0;

				szInitialMessageInput = cJSON_Print(lpJsonInitialMessage);
				if (NULL == szInitialMessageInput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("szInitialMessageInput = cJSON_Print(%p) failed."),
						lpJsonInitialMessage
					);
					__leave;
				}
				dwInitialMessageInputSize = (DWORD)strlen(szInitialMessageInput);
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitialMessageInputSize: %d"), dwInitialMessageInputSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("szInitialMessageInput:     %p"), szInitialMessageInput);
				printf("[%s(%d)] szInitialMessageInput:\n%s\n", __FILE__, __LINE__, szInitialMessageInput);


				// Call the ManagerBasicDispatchMessage
				dwErrorCode = ManagerBasicCallbackDispatchMessage(
					(LPBYTE)szInitialMessageInput,
					dwInitialMessageInputSize,
					(LPBYTE*)&szInitialMessageOutput,
					&dwInitialMessageOutputSize
				);
				if (ERROR_SUCCESS != dwErrorCode)
				{
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("ManagerBasicCallbackDispatchMessage(%p, %d, %p, %p) failed.(%08x)"),
						(LPBYTE)szInitialMessageInput,
						dwInitialMessageInputSize,
						(LPBYTE*)&szInitialMessageOutput,
						&dwInitialMessageOutputSize,
						dwErrorCode
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitialMessageOutputSize: %d"), dwInitialMessageOutputSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("szInitialMessageOutput:     %p"), szInitialMessageOutput);
				//printf("[%s(%d)] szInitialMessageOutput:\n%s\n", __FILE__, __LINE__, szInitialMessageOutput);

				// Parse the output
				lpJsonInitialMessageOutput = cJSON_Parse(szInitialMessageOutput);
				if (NULL == lpJsonInitialMessageOutput)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonInitialMessageOutput = cJSON_Parse(%p) failed."),
						szInitialMessageOutput
					);
					__leave;
				}
				
				// Add response to messages to send
				cJSON_AddItemToArray(lpJsonMessagesToSend, lpJsonInitialMessageOutput);

				// Free input and output JSON strings
				if (NULL != szInitialMessageInput)
				{
					cJSON_free(szInitialMessageInput);
					szInitialMessageInput = NULL;
				}

				if (NULL != szInitialMessageOutput)
				{
					cJSON_free(szInitialMessageOutput);
					szInitialMessageOutput = NULL;
				}

			} // end cJSON_ArrayForEach(lpJsonInitialMessage, lpJsonInitialMessages)



			// Run the Agent's periodic loop
			do
			{
				DBGPRINT(DEBUG_INFO, TEXT("------------------------------------------------------------------------------------------------------------------------------------"));
				

				// Run the Agent's callback loop

				do
				{
					printf("[%s(%d)] Sending messages...\n", __FILE__, __LINE__);

					/***************************************************************************
						Send any results to the server (InternetConnectSendRequest)
					***************************************************************************/
					DBGPRINT(DEBUG_INFO, TEXT("Use the InternetConnectSendRequest"));

					// Free the request buffer sent to the server
					if (NULL != szRequestBuffer)
					{
						cJSON_free(szRequestBuffer);
						szRequestBuffer = NULL;
					}

					// Free the response buffer sent to the server
					if (NULL != szResponseBuffer)
					{
						cJSON_free(szResponseBuffer);
						szResponseBuffer = NULL;
					}

					// Delete input and output JSON objects
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

					// Free input and output JSON strings
					if (NULL != szInternetConnectInput)
					{
						cJSON_free(szInternetConnectInput);
						szInternetConnectInput = NULL;
					}
					if (NULL != szInternetConnectOutput)
					{
						cJSON_free(szInternetConnectOutput);
						szInternetConnectOutput = NULL;
					}


					// Create the request from messages to send
					szRequestBuffer = cJSON_Print(lpJsonMessagesToSend);
					if (NULL == szRequestBuffer)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("szRequestBuffer = cJSON_Print(%p) failed."),
							lpJsonMessagesToSend
						);
						__leave;
					}
					dwRequestBufferSize = (DWORD)strlen(szRequestBuffer);
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwRequestBufferSize: %d"), dwRequestBufferSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("szRequestBuffer:     %p"), szRequestBuffer);
					//printf("[%s(%d)] szRequestBuffer:\n%s\n", __FILE__, __LINE__, szRequestBuffer);


					// Create the input JSON object for the callback
					lpJsonInternetConnectInput = cJSON_CreateObject();
					if (NULL == lpJsonInternetConnectInput)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonInternetConnectInput = cJSON_CreateObject failed.\n")
						);
						__leave;
					}

					// Create the InternetConnect_Send_Request JSON object
					lpJsonSendRequest = cJSON_AddObjectToObject(lpJsonInternetConnectInput, InternetConnect_Send_Request);
					if (NULL == lpJsonSendRequest)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonSendRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
							lpJsonInternetConnectInput, InternetConnect_Send_Request
						);
						__leave;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSendRequest:    %p"), lpJsonSendRequest);

					// Create the InternetConnect_Send_Request_Buffer input JSON object
					lpJsonSendRequestBuffer = cJSON_CreateBytes((PBYTE)szRequestBuffer, dwRequestBufferSize);
					if (NULL == lpJsonSendRequestBuffer)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonSendRequestBuffer = cJSON_CreateBytes(%p,%d) failed."),
							szRequestBuffer, dwRequestBufferSize
						);
						__leave;
					}
					cJSON_AddItemToObject(lpJsonSendRequest, InternetConnect_Send_Request_Buffer, lpJsonSendRequestBuffer);

					// Create the string representation of the InternetConnect_Send_Request
					szInternetConnectInput = cJSON_Print(lpJsonInternetConnectInput);
					if (NULL == szInternetConnectInput)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("szInternetConnectInput = cJSON_Print(%p) failed."),
							lpJsonInternetConnectInput
						);
						__leave;
					}
					dwInternetConnectInputSize = (DWORD)strlen(szInternetConnectInput);
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwInternetConnectInputSize: %d"), dwInternetConnectInputSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectInput:     %p"), szInternetConnectInput);
					//printf("szInternetConnectInput: \n%s\n", szInternetConnectInput);

					// Call the InternetConnectSendRequest passing in the JSON input object and receiving the JSON output object
					DBGPRINT(
						DEBUG_VERBOSE,
						TEXT("InternetConnectSendRequest(%p, %d, %p, %p)"),
						(LPBYTE)szInternetConnectInput,
						dwInternetConnectInputSize,
						(LPBYTE*)&szInternetConnectOutput,
						&dwInternetConnectOutputSize
					);
					dwErrorCode = InternetConnectSendRequest(
						(LPBYTE)szInternetConnectInput,
						dwInternetConnectInputSize,
						(LPBYTE*)&szInternetConnectOutput,
						&dwInternetConnectOutputSize
					);
					if (ERROR_SUCCESS != dwErrorCode)
					{
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("InternetConnectSendRequest(%p, %d, %p, %p) failed.(%08x)"),
							(LPBYTE)szInternetConnectInput,
							dwInternetConnectInputSize,
							(LPBYTE*)&szInternetConnectOutput,
							&dwInternetConnectOutputSize,
							dwErrorCode
						);
						//__leave;

						DBGPRINT(DEBUG_INFO, TEXT("Could not connect to server. Breaking the callback loop..."));
						break;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwInternetConnectOutputSize: %d"), dwInternetConnectOutputSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("szInternetConnectOutput:     %p"), szInternetConnectOutput);
					//printf("szInternetConnectOutput:\n%s\n", szInternetConnectOutput);


					// TODO: Successfully sent the messages???
					printf("[%s(%d)] Received %d bytes...\n", __FILE__, __LINE__, dwInternetConnectOutputSize);


					// Delete the messages to send JSON objects since we just sent them
					if (NULL != lpJsonMessagesToSend)
					{
						cJSON_Delete(lpJsonMessagesToSend);
						lpJsonMessagesToSend = NULL;
					}


					// Check if we received any bytes from the server, and if so, try to parse it
					if (0 == dwInternetConnectOutputSize)
					{
						DBGPRINT(DEBUG_INFO, TEXT("No commands. Breaking the callback loop..."));
						break;
					}
					

					// Parse the output string into a JSON object
					lpJsonInternetConnectOutput = cJSON_Parse(szInternetConnectOutput);
					if (NULL == lpJsonInternetConnectOutput)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonInternetConnectOutput = cJSON_Parse(%p) failed."),
							szInternetConnectOutput
						);
						__leave;
					}


					// Get and display the InternetConnect_Send_Response_Buffer
					lpJsonSendResponseBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonInternetConnectOutput, InternetConnect_Send_Response_Buffer);
					if (NULL == lpJsonSendResponseBuffer)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("lpJsonSendResponseBuffer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
							lpJsonInternetConnectOutput, InternetConnect_Send_Response_Buffer
						);
						__leave;
					}
					if (
						(!cJSON_IsBytes(lpJsonSendResponseBuffer)) ||
						(NULL == lpJsonSendResponseBuffer->valuestring)
						)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_IsBytes(%p) failed."),
							lpJsonSendResponseBuffer
						);
						__leave;
					}
					szResponseBuffer = (CHAR*)cJSON_GetBytesValue(lpJsonSendResponseBuffer, (int*)(&dwResponseBufferSize));
					if (NULL == szResponseBuffer)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_WARNING,
							TEXT("cJSON_GetBytesValue(%p, %p) failed."),
							lpJsonSendResponseBuffer, &dwResponseBufferSize
						);
						__leave;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("  dwResponseBufferSize: %d"), dwResponseBufferSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("  szResponseBuffer:     %p"), szResponseBuffer);

					// If we got a response from the server, try to execute the command
					if (2 < dwResponseBufferSize)
					{
						printf("[%s(%d)] szResponseBuffer:\n%s\n", __FILE__, __LINE__, szResponseBuffer);

						Sleep(1000);



						cJSON*	lpJsonCommandOutput = NULL;
						CHAR*	szCommandOutput = NULL;
						DWORD	dwCommandOutputSize = 0;

						/***************************************************************************
							Use the ManagerBasicDispatchMessage
						***************************************************************************/
						DBGPRINT(DEBUG_INFO, TEXT("Use the ManagerBasicDispatchMessage"));

						// Call the ManagerBasicDispatchMessage
						dwErrorCode = ManagerBasicCallbackDispatchMessage(
							(LPBYTE)szResponseBuffer,
							dwResponseBufferSize,
							(LPBYTE*)&szCommandOutput,
							&dwCommandOutputSize
						);
						if (ERROR_SUCCESS != dwErrorCode)
						{
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("ManagerBasicCallbackDispatchMessage(%p, %d, %p, %p) failed.(%08x)"),
								(LPBYTE)szResponseBuffer,
								dwResponseBufferSize,
								(LPBYTE*)&szCommandOutput,
								&dwCommandOutputSize,
								dwErrorCode
							);
							__leave;
						}
						DBGPRINT(DEBUG_VERBOSE, TEXT("dwCommandOutputSize: %d"), dwCommandOutputSize);
						DBGPRINT(DEBUG_VERBOSE, TEXT("szCommandOutput:     %p"), szCommandOutput);
						//printf("[%s(%d)] szCommandOutput:\n%s\n", __FILE__, __LINE__, szCommandOutput);

						// Parse the output string into a JSON object
						lpJsonCommandOutput = cJSON_Parse(szCommandOutput);
						if (NULL == lpJsonCommandOutput)
						{
							dwErrorCode = ERROR_OBJECT_NOT_FOUND;
							DBGPRINT(
								DEBUG_ERROR,
								TEXT("lpJsonCommandOutput = cJSON_Parse(%p) failed."),
								szCommandOutput
							);
							__leave;
						}

						// Free the input string
						if (NULL != szResponseBuffer)
						{
							cJSON_free(szResponseBuffer);
							szResponseBuffer = NULL;
						}

						// Free the output string
						if (NULL != szCommandOutput)
						{
							cJSON_free(szCommandOutput);
							szCommandOutput = NULL;
						}

						// Create array of messages to send (if necessary)
						if (NULL == lpJsonMessagesToSend)
						{
							lpJsonMessagesToSend = cJSON_CreateArray();
							if (NULL == lpJsonMessagesToSend)
							{
								dwErrorCode = ERROR_OBJECT_NOT_FOUND;
								DBGPRINT(
									DEBUG_ERROR,
									TEXT("lpJsonMessagesToSend = cJSON_CreateArray failed.")
								);
								__leave;
							}
						}

						// Add the command response to the array of messages to send
						cJSON_AddItemToArray(lpJsonMessagesToSend, lpJsonCommandOutput);

						// DO NOT FREE the lpJsonCommandOutput else it will cause an access violation 
						// because the lpJsonMessagesToSend still has a reference to this object

					} // end if dwResponseBufferSize > 0
					else
					{
						DBGPRINT(DEBUG_INFO, TEXT("No more commands.  Breaking the callback loop..."));
						break;
					}

				} while (true); // End of Agent's callback loop

				
				
				

				/********************************************************************************
					Perform periodic actions
				********************************************************************************/

				// Get the Agent_Periodic_Messages
				lpJsonPeriodicMessages = cJSON_GetObjectItemCaseSensitive(g_Agent_lpJsonConfigurationSettings, Agent_Periodic_Messages);
				if (NULL == lpJsonPeriodicMessages)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonPeriodicMessages = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
						g_Agent_lpJsonConfigurationSettings,
						Agent_Periodic_Messages
					);
					__leave;
				}
				if (!cJSON_IsArray(lpJsonPeriodicMessages))
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsArray(%p) failed."),
						lpJsonPeriodicMessages
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonPeriodicMessages: %p"), lpJsonPeriodicMessages);

				dwPeriodicCount++;
				DBGPRINT(DEBUG_VERBOSE, TEXT("dwPeriodicCount: %d"), dwPeriodicCount);
				printf("[%s(%d)] Periodic loop #%d..\n", __FILE__, __LINE__, dwPeriodicCount);

				// Create array of messages to send (if necessary)
				if (NULL == lpJsonMessagesToSend)
				{
					lpJsonMessagesToSend = cJSON_CreateArray();
					if (NULL == lpJsonMessagesToSend)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonMessagesToSend = cJSON_CreateArray failed.")
						);
						__leave;
					}
				}

				// Loop through each periodic message and run the command using ManagerBasicDispatchMessage
				cJSON_ArrayForEach(lpJsonPeriodicMessage, lpJsonPeriodicMessages)
				{
					cJSON*	lpJsonPeriodicMessageOutput = NULL;
					CHAR*	szPeriodicMessageInput = NULL;
					DWORD	dwPeriodicMessageInputSize = 0;
					CHAR*	szPeriodicMessageOutput = NULL;
					DWORD	dwPeriodicMessageOutputSize = 0;

					cJSON* lpJsonPeriodicCount = cJSON_CreateNumber(dwPeriodicCount);
					if (NULL == lpJsonPeriodicCount)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonPeriodicCount = cJSON_CreateNumber(%d) failed."),
							dwPeriodicCount
						);
						__leave;
					}

					
					szPeriodicMessageInput = cJSON_Print(lpJsonPeriodicMessage);
					if (NULL == szPeriodicMessageInput)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("szPeriodicMessageInput = cJSON_Print(%p) failed."),
							lpJsonPeriodicMessage
						);
						__leave;
					}
					dwPeriodicMessageInputSize = (DWORD)strlen(szPeriodicMessageInput);
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwPeriodicMessageInputSize: %d"), dwPeriodicMessageInputSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("szPeriodicMessageInput:     %p"), szPeriodicMessageInput);
					//printf("[%s(%d)] szPeriodicMessageInput:\n%s\n", __FILE__, __LINE__, szPeriodicMessageInput);


					// Call the ManagerBasicDispatchMessage
					dwErrorCode = ManagerBasicCallbackDispatchMessage(
						(LPBYTE)szPeriodicMessageInput,
						dwPeriodicMessageInputSize,
						(LPBYTE*)&szPeriodicMessageOutput,
						&dwPeriodicMessageOutputSize
					);
					if (ERROR_SUCCESS != dwErrorCode)
					{
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("ManagerBasicCallbackDispatchMessage(%p, %d, %p, %p) failed.(%08x)"),
							(LPBYTE)szPeriodicMessageInput,
							dwPeriodicMessageInputSize,
							(LPBYTE*)&szPeriodicMessageOutput,
							&dwPeriodicMessageOutputSize,
							dwErrorCode
						);
						__leave;
					}
					DBGPRINT(DEBUG_VERBOSE, TEXT("dwPeriodicMessageOutputSize: %d"), dwPeriodicMessageOutputSize);
					DBGPRINT(DEBUG_VERBOSE, TEXT("szPeriodicMessageOutput:     %p"), szPeriodicMessageOutput);
					//printf("[%s(%d)] szPeriodicMessageOutput:\n%s\n", __FILE__, __LINE__, szPeriodicMessageOutput);

					// Parse the output
					lpJsonPeriodicMessageOutput = cJSON_Parse(szPeriodicMessageOutput);
					if (NULL == lpJsonPeriodicMessageOutput)
					{
						dwErrorCode = ERROR_OBJECT_NOT_FOUND;
						DBGPRINT(
							DEBUG_ERROR,
							TEXT("lpJsonPeriodicMessageOutput = cJSON_Parse(%p) failed."),
							szPeriodicMessageOutput
						);
						__leave;
					}

					// Update the message id to the periodic count
					cJSON_ReplaceItemInObject(lpJsonPeriodicMessageOutput, TSM_MANAGER_DISPATCH_MESSAGE_REQUEST_MESSAGE_ID, lpJsonPeriodicCount);
					
					// Add response to messages to send
					cJSON_AddItemToArray(lpJsonMessagesToSend, lpJsonPeriodicMessageOutput);

					// DO NOT FREE the lpJsonCommandOutput else it will cause an access violation 
					// because the lpJsonMessagesToSend still has a reference to this object

					// Free the input string
					if (NULL != szPeriodicMessageInput)
					{
						cJSON_free(szPeriodicMessageInput);
						szPeriodicMessageInput = NULL;
					}

					// Free the output string
					if (NULL != szPeriodicMessageOutput)
					{
						cJSON_free(szPeriodicMessageOutput);
						szPeriodicMessageOutput = NULL;
					}

				} // end cJSON_ArrayForEach(lpJsonPeriodicMessage, lpJsonPeriodicMessages)

			} while (true); // End of Agent's periodic loop

			












		} // end try-finally
		__finally
		{
			DBGPRINT(DEBUG_INFO, TEXT("===================================================================================================================================="));

			if (NULL != lpJsonAgentInput)
			{
				cJSON_Delete(lpJsonAgentInput);
				lpJsonAgentInput = NULL;
			}

			if (NULL != lpJsonAgentOutput)
			{
				cJSON_Delete(lpJsonAgentOutput);
				lpJsonAgentOutput = NULL;
			}
			
			if (NULL != szRequestBuffer)
			{
				cJSON_free(szRequestBuffer);
				szRequestBuffer = NULL;
			}

			if (NULL != szResponseBuffer)
			{
				cJSON_free(szResponseBuffer);
				szResponseBuffer = NULL;
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

					cJSON_free(szAgentOutput);
					*lppOutputBuffer = NULL;
				}
				*lpdwOutputBufferSize = 0;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("end"));

			DBGPRINT(DEBUG_INFO, TEXT("===================================================================================================================================="));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}


DWORD WINAPI AgentGetSettings(
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

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	CHAR*	szCurrentSettings = NULL;


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
			//szInput = (char*)lpInputBuffer;
			//dwInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			//lpJsonInput = cJSON_Parse(szInput);
			//if (NULL == lpJsonInput)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Parse(%p) failed."),
			//		szInput
			//	);
			//	__leave;
			//}


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
			szCurrentSettings = cJSON_Print(g_Agent_lpJsonConfigurationSettings);
			if (NULL == szCurrentSettings)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					g_Agent_lpJsonInitializationConfig
				);
				__leave;
			}
			cJSON_AddStringToObject( lpJsonOutput, Agent_GetSettings_Response_Settings, szCurrentSettings );
		


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
			if (NULL != szCurrentSettings)
			{
				cJSON_free(szCurrentSettings);
				szCurrentSettings = NULL;
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


DWORD WINAPI AgentSetSettings(
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
	cJSON*	lpJsonSetSettings = NULL;
	CHAR*	lpPluginBuffer = NULL;
	cJSON*	lpJsonNewSettings = NULL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	


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
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonInput: %p"), lpJsonInput);

			// Get the Agent_SetSettings_Request_Settings
			lpJsonSetSettings = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_SetSettings_Request_Settings);
			if (NULL == lpJsonSetSettings)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, Agent_SetSettings_Request_Settings
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonSetSettings: %p"), lpJsonSetSettings);

			lpPluginBuffer = cJSON_GetStringValue(lpJsonSetSettings);
			if (NULL == lpPluginBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpPluginBuffer = cJSON_GetStringValue(%p) failed."),
					lpJsonSetSettings
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPluginBuffer: %p"), lpPluginBuffer);
			printf("[%s(%d)] lpPluginBuffer:\n%s\n", __FILE__, __LINE__, lpPluginBuffer);

			DBGPRINT(DEBUG_VERBOSE, TEXT("Parsing the new settings..."));
			lpJsonNewSettings = cJSON_Parse(lpPluginBuffer);
			if (NULL == lpJsonNewSettings)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonNewSettings = cJSON_Parse(%p) failed."),
					lpPluginBuffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonNewSettings: %p"), lpJsonNewSettings);

			DBGPRINT(DEBUG_VERBOSE, TEXT("Replacing the old settings..."));
			cJSON_Delete(g_Agent_lpJsonConfigurationSettings);
			g_Agent_lpJsonConfigurationSettings = lpJsonNewSettings;
			lpJsonNewSettings = NULL;
			DBGPRINT(DEBUG_VERBOSE, TEXT("g_Agent_lpJsonConfigurationSettings: %p"), g_Agent_lpJsonConfigurationSettings);


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
			cJSON_AddBoolToObject(lpJsonOutput, Agent_SetSettings_Response_Result, cJSON_True);


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
			//if (NULL != lpPluginBuffer)
			//{
			//	cJSON_free(lpPluginBuffer);
			//	lpPluginBuffer = NULL;
			//}

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





DWORD WINAPI AgentLoadPlugin(
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
	
	cJSON*	lpJsonLoadPluginBuffer = NULL;
	LPBYTE	lpPluginBuffer = NULL;
	DWORD	dwPluginBufferSize = NULL;

	cJSON*	lpJsonLoadPluginName = NULL;
	CHAR*	szPluginName = NULL;

	cJSON*	lpJsonLoadPluginConfig = NULL;
	CHAR*	szPluginConfig = NULL;
	cJSON*	lpJsonPluginConfig = NULL;
	cJSON*	lpJsonPluginConfigEntry = NULL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;

	cJSON*	lpJsonLoadPluginResponsePluginHandle = NULL;

	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;

	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;

	cJSON*	lpJsonLoadLibraryRequest = NULL;
	cJSON*	lpJsonLoadLibraryRequestLibraryBuffer = NULL;
	cJSON*	lpJsonLoadLibraryResponseModuleHandle = NULL;

	HANDLE	hPluginMemoryModule = NULL;

	cJSON*	lpJsonGetProcAddrRequest = NULL;
	cJSON*	lpJsonGetProcAddrRequestFunctionName = NULL;
	cJSON*	lpJsonGetProcAddrRequestMemoryModule = NULL;
	cJSON*	lpJsonGetProcAddrResponseFunctionPointer = NULL;

	CHAR	szFunctionName[MAX_PATH];
	
	f_CallbackInterface		m_Initialization = NULL;

	
	cJSON*	lpJsonInitializationConfig = NULL;

	CHAR*	szInitializationInput = NULL;
	DWORD	dwInitializationInputSize = 0;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;


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
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonInput: %p"), lpJsonInput);

			// Get the Agent_LoadPlugin_Request_Plugin_Name
			lpJsonLoadPluginName = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_LoadPlugin_Request_Plugin_Name);
			if (NULL == lpJsonLoadPluginName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, Agent_LoadPlugin_Request_Plugin_Name
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadPluginName: %p"), lpJsonLoadPluginName);

			szPluginName = cJSON_GetStringValue(lpJsonLoadPluginName);
			if (NULL == szPluginName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpPluginName = cJSON_GetStringValue(%p) failed."),
					lpJsonLoadPluginName
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginName: %p"), szPluginName);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginName: %S"), szPluginName);
			//printf("[%s(%d)] szPluginName:\n%s\n", __FILE__, __LINE__, szPluginName);

			// Get the Agent_LoadPlugin_Request_Plugin_Buffer
			lpJsonLoadPluginBuffer = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_LoadPlugin_Request_Plugin_Buffer);
			if (NULL == lpJsonLoadPluginBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, Agent_LoadPlugin_Request_Plugin_Buffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadPluginBuffer: %p"), lpJsonLoadPluginBuffer);

			lpPluginBuffer = cJSON_GetBytesValue(lpJsonLoadPluginBuffer, (INT*)(&dwPluginBufferSize));
			if (NULL == lpPluginBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpPluginBuffer = cJSON_GetBytesValue(%p,%p) failed."),
					lpJsonLoadPluginBuffer, (INT*)(&dwPluginBufferSize)
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwPluginBufferSize: %d"), dwPluginBufferSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpPluginBuffer:     %p"), lpPluginBuffer);
			//printf("[%s(%d)] lpPluginBuffer:\n%s\n", __FILE__, __LINE__, lpPluginBuffer);

			// Get the Agent_LoadPlugin_Request_Plugin_Config
			lpJsonLoadPluginConfig = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_LoadPlugin_Request_Plugin_Config);
			if (NULL == lpJsonLoadPluginConfig)
			{
				DBGPRINT(
					DEBUG_INFO,
					TEXT("Plugin does not have a config.")
				);
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadPluginConfig: %p"), lpJsonLoadPluginConfig);

				szPluginConfig = cJSON_GetStringValue(lpJsonLoadPluginConfig);
				if (NULL == szPluginConfig)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpPluginConfig = cJSON_GetStringValue(%p) failed."),
						lpJsonLoadPluginConfig
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginConfig: %p"), szPluginConfig);
				DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginConfig: %S"), szPluginConfig);
				//printf("[%s(%d)] szPluginConfig:\n%s\n", __FILE__, __LINE__, szPluginConfig);

				lpJsonPluginConfig = cJSON_Parse(szPluginConfig);
				if (NULL == lpJsonPluginConfig)
				{
					dwErrorCode = ERROR_BAD_ARGUMENTS;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_Parse(%p) failed."),
						szPluginConfig
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonPluginConfig: %p"), lpJsonPluginConfig);

				if (NULL != szPluginConfig)
				{
					cJSON_free(szPluginConfig);
					szPluginConfig = NULL;
				}
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
			
			/***************************************************************************
				Use the LoaderMemoryModuleLoadLibrary
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleLoadLibrary"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_LoadLibrary_Request JSON object
			lpJsonLoadLibraryRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_LoadLibrary_Request);
			if (NULL == lpJsonLoadLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadLibraryRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_LoadLibrary_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadLibraryRequest:    %p"), lpJsonLoadLibraryRequest);

			// Create the LoaderMemoryModule_LoadLibrary_LibraryBuffer input JSON object
			lpJsonLoadLibraryRequestLibraryBuffer = cJSON_CreateBytes(lpPluginBuffer, dwPluginBufferSize);
			if (NULL == lpJsonLoadLibraryRequestLibraryBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadLibraryRequestLibraryBuffer = cJSON_CreateBytes(%p,%d) failed."),
					lpPluginBuffer,	dwPluginBufferSize
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonLoadLibraryRequest, LoaderMemoryModule_LoadLibrary_LibraryBuffer, lpJsonLoadLibraryRequestLibraryBuffer);

			// Create the string representation of the LoaderMemoryModule_LoadLibrary_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleLoadLibrary passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleLoadLibrary(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleLoadLibrary(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleLoadLibrary(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoaderMemoryModuleOutput: %p"), lpJsonLoaderMemoryModuleOutput);

			// Get and display the LoaderMemoryModule_LoadLibrary_ModuleHandle
			lpJsonLoadLibraryResponseModuleHandle = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_ModuleHandle);
			if (NULL == lpJsonLoadLibraryResponseModuleHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadLibraryResponseModuleHandle = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_LoadLibrary_ModuleHandle
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoadLibraryResponseModuleHandle: %p"), lpJsonLoadLibraryResponseModuleHandle);

			if (
				(!cJSON_IsBytes(lpJsonLoadLibraryResponseModuleHandle)) ||
				(NULL == lpJsonLoadLibraryResponseModuleHandle->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonLoadLibraryResponseModuleHandle
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonLoadLibraryResponseModuleHandle, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonLoadLibraryResponseModuleHandle, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_LoadLibrary_ModuleHandle, *((HMODULE*)pBuffer));
			hPluginMemoryModule = *((HMODULE*)pBuffer);

			lpJsonLoadPluginResponsePluginHandle = cJSON_Duplicate(lpJsonLoadLibraryResponseModuleHandle, cJSON_True);
			if (NULL == lpJsonLoadPluginResponsePluginHandle)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoadPluginResponsePluginHandle = cJSON_Duplicate(%p, cJSON_True) failed."),
						lpJsonLoadLibraryResponseModuleHandle
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonOutput, Agent_LoadPlugin_Response_Plugin_Handle, lpJsonLoadPluginResponsePluginHandle);


			// free allocated buffer
			if (NULL == pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}



			/***************************************************************************
				Use the LoaderMemoryModuleGetProc to get the Initialization Function
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleGetProc to get the Initialization Function"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_GetProcAddr_Request JSON object
			lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request);
			if (NULL == lpJsonGetProcAddrRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonGetProcAddrRequest:    %p"), lpJsonGetProcAddrRequest);

			// Create the LoaderMemoryModule_GetProcAddr_FunctionName input JSON object
			lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(SZ_DEFAULT_Initialization_FUNC_NAME);
			if (NULL == lpJsonGetProcAddrRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(%S) failed."),
					szFunctionName
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName, lpJsonGetProcAddrRequestFunctionName);

			// Create the LoaderMemoryModule_GetProcAddr_ModuleHandle input JSON object
			lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hPluginMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonGetProcAddrRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hPluginMemoryModule),
					sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_ModuleHandle, lpJsonGetProcAddrRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_GetProcAddr_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleGetProc passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleGetProcAddr(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			// Get and display the LoaderMemoryModule_GetProcAddr_FunctionPointer
			lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer);
			if (NULL == lpJsonGetProcAddrResponseFunctionPointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonGetProcAddrResponseFunctionPointer)) ||
				(NULL == lpJsonGetProcAddrResponseFunctionPointer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_GetProcAddr_FunctionPointer, *((HMODULE*)pBuffer));
			m_Initialization = *((f_CallbackInterface*)pBuffer);

			// free allocated buffer
			if (NULL == pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}

			DBGPRINT(DEBUG_INFO, TEXT("m_Initialization:        %p"), m_Initialization);


			/***************************************************************************
				Use the m_Initialization
			***************************************************************************/
			lpJsonInitializationConfig = cJSON_Duplicate(g_Agent_lpJsonInitializationConfig, cJSON_True);
			if (NULL == lpJsonInitializationConfig)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonInitializationConfig = cJSON_Duplicate(%p) failed."),
					g_Agent_lpJsonInitializationConfig
				);
				__leave;
			}
			
			if (NULL != lpJsonPluginConfig)
			{
				cJSON_ArrayForEach(lpJsonPluginConfigEntry, lpJsonPluginConfig)
				{
					cJSON_AddItemToObject(lpJsonInitializationConfig, lpJsonPluginConfigEntry->string, lpJsonPluginConfigEntry);
				}
			}
			
			szInitializationInput = cJSON_Print(lpJsonInitializationConfig);
			if (NULL == szInitializationInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szInitializationInput = cJSON_Print(%p) failed."),
					lpJsonInitializationConfig
				);
				__leave;
			}
			dwInitializationInputSize = (DWORD)strlen(szInitializationInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwInitializationInputSize: %d"), dwInitializationInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szInitializationInput:     %p"), szInitializationInput);
			//printf("[%s(%d)] szInitializationInput:\n%s\n", __FILE__, __LINE__, szInitializationInput);

			DBGPRINT(DEBUG_INFO, TEXT("Initialize plugin: %S"), szPluginName);
			dwErrorCode = m_Initialization(
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_Initialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szInitializationInput,
					dwInitializationInputSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_Initialization(%p, %d, %p, %p) was successful"),
				(LPBYTE)szInitializationInput,
				dwInitializationInputSize,
				NULL,
				NULL
			);


			// Add the plugin to our list of plugins
			if (!PluginInsert(szPluginName, hPluginMemoryModule))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("PluginInsert(%S, %p) failed."),
					szPluginName, hPluginMemoryModule
				);
				__leave;
			}

			
			

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
			//if (NULL != lpPluginBuffer)
			//{
			//	cJSON_free(lpPluginBuffer);
			//	lpPluginBuffer = NULL;
			//}

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

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}
			
			if (NULL != szInitializationInput)
			{
				cJSON_free(szInitializationInput);
				szInitializationInput = NULL;
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




DWORD WINAPI AgentUnloadPlugin(
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
	cJSON*	lpJsonUnloadPluginName = NULL;
	CHAR*	szPluginName = NULL;
	cJSON*	lpJsonUnloadPluginHandle = NULL;

	HANDLE	hPluginMemoryModule = NULL;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;

	cJSON*	lpJsonLoaderMemoryModuleInput = NULL;
	CHAR*	szLoaderMemoryModuleInput = NULL;
	DWORD	dwLoaderMemoryModuleInputSize = 0;

	cJSON*	lpJsonLoaderMemoryModuleOutput = NULL;
	CHAR*	szLoaderMemoryModuleOutput = NULL;
	DWORD	dwLoaderMemoryModuleOutputSize = 0;

	cJSON*	lpJsonFreeLibraryRequest = NULL;
	cJSON*	lpJsonFreeLibraryRequestMemoryModule = NULL;
	cJSON*	lpJsonFreeLibraryResponseResults = NULL;

	cJSON*	lpJsonGetProcAddrRequest = NULL;
	cJSON*	lpJsonGetProcAddrRequestFunctionName = NULL;
	cJSON*	lpJsonGetProcAddrRequestMemoryModule = NULL;
	cJSON*	lpJsonGetProcAddrResponseFunctionPointer = NULL;

	CHAR	szFunctionName[MAX_PATH];

	f_CallbackInterface	m_Finalization = NULL;

	CHAR*	szFinalizationInput = NULL;
	DWORD	dwFinalizationInputSize = 0;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;


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
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonInput: %p"), lpJsonInput);

			// Get the Agent_UnloadPlugin_Request_Plugin_Name
			lpJsonUnloadPluginName = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_UnloadPlugin_Request_Plugin_Name);
			if (NULL == lpJsonUnloadPluginName)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("%S is NOT specified."),
					Agent_UnloadPlugin_Request_Plugin_Name
				);
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonUnloadPluginName: %p"), lpJsonUnloadPluginName);

				szPluginName = cJSON_GetStringValue(lpJsonUnloadPluginName);
				if (NULL == szPluginName)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpPluginName = cJSON_GetStringValue(%p) failed."),
						lpJsonUnloadPluginName
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginName: %p"), szPluginName);
				DBGPRINT(DEBUG_VERBOSE, TEXT("szPluginName: %S"), szPluginName);
				//printf("[%s(%d)] szPluginName:\n%s\n", __FILE__, __LINE__, szPluginName);
			}

			// Get the Agent_UnloadPlugin_Request_Plugin_Handle
			lpJsonUnloadPluginHandle = cJSON_GetObjectItemCaseSensitive(lpJsonInput, Agent_UnloadPlugin_Request_Plugin_Handle);
			if (NULL == lpJsonUnloadPluginHandle)
			{
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("%S is NOT specified."),
					Agent_UnloadPlugin_Request_Plugin_Handle
				);
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonUnloadPluginName: %p"), lpJsonUnloadPluginName);

				pBuffer = cJSON_GetBytesValue(lpJsonUnloadPluginName, (INT*)(&nBufferSize));
				if (NULL == szPluginName)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("pBuffer = cJSON_GetBytesValue(%p,%p) failed."),
						lpJsonUnloadPluginName, (INT*)(&nBufferSize)
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("nBufferSize: %d"), nBufferSize);
				DBGPRINT(DEBUG_VERBOSE, TEXT("pBuffer:     %p"), pBuffer);
				//printf("[%s(%d)] szPluginName:\n%s\n", __FILE__, __LINE__, szPluginName);
				hPluginMemoryModule = *((HANDLE*)(pBuffer));
				DBGPRINT(DEBUG_VERBOSE, TEXT("hPluginMemoryModule:     %p"), hPluginMemoryModule);

				if (NULL != pBuffer)
				{
					cJSON_free(pBuffer);
					pBuffer = NULL;
				}
			}

			if (
				(NULL == hPluginMemoryModule)
				&&
				(NULL == szPluginName)
				)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("Must specifiy %S or %S."),
					Agent_UnloadPlugin_Request_Plugin_Handle,
					Agent_UnloadPlugin_Request_Plugin_Name
				);
				__leave;
			}

			if (NULL == hPluginMemoryModule)
			{
				hPluginMemoryModule = PluginFindHandle(szPluginName);
				if (NULL == hPluginMemoryModule)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("hPluginMemoryModule = PluginFindHandle(%S) failed."),
						szPluginName
					);
					__leave;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("hPluginMemoryModule:     %p"), hPluginMemoryModule);
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


			/***************************************************************************
				Fremove the plugin to our list of plugins
			***************************************************************************/
			if (!PluginRemove(szPluginName, hPluginMemoryModule))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("PluginRemove(%S, %p) failed."),
					szPluginName, hPluginMemoryModule
				);
				__leave;
			}


			/***************************************************************************
				Use the LoaderMemoryModuleGetProc to get the Finalization Function
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleGetProc to get the Finalization Function"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_GetProcAddr_Request JSON object
			lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request);
			if (NULL == lpJsonGetProcAddrRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_GetProcAddr_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonGetProcAddrRequest:    %p"), lpJsonGetProcAddrRequest);

			// Create the LoaderMemoryModule_GetProcAddr_FunctionName input JSON object
			lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(SZ_DEFAULT_Finalization_FUNC_NAME);
			if (NULL == lpJsonGetProcAddrRequestFunctionName)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestFunctionName = cJSON_CreateString(%S) failed."),
					szFunctionName
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_FunctionName, lpJsonGetProcAddrRequestFunctionName);

			// Create the LoaderMemoryModule_GetProcAddr_ModuleHandle input JSON object
			lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hPluginMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonGetProcAddrRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonGetProcAddrRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hPluginMemoryModule),
					sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonGetProcAddrRequest, LoaderMemoryModule_GetProcAddr_ModuleHandle, lpJsonGetProcAddrRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_GetProcAddr_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleGetProc passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleGetProcAddr(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleGetProcAddr(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}

			// Get and display the LoaderMemoryModule_GetProcAddr_FunctionPointer
			lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer);
			if (NULL == lpJsonGetProcAddrResponseFunctionPointer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonGetProcAddrResponseFunctionPointer = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_GetProcAddr_FunctionPointer
				);
				__leave;
			}
			if (
				(!cJSON_IsBytes(lpJsonGetProcAddrResponseFunctionPointer)) ||
				(NULL == lpJsonGetProcAddrResponseFunctionPointer->valuestring)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBytes(%p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer
				);
				__leave;
			}
			pBuffer = cJSON_GetBytesValue(lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize);
			if (NULL == pBuffer)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetBytesValue(%p, %p) failed."),
					lpJsonGetProcAddrResponseFunctionPointer, &nBufferSize
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %p"), LoaderMemoryModule_GetProcAddr_FunctionPointer, *((HMODULE*)pBuffer));
			m_Finalization = *((f_CallbackInterface*)pBuffer);

			// free allocated buffer
			if (NULL == pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}

			DBGPRINT(DEBUG_INFO, TEXT("m_Finalization:        %p"), m_Finalization);


			/***************************************************************************
				Use the m_Finalization
			***************************************************************************/
			szFinalizationInput = cJSON_Print(g_Agent_lpJsonInitializationConfig);
			if (NULL == szFinalizationInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szFinalizationInput = cJSON_Print(%p) failed."),
					g_Agent_lpJsonInitializationConfig
				);
				__leave;
			}
			dwFinalizationInputSize = (DWORD)strlen(szFinalizationInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwFinalizationInputSize: %d"), dwFinalizationInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szFinalizationInput:     %p"), szFinalizationInput);
			//printf("[%s(%d)] szFinalizationInput:\n%s\n", __FILE__, __LINE__, szFinalizationInput);

			DBGPRINT(DEBUG_INFO, TEXT("Finalize plugin: %s"), szPluginName);
			dwErrorCode = m_Finalization(
				(LPBYTE)szFinalizationInput,
				dwFinalizationInputSize,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("m_Finalization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szFinalizationInput,
					dwFinalizationInputSize,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("m_Finalization(%p, %d, %p, %p) was successful"),
				(LPBYTE)szFinalizationInput,
				dwFinalizationInputSize,
				NULL,
				NULL
			);


			/***************************************************************************
				Use the LoaderMemoryModuleFreeLibrary
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Use the LoaderMemoryModuleFreeLibrary"));

			// Create the input JSON object for the callback
			lpJsonLoaderMemoryModuleInput = cJSON_CreateObject();
			if (NULL == lpJsonLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleInput = cJSON_CreateObject failed.\n")
				);
				__leave;
			}

			// Create the LoaderMemoryModule_FreeLibrary_Request JSON object
			lpJsonFreeLibraryRequest = cJSON_AddObjectToObject(lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_FreeLibrary_Request);
			if (NULL == lpJsonFreeLibraryRequest)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFreeLibraryRequest = cJSON_AddObjectToObject(%p, %S) failed.\n"),
					lpJsonLoaderMemoryModuleInput, LoaderMemoryModule_FreeLibrary_Request
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonFreeLibraryRequest:    %p"), lpJsonFreeLibraryRequest);

			// Create the LoaderMemoryModule_FreeLibrary_ModuleHandle input JSON object
			lpJsonFreeLibraryRequestMemoryModule = cJSON_CreateBytes((LPBYTE)(&hPluginMemoryModule), sizeof(HMODULE));
			if (NULL == lpJsonFreeLibraryRequestMemoryModule)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonFreeLibraryRequestMemoryModule = cJSON_CreateBytes(%p,%d) failed."),
					(LPBYTE)(&hPluginMemoryModule), sizeof(HMODULE)
				);
				__leave;
			}
			cJSON_AddItemToObject(lpJsonFreeLibraryRequest, LoaderMemoryModule_FreeLibrary_ModuleHandle, lpJsonFreeLibraryRequestMemoryModule);

			// Create the string representation of the LoaderMemoryModule_FreeLibrary_Request
			szLoaderMemoryModuleInput = cJSON_Print(lpJsonLoaderMemoryModuleInput);
			if (NULL == szLoaderMemoryModuleInput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("szLoaderMemoryModuleInput = cJSON_Print(%p) failed."),
					lpJsonLoaderMemoryModuleInput
				);
				__leave;
			}
			dwLoaderMemoryModuleInputSize = (DWORD)strlen(szLoaderMemoryModuleInput);
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleInputSize: %d"), dwLoaderMemoryModuleInputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleInput:     %p"), szLoaderMemoryModuleInput);
			//printf("[%s(%d)] szLoaderMemoryModuleInput: \n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleInput);

			// Call the LoaderMemoryModuleFreeLibrary passing in the JSON input object and receiving the JSON output object
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("LoaderMemoryModuleFreeLibrary(%p, %d, %p, %p)"),
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			dwErrorCode = LoaderMemoryModuleFreeLibrary(
				(LPBYTE)szLoaderMemoryModuleInput,
				dwLoaderMemoryModuleInputSize,
				(LPBYTE*)&szLoaderMemoryModuleOutput,
				&dwLoaderMemoryModuleOutputSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("LoaderMemoryModuleFreeLibrary(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szLoaderMemoryModuleInput,
					dwLoaderMemoryModuleInputSize,
					(LPBYTE*)&szLoaderMemoryModuleOutput,
					&dwLoaderMemoryModuleOutputSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwLoaderMemoryModuleOutputSize: %d"), dwLoaderMemoryModuleOutputSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szLoaderMemoryModuleOutput:     %p"), szLoaderMemoryModuleOutput);
			//printf("[%s(%d)] szLoaderMemoryModuleOutput:\n%s\n", __FILE__, __LINE__, szLoaderMemoryModuleOutput);

			// Parse the output string into a JSON object
			lpJsonLoaderMemoryModuleOutput = cJSON_Parse(szLoaderMemoryModuleOutput);
			if (NULL == lpJsonLoaderMemoryModuleOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonLoaderMemoryModuleOutput = cJSON_Parse(%p) failed."),
					szLoaderMemoryModuleOutput
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonLoaderMemoryModuleOutput: %p"), lpJsonLoaderMemoryModuleOutput);

			// Get and display the LoaderMemoryModule_FreeLibrary_Results
			lpJsonFreeLibraryResponseResults = cJSON_GetObjectItemCaseSensitive(lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Results);
			if (NULL == lpJsonFreeLibraryResponseResults)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("lpJsonFreeLibraryResponseResults = cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonLoaderMemoryModuleOutput, LoaderMemoryModule_FreeLibrary_Results
				);
				__leave;
			}
			if (!cJSON_IsBool(lpJsonFreeLibraryResponseResults))
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("cJSON_IsBool(%p) failed."),
					lpJsonFreeLibraryResponseResults
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("  %S: %d"), LoaderMemoryModule_FreeLibrary_Results, lpJsonFreeLibraryResponseResults->valueint);

			// free allocated buffer
			if (NULL == pBuffer)
			{
				cJSON_free(pBuffer);
				pBuffer = NULL;
			}

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}



			/***************************************************************************
				Update the output result
			***************************************************************************/
			cJSON_AddBoolToObject(lpJsonOutput, Agent_UnloadPlugin_Response_Result, cJSON_True);


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
			//if (NULL != szPluginName)
			//{
			//	cJSON_free(szPluginName);
			//	szPluginName = NULL;
			//}

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

			// Delete input and output JSON objects
			if (NULL != lpJsonLoaderMemoryModuleInput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleInput);
				lpJsonLoaderMemoryModuleInput = NULL;
			}
			if (NULL != lpJsonLoaderMemoryModuleOutput)
			{
				cJSON_Delete(lpJsonLoaderMemoryModuleOutput);
				lpJsonLoaderMemoryModuleOutput = NULL;
			}
			// Free input and output JSON strings
			if (NULL != szLoaderMemoryModuleInput)
			{
				cJSON_free(szLoaderMemoryModuleInput);
				szLoaderMemoryModuleInput = NULL;
			}
			if (NULL != szLoaderMemoryModuleOutput)
			{
				cJSON_free(szLoaderMemoryModuleOutput);
				szLoaderMemoryModuleOutput = NULL;
			}

			if (NULL != szFinalizationInput)
			{
				cJSON_free(szFinalizationInput);
				szFinalizationInput = NULL;
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





DWORD WINAPI AgentListPlugins(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szAgentInput = NULL;
	DWORD	dwAgentInputSize = 0;
	cJSON*	lpJsonAgentInput = NULL;

	//LPBYTE	lpMessageHandlerInput = NULL;
	//DWORD	dwMessageHandlerInputSize = 0;
	//LPBYTE	lpMessageHandlerOutput = NULL;
	//DWORD	dwMessageHandlerOutputSize = 0;

	CHAR*	szAgentOutput = NULL;
	DWORD	dwAgentOutputSize = 0;
	cJSON*	lpJsonAgentOutput = NULL;
	cJSON*	lpJsonAgentOutputHandlers = NULL;


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
			//szAgentInput = (char*)lpInputBuffer;
			//dwAgentInputSize = dwInputBufferSize;


			/********************************************************************************
				Convert input string into JSON
			********************************************************************************/
			//printf("[%s(%d)] szAgentInput:\n%s\n", __FILE__, __LINE__, szAgentInput);
			//lpJsonAgentInput = cJSON_Parse(szAgentInput);
			//if (NULL == lpJsonAgentInput)
			//{
			//	dwErrorCode = ERROR_BAD_ARGUMENTS;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Parse(%p) failed."),
			//		szAgentInput
			//	);
			//	__leave;
			//}


			/********************************************************************************
				Create the output JSON object
			********************************************************************************/
			// Create a response JSON object
			lpJsonAgentOutput = cJSON_CreateObject();
			if (NULL == lpJsonAgentOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentOutput = cJSON_CreateObject() failed.\n")
				);
				__leave;
			}

			lpJsonAgentOutputHandlers = cJSON_AddArrayToObject(lpJsonAgentOutput, Agent_ListPlugins_Response_Plugins);
			if (NULL == lpJsonAgentOutputHandlers)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonAgentOutputHandlers = cJSON_CreateArray() failed.\n")
				);
				__leave;
			}


			/********************************************************************************
				Perform action given input parameters
			********************************************************************************/
			for (PPLUGIN_NODE pCurrentPlugin = g_Agent_lpPluginList; pCurrentPlugin != NULL; pCurrentPlugin = pCurrentPlugin->lpNext)
			{
				cJSON* lpJsonCurrentPlugin = NULL;
				cJSON* lpJsonCurrentPluginName = NULL;
				cJSON* lpJsonCurrentPluginHandle = NULL;

				DBGPRINT(DEBUG_VERBOSE, TEXT("pCurrentPlugin->szPluginName:     %S"), pCurrentPlugin->szPluginName);

				// Create an object for the current plugin
				lpJsonCurrentPlugin = cJSON_CreateObject();
				if (NULL == lpJsonCurrentPlugin)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCurrentPlugin = cJSON_CreateObject() failed.\n")
					);
					__leave;
				}

				// Create an object for the current plugin name
				lpJsonCurrentPluginName = cJSON_CreateString(pCurrentPlugin->szPluginName);
				if (NULL == lpJsonCurrentPluginName)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCurrentPluginName = cJSON_CreateString(%S) failed.\n"),
						pCurrentPlugin->szPluginName
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCurrentPlugin, Agent_ListPlugins_Response_Plugin_Name, lpJsonCurrentPluginName);

				// Create an object for the current plugin handle
				lpJsonCurrentPluginHandle = cJSON_CreateBytes((PBYTE)(&(pCurrentPlugin->hPlugin)), sizeof(HANDLE));
				if (NULL == lpJsonCurrentPluginHandle)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("lpJsonCurrentPluginHandle = cJSON_CreateBytes(%p, %d) failed.\n"),
						(PBYTE)(&(pCurrentPlugin->hPlugin)), sizeof(HANDLE)
					);
					__leave;
				}
				cJSON_AddItemToObject(lpJsonCurrentPlugin, Agent_ListPlugins_Response_Plugin_Handle, lpJsonCurrentPluginHandle);

				// Add current plugin to the array of plugins
				cJSON_AddItemToArray(lpJsonAgentOutputHandlers, lpJsonCurrentPlugin);
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			szAgentOutput = cJSON_Print(lpJsonAgentOutput);
			if (NULL == szAgentOutput)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Print(%p) failed."),
					lpJsonAgentOutput
				);
				__leave;
			}
			dwAgentOutputSize = (DWORD)strlen(szAgentOutput);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("dwJsonOutputSize: %d"), dwJsonOutputSize);
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szJsonOutput:     %p"), szJsonOutput);
			//printf("[%s(%d)] szJsonOutput:\n%s\n", __FILE__, __LINE__, szJsonOutput);

			(*lppOutputBuffer) = (LPBYTE)szAgentOutput;
			(*lpdwOutputBufferSize) = dwAgentOutputSize;

		} // end try-finally
		__finally
		{
			if (NULL != lpJsonAgentInput)
			{
				cJSON_Delete(lpJsonAgentInput);
				lpJsonAgentInput = NULL;
			}

			if (NULL != lpJsonAgentOutput)
			{
				cJSON_Delete(lpJsonAgentOutput);
				lpJsonAgentOutput = NULL;
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

					cJSON_free(szAgentOutput);
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










