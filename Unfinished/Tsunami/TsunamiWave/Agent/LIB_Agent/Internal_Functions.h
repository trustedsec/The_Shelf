#pragma once
#ifndef __LIB_Internal_Agent_Functions__
#define __LIB_Internal_Agent_Functions__

#include <Windows.h>

#define MAX_PLUGIN_NAME 128

typedef struct _PLUGIN_NODE {
	CHAR szPluginName[MAX_PLUGIN_NAME];
	HANDLE hPlugin;
	_PLUGIN_NODE* lpNext;
} PLUGIN_NODE, *PPLUGIN_NODE;

extern PPLUGIN_NODE g_Agent_lpPluginList;

PPLUGIN_NODE PluginInsert(const CHAR* szNewPluginName, const HANDLE hNewPlugin);

BOOL PluginRemove(const CHAR* szRemovePluginType, const HANDLE hRemovePlugin);

HANDLE PluginFindHandle(const CHAR* szPluginName);

#endif
