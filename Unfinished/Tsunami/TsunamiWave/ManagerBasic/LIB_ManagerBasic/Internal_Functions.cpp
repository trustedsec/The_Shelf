#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"


PMESSAGE_HANDLER_NODE g_ManagerBasic_lpMessageHandlerList = NULL;

//PMESSAGE_HANDLER_NODE MessageHandlerInsert(const DWORD dwNewModuleType, const DWORD dwNewMessageType, const f_CallbackInterface fpNewMessageHandler)
//{
//	PMESSAGE_HANDLER_NODE pNewMessageHandler = (PMESSAGE_HANDLER_NODE)malloc(sizeof(MESSAGE_HANDLER_NODE));
//	if (NULL != pNewMessageHandler)
//	{
//		memset(pNewMessageHandler, 0, sizeof(MESSAGE_HANDLER_NODE));
//		pNewMessageHandler->dwModuleType		= dwNewModuleType;
//		pNewMessageHandler->dwMessageType		= dwNewMessageType;
//		pNewMessageHandler->fpMessageHandler	= fpNewMessageHandler;
//		pNewMessageHandler->lpNext				= g_ManagerBasic_lpMessageHandlerList;
//		g_ManagerBasic_lpMessageHandlerList					= pNewMessageHandler;
//	}
//	return pNewMessageHandler;
//}

//BOOL MessageHandlerRemove(const DWORD dwRemoveModuleType, const DWORD dwRemoveMessageType)
//{
//	PMESSAGE_HANDLER_NODE pPreviousMessageHandler = NULL;
//	PMESSAGE_HANDLER_NODE pCurrentMessageHandler = NULL;
//	BOOL bFound = FALSE;
//
//	if (g_ManagerBasic_lpMessageHandlerList)
//	{
//		pPreviousMessageHandler = NULL;
//		pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
//		do
//		{
//			// Check if we want to remove this node
//			if (
//				(
//				(dwRemoveModuleType == pCurrentMessageHandler->dwModuleType) ||
//					(dwRemoveModuleType == TSM_ANY_FAMILY)
//					) &&
//					(
//				(dwRemoveMessageType == pCurrentMessageHandler->dwMessageType) ||
//						(dwRemoveMessageType == TSM_ANY_MESSAGE)
//						)
//				)
//			{
//				bFound = TRUE;
//
//				// Are we removing the list head? If so, pPreviousMessageHandler will be NULL
//				if (NULL == pPreviousMessageHandler)
//				{
//					g_ManagerBasic_lpMessageHandlerList = pCurrentMessageHandler->lpNext;
//					free(pCurrentMessageHandler);
//					pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
//				}
//				else
//				{
//					pPreviousMessageHandler->lpNext = pCurrentMessageHandler->lpNext;
//					free(pCurrentMessageHandler);
//					pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
//				}
//			}
//			else
//			{
//				pPreviousMessageHandler = pCurrentMessageHandler;
//				pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
//			}
//		} while (pCurrentMessageHandler);
//	}
//
//	return bFound;
//}

//f_CallbackInterface MessageHandlerFindFirst(const DWORD dwInputModuleType, const DWORD dwInputMessageType)
//{
//	PMESSAGE_HANDLER_NODE pPreviousMessageHandler = NULL;
//	PMESSAGE_HANDLER_NODE pCurrentMessageHandler = NULL;
//
//	if (g_ManagerBasic_lpMessageHandlerList)
//	{
//		pPreviousMessageHandler = NULL;
//		pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
//		do
//		{
//			// Find first handler that matches input
//			if (
//				(
//				(dwInputModuleType == pCurrentMessageHandler->dwModuleType) ||
//					(dwInputModuleType == TSM_ANY_FAMILY)
//					) &&
//					(
//				(dwInputMessageType == pCurrentMessageHandler->dwMessageType) ||
//						(dwInputMessageType == TSM_ANY_MESSAGE)
//						)
//				)
//			{
//				return pCurrentMessageHandler->fpMessageHandler;
//			}
//
//			pPreviousMessageHandler = pCurrentMessageHandler;
//			pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
//		} while (pCurrentMessageHandler);
//	}
//
//	return NULL;
//}

PMESSAGE_HANDLER_NODE MessageHandlerInsert(const CHAR* szNewMessageHandlerType, const f_CallbackInterface fpNewMessageHandler)
{
	PMESSAGE_HANDLER_NODE pNewMessageHandler = (PMESSAGE_HANDLER_NODE)malloc(sizeof(MESSAGE_HANDLER_NODE));
	if (NULL != pNewMessageHandler)
	{
		memset(pNewMessageHandler, 0, sizeof(MESSAGE_HANDLER_NODE));
		strcpy(pNewMessageHandler->szMessageHandlerType, szNewMessageHandlerType);
		pNewMessageHandler->fpMessageHandler	= fpNewMessageHandler;
		pNewMessageHandler->lpNext				= g_ManagerBasic_lpMessageHandlerList;
		g_ManagerBasic_lpMessageHandlerList					= pNewMessageHandler;
	}
	return pNewMessageHandler;
}

BOOL MessageHandlerRemove(const CHAR* szRemoveMessageHandlerType)
{
	PMESSAGE_HANDLER_NODE pPreviousMessageHandler = NULL;
	PMESSAGE_HANDLER_NODE pCurrentMessageHandler = NULL;
	BOOL bFound = FALSE;

	if (g_ManagerBasic_lpMessageHandlerList)
	{
		pPreviousMessageHandler = NULL;
		pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
		do
		{
			// Check if we want to remove this node
			if	( 
				( NULL == szRemoveMessageHandlerType ) ||
				( 0 == strcmp(szRemoveMessageHandlerType, pCurrentMessageHandler->szMessageHandlerType) ) 
				)
			{
				bFound = TRUE;

				// Are we removing the list head? If so, pPreviousMessageHandler will be NULL
				if (NULL == pPreviousMessageHandler)
				{
					g_ManagerBasic_lpMessageHandlerList = pCurrentMessageHandler->lpNext;
					free(pCurrentMessageHandler);
					pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
				}
				else
				{
					pPreviousMessageHandler->lpNext = pCurrentMessageHandler->lpNext;
					free(pCurrentMessageHandler);
					pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
				}
			}
			else
			{
				pPreviousMessageHandler = pCurrentMessageHandler;
				pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
			}
		} while (pCurrentMessageHandler);
	}

	return bFound;
}

f_CallbackInterface MessageHandlerFindFirst(const CHAR* szFindMessageHandlerType)
{
	PMESSAGE_HANDLER_NODE pPreviousMessageHandler = NULL;
	PMESSAGE_HANDLER_NODE pCurrentMessageHandler = NULL;

	if (g_ManagerBasic_lpMessageHandlerList)
	{
		pPreviousMessageHandler = NULL;
		pCurrentMessageHandler = g_ManagerBasic_lpMessageHandlerList;
		do
		{
			// Find first handler that matches input
			if ( 0 == strcmp(szFindMessageHandlerType, pCurrentMessageHandler->szMessageHandlerType)	)
			{
				return pCurrentMessageHandler->fpMessageHandler;
			}
			
			pPreviousMessageHandler = pCurrentMessageHandler;
			pCurrentMessageHandler = pPreviousMessageHandler->lpNext;
		} while (pCurrentMessageHandler);
	}

	return NULL;
}