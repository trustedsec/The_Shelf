#pragma once
#ifndef __LIB_Internal_ManagerBasic_Functions__
#define __LIB_Internal_ManagerBasic_Functions__

#include <Windows.h>

#define MAX_HANDLER_TYPE 128

typedef struct _MESSAGE_HANDLER_NODE {
	CHAR szMessageHandlerType[MAX_HANDLER_TYPE];
	f_CallbackInterface fpMessageHandler;
	_MESSAGE_HANDLER_NODE* lpNext;
} MESSAGE_HANDLER_NODE, *PMESSAGE_HANDLER_NODE;

extern PMESSAGE_HANDLER_NODE g_ManagerBasic_lpMessageHandlerList;

PMESSAGE_HANDLER_NODE MessageHandlerInsert(const CHAR* szNewMessageHandlerType, const f_CallbackInterface fpNewMessageHandler);

BOOL MessageHandlerRemove(const CHAR* szRemoveMessageHandlerType);

f_CallbackInterface MessageHandlerFindFirst(const CHAR* szFindMessageHandlerType);

#endif
