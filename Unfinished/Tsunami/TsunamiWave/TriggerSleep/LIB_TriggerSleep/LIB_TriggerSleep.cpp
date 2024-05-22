#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_TriggerSleep.h"
#include "Internal_Functions.h"


#define SEVEN_DAYS_IN_SECONDS 604800
#define MILLISECONDS_PER_SECOND 1000

// Global variables
cJSON* g_TriggerSleep_lpJsonConfig = NULL;


DWORD WINAPI TriggerSleepInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonTriggerSleepInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonTriggerSleepInitializeOutput = NULL;
	CHAR*	szTriggerSleepOutput = NULL;
	DWORD	dwTriggerSleepOutputSize = 0;

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
			lpJsonTriggerSleepInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonTriggerSleepInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonTriggerSleepInitializeInput:     %p"), lpJsonTriggerSleepInitializeInput);

			// Set the local module's configuration to the input configuration
			g_TriggerSleep_lpJsonConfig = lpJsonTriggerSleepInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonTriggerSleepInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_TriggerSleep_Kernel32_Functions()"));
			dwErrorCode = Initialize_TriggerSleep_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_TriggerSleep_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/
			

			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register TriggerSleep_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_TriggerSleep_lpJsonConfig,
				TriggerSleep_Request,
				TriggerSleepCallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_TriggerSleep_lpJsonConfig,
					TriggerSleep_Request,
					TriggerSleepCallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register TriggerSleep_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szTriggerSleepOutput = cJSON_Print(lpJsonTriggerSleepInitializeOutput);
			//if (NULL == szTriggerSleepOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonTriggerSleepInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szTriggerSleepOutput:     %p"), szTriggerSleepOutput);
			////printf("szTriggerSleepOutput:\n%s\n", szTriggerSleepOutput);
			//dwTriggerSleepOutputSize = (DWORD)strlen(szTriggerSleepOutput);

			//(*lppOutputBuffer) = (LPBYTE)szTriggerSleepOutput;
			//(*lpdwOutputBufferSize) = dwTriggerSleepOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonTriggerSleepInitializeInput)
			{
				cJSON_Delete(lpJsonTriggerSleepInitializeInput);
				lpJsonTriggerSleepInitializeInput = NULL;
			}
			if (NULL != lpJsonTriggerSleepInitializeOutput)
			{
				cJSON_Delete(lpJsonTriggerSleepInitializeOutput);
				lpJsonTriggerSleepInitializeOutput = NULL;
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


DWORD WINAPI TriggerSleepFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szTriggerSleepInput = NULL;
	DWORD	dwTriggerSleepInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szTriggerSleepOutput = NULL;
	DWORD	dwTriggerSleepOutputSize = 0;
	cJSON*	lpJsonTriggerSleepOutput = NULL;

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
			//szTriggerSleepInput = (char*)lpInputBuffer;
			//dwTriggerSleepInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TriggerSleep_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_TriggerSleep_lpJsonConfig,
				TriggerSleep_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_TriggerSleep_lpJsonConfig,
					TriggerSleep_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TriggerSleep_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_TriggerSleep_Kernel32_Functions()"));
			dwErrorCode = Finalize_TriggerSleep_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_TriggerSleep_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_TriggerSleep_lpJsonConfig)
			{
				cJSON_Delete(g_TriggerSleep_lpJsonConfig);
				g_TriggerSleep_lpJsonConfig = NULL;
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


DWORD WINAPI TriggerSleepCallbackInterface(
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
	cJSON*	lpJsonSleepSeconds = NULL;
	INT		nSleepSeconds = 0;
	cJSON*	lpJsonVariationSeconds = NULL;
	INT		nVariationSeconds = 0;
	DWORD	dwSleepSeconds = 0;

	CHAR*	szOutput = NULL;
	DWORD	dwOutputSize = 0;
	cJSON*	lpJsonOutput = NULL;
	cJSON*	lpJsonWakeTime = NULL;
	SYSTEMTIME	currentSystemTime;


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
			// Get the TriggerSleep_Request_Sleep_Seconds
			lpJsonSleepSeconds = cJSON_GetObjectItemCaseSensitive(lpJsonInput, TriggerSleep_Request_Sleep_Seconds);
			if (NULL == lpJsonSleepSeconds)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, TriggerSleep_Request_Sleep_Seconds
				);
				__leave;
			}
			// Check the TriggerSleep_Request_Sleep_Seconds JSON type
			if (
				!cJSON_IsNumber(lpJsonSleepSeconds)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonSleepSeconds
				);
				__leave;
			}
			// Get the TriggerSleep_Request_Sleep_Seconds value and check for sanity
			nSleepSeconds = lpJsonSleepSeconds->valueint;
			if (0 > nSleepSeconds)
			{
				DBGPRINT(DEBUG_WARNING, TEXT("nSleepSeconds: 0 > %d"), nSleepSeconds);
				nSleepSeconds = 0;
			}
			if (SEVEN_DAYS_IN_SECONDS < nSleepSeconds)
			{
				DBGPRINT(DEBUG_WARNING, TEXT("nSleepSeconds: %d < %d"), SEVEN_DAYS_IN_SECONDS, nSleepSeconds);
				nSleepSeconds = SEVEN_DAYS_IN_SECONDS;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("nSleepSeconds: %d"), nSleepSeconds);


			// Get the TriggerSleep_Request_Variation_Seconds
			lpJsonVariationSeconds = cJSON_GetObjectItemCaseSensitive(lpJsonInput, TriggerSleep_Request_Variation_Seconds);
			if (NULL == lpJsonVariationSeconds)
			{
				DBGPRINT(DEBUG_WARNING, TEXT("%S not defined"), TriggerSleep_Request_Variation_Seconds);
				nVariationSeconds = 0;
			}
			else
			{
				// Check the TriggerSleep_Request_Variation_Seconds JSON type
				if (
					!cJSON_IsNumber(lpJsonVariationSeconds)
					)
				{
					dwErrorCode = ERROR_OBJECT_NOT_FOUND;
					DBGPRINT(
						DEBUG_ERROR,
						TEXT("cJSON_IsNumber(%p) failed."),
						lpJsonVariationSeconds
					);
					__leave;
				}
				// Get the TriggerSleep_Request_Variation_Seconds value and check for sanity
				nVariationSeconds = lpJsonVariationSeconds->valueint;
				if (0 > nVariationSeconds)
				{
					DBGPRINT(DEBUG_WARNING, TEXT("nVariationSeconds: 0 > %d"), nVariationSeconds);
					nVariationSeconds = 0;
				}
				if (SEVEN_DAYS_IN_SECONDS < nVariationSeconds)
				{
					DBGPRINT(DEBUG_WARNING, TEXT("nVariationSeconds: %d < %d"), SEVEN_DAYS_IN_SECONDS, nVariationSeconds);
					nVariationSeconds = SEVEN_DAYS_IN_SECONDS;
				}
				DBGPRINT(DEBUG_VERBOSE, TEXT("nVariationSeconds: %d"), nVariationSeconds);

				if (nVariationSeconds > 0)
				{
					// Calculate the random variation
					DBGPRINT(DEBUG_VERBOSE, TEXT("srand()"));
					srand(GetTickCount());
					DBGPRINT(DEBUG_VERBOSE, TEXT("rand()"));
					if (rand() % 2)
						nVariationSeconds = (rand() % nVariationSeconds) * (-1);
					else
						nVariationSeconds = (rand() % nVariationSeconds);
				}
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("nVariationSeconds: %d"), nVariationSeconds);

			// Calculate actual sleep time
			dwSleepSeconds = (DWORD)( nSleepSeconds + nVariationSeconds );
			if (0 > dwSleepSeconds)
			{
				DBGPRINT(DEBUG_WARNING, TEXT("dwSleepSeconds: 0 > %d"), dwSleepSeconds);
				dwSleepSeconds = 0;
			}
			if (SEVEN_DAYS_IN_SECONDS < dwSleepSeconds)
			{
				DBGPRINT(DEBUG_WARNING, TEXT("dwSleepSeconds: %d < %d"), SEVEN_DAYS_IN_SECONDS, dwSleepSeconds);
				dwSleepSeconds = SEVEN_DAYS_IN_SECONDS;
			}

			// Acutally sleep
			DBGPRINT(DEBUG_INFO, TEXT("Sleeping %d seconds..."), dwSleepSeconds);
			m_TriggerSleep_Sleep(dwSleepSeconds*MILLISECONDS_PER_SECOND);


			// Create response varialbe
			memset(&currentSystemTime, 0, sizeof(SYSTEMTIME));
			m_TriggerSleep_GetSystemTime(&currentSystemTime);
			DBGPRINT(
				DEBUG_VERBOSE, 
				TEXT("currentSystemTime: %d-%d-%d %d:%d:%d"), 
				currentSystemTime.wYear,
				currentSystemTime.wMonth,
				currentSystemTime.wDay,
				currentSystemTime.wHour,
				currentSystemTime.wMinute,
				currentSystemTime.wSecond
				);
			lpJsonWakeTime = cJSON_CreateBytes((PBYTE)(&currentSystemTime), sizeof(SYSTEMTIME));
			if (NULL == lpJsonWakeTime)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("lpJsonWakeTime = cJSON_CreateBytes(%p, %d) failed.\n"),
					(PBYTE)(&currentSystemTime), sizeof(SYSTEMTIME)
				);
				__leave;
			}
			// Add the TriggerSleep_Response_ProcessId to the current entry
			cJSON_AddItemToObject(lpJsonOutput, TriggerSleep_Response_Wake_Time, lpJsonWakeTime);



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


