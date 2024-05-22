#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"
#include "cJSON.h"

#include "LIB_TEMPLATE_MODULE.h"
#include "Internal_Functions.h"


#define SEVEN_DAYS_IN_MILLISECONDS 604800000

// Global variables
cJSON* g_TEMPLATE_MODULE_lpJsonConfig = NULL;


DWORD WINAPI TEMPLATE_MODULEInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	cJSON*	lpJsonTEMPLATE_MODULEInitializeInput = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	cJSON*	lpJsonTEMPLATE_MODULEInitializeOutput = NULL;
	CHAR*	szTEMPLATE_MODULEOutput = NULL;
	DWORD	dwTEMPLATE_MODULEOutputSize = 0;

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
			lpJsonTEMPLATE_MODULEInitializeInput = cJSON_Parse((CHAR*)lpInputBuffer);
			if (NULL == lpJsonTEMPLATE_MODULEInitializeInput)
			{
				dwErrorCode = ERROR_BAD_ARGUMENTS;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_Parse(%p) failed."),
					(CHAR*)lpInputBuffer
				);
				__leave;
			}
			DBGPRINT(DEBUG_VERBOSE, TEXT("lpJsonTEMPLATE_MODULEInitializeInput:     %p"), lpJsonTEMPLATE_MODULEInitializeInput);

			// Set the local module's configuration to the input configuration
			g_TEMPLATE_MODULE_lpJsonConfig = lpJsonTEMPLATE_MODULEInitializeInput;
			// and then null the input configuration so it isn't free'd in the finally
			lpJsonTEMPLATE_MODULEInitializeInput = NULL;


			/********************************************************************************
				Initialize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Initialize_TEMPLATE_MODULE_Kernel32_Functions()"));
			dwErrorCode = Initialize_TEMPLATE_MODULE_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Initialize_TEMPLATE_MODULE_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}


			/********************************************************************************
				Perform additional initialization actions
			********************************************************************************/
			

			/********************************************************************************
				Register callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Register TEMPLATE_MODULE_Request"));
			bRegisterResult = RegisterModuleMessageHandler(
				g_TEMPLATE_MODULE_lpJsonConfig,
				TEMPLATE_MODULE_Request,
				TEMPLATE_MODULECallbackInterface
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("RegisterModuleMessageHandler(%p, %S, %p) failed."),
					g_TEMPLATE_MODULE_lpJsonConfig,
					TEMPLATE_MODULE_Request,
					TEMPLATE_MODULECallbackInterface
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Register TEMPLATE_MODULE_Request was successful."));
			}


			/********************************************************************************
				Create output message based on results
			********************************************************************************/
			//DBGPRINT(DEBUG_VERBOSE, TEXT("Create output message based on results"));
			//szTEMPLATE_MODULEOutput = cJSON_Print(lpJsonTEMPLATE_MODULEInitializeOutput);
			//if (NULL == szTEMPLATE_MODULEOutput)
			//{
			//	dwErrorCode = ERROR_OBJECT_NOT_FOUND;
			//	DBGPRINT(
			//		DEBUG_ERROR,
			//		TEXT("cJSON_Print(%p) failed."),
			//		lpJsonTEMPLATE_MODULEInitializeOutput
			//	);
			//	__leave;
			//}
			//DBGPRINT(DEBUG_VERBOSE, TEXT("szTEMPLATE_MODULEOutput:     %p"), szTEMPLATE_MODULEOutput);
			////printf("szTEMPLATE_MODULEOutput:\n%s\n", szTEMPLATE_MODULEOutput);
			//dwTEMPLATE_MODULEOutputSize = (DWORD)strlen(szTEMPLATE_MODULEOutput);

			//(*lppOutputBuffer) = (LPBYTE)szTEMPLATE_MODULEOutput;
			//(*lpdwOutputBufferSize) = dwTEMPLATE_MODULEOutputSize;

		} // end try-finally
		__finally
		{
			// Delete input and output JSON objects
			if (NULL != lpJsonTEMPLATE_MODULEInitializeInput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEInitializeInput);
				lpJsonTEMPLATE_MODULEInitializeInput = NULL;
			}
			if (NULL != lpJsonTEMPLATE_MODULEInitializeOutput)
			{
				cJSON_Delete(lpJsonTEMPLATE_MODULEInitializeOutput);
				lpJsonTEMPLATE_MODULEInitializeOutput = NULL;
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


DWORD WINAPI TEMPLATE_MODULEFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
)
{
	DWORD	dwErrorCode = ERROR_SUCCESS;
	DWORD	dwTempErrorCode = ERROR_SUCCESS;

	CHAR*	szTEMPLATE_MODULEInput = NULL;
	DWORD	dwTEMPLATE_MODULEInputSize = 0;
	cJSON*	lpJsonInput = NULL;
	cJSON*	lpJsonMessageType = NULL;

	BOOL	bRegisterResult = FALSE;

	PBYTE	pBuffer = NULL;
	int		nBufferSize = 0;

	CHAR*	szTEMPLATE_MODULEOutput = NULL;
	DWORD	dwTEMPLATE_MODULEOutputSize = 0;
	cJSON*	lpJsonTEMPLATE_MODULEOutput = NULL;

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
			//szTEMPLATE_MODULEInput = (char*)lpInputBuffer;
			//dwTEMPLATE_MODULEInputSize = dwInputBufferSize;


			/********************************************************************************
				Unregister callback functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TEMPLATE_MODULE_Request"));
			bRegisterResult = UnregisterModuleMessageHandler(
				g_TEMPLATE_MODULE_lpJsonConfig,
				TEMPLATE_MODULE_Request
			);
			if (FALSE == bRegisterResult)
			{
				//dwErrorCode = ERROR_BAD_PROVIDER;
				DBGPRINT(
					DEBUG_WARNING,
					TEXT("UnregisterModuleMessageHandler(%p, %S) failed."),
					g_TEMPLATE_MODULE_lpJsonConfig,
					TEMPLATE_MODULE_Request
				);
				//__leave;
			}
			else
			{
				DBGPRINT(DEBUG_VERBOSE, TEXT("Unregister TEMPLATE_MODULE_Request was successful."));
			}


			/********************************************************************************
				Perform additional Finalization actions
			********************************************************************************/


			/********************************************************************************
				Finalize internal functions
			********************************************************************************/
			DBGPRINT(DEBUG_VERBOSE, TEXT("Finalize_TEMPLATE_MODULE_Kernel32_Functions()"));
			dwErrorCode = Finalize_TEMPLATE_MODULE_Kernel32_Functions();
			if (ERROR_SUCCESS != dwErrorCode)
			{
				DBGPRINT(DEBUG_ERROR, TEXT("Finalize_TEMPLATE_MODULE_Kernel32_Functions() failed.(%08x)"), dwErrorCode);
				__leave;
			}

		} // end try-finally
		__finally
		{
			/********************************************************************************
				Delete local JSON config
			********************************************************************************/
			if (NULL != g_TEMPLATE_MODULE_lpJsonConfig)
			{
				cJSON_Delete(g_TEMPLATE_MODULE_lpJsonConfig);
				g_TEMPLATE_MODULE_lpJsonConfig = NULL;
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


DWORD WINAPI TEMPLATE_MODULECallbackInterface(
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
	cJSON*	lpJsonSleepMilliseconds = NULL;
	INT		nSleepMilliseconds = 0;

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
			// Get the TEMPLATE_MODULE_Request_Sleep_Milliseconds
			lpJsonSleepMilliseconds = cJSON_GetObjectItemCaseSensitive(lpJsonInput, TEMPLATE_MODULE_Request_Sleep_Milliseconds);
			if (NULL == lpJsonSleepMilliseconds)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_GetObjectItemCaseSensitive(%p, %S) failed."),
					lpJsonInput, TEMPLATE_MODULE_Request_Sleep_Milliseconds
				);
				__leave;
			}
				
			
			// Check the TEMPLATE_MODULE_Request_Sleep_Milliseconds JSON type
			if (
				!cJSON_IsNumber(lpJsonSleepMilliseconds)
				)
			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("cJSON_IsNumber(%p) failed."),
					lpJsonSleepMilliseconds
				);
				__leave;
			}
			// Get the TEMPLATE_MODULE_Request_Sleep_Milliseconds value and check for sanity
			nSleepMilliseconds = lpJsonSleepMilliseconds->valueint;
			if ( (0 > nSleepMilliseconds) || (SEVEN_DAYS_IN_MILLISECONDS  < nSleepMilliseconds) )

			{
				dwErrorCode = ERROR_OBJECT_NOT_FOUND;
				DBGPRINT(
					DEBUG_ERROR,
					TEXT("%S is invalid: %d"),
					TEMPLATE_MODULE_Request_Sleep_Milliseconds, nSleepMilliseconds
				);
				__leave;
			}
			DBGPRINT(DEBUG_INFO, TEXT("Sleeping %d milliseonds..."), nSleepMilliseconds);
			m_TEMPLATE_MODULE_Sleep((DWORD)nSleepMilliseconds);

			// Create response varialbe
			memset(&currentSystemTime, 0, sizeof(SYSTEMTIME));
			m_TEMPLATE_MODULE_GetSystemTime(&currentSystemTime);
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
			// Add the TEMPLATE_MODULE_Response_ProcessId to the current entry
			cJSON_AddItemToObject(lpJsonOutput, TEMPLATE_MODULE_Response_Wake_Time, lpJsonWakeTime);



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


