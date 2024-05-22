#pragma once
#ifndef __LIB_Agent__
#define __LIB_Agent__

#include <Windows.h>

#define Agent_Initial_Messages					"initial_messages"
#define Agent_Periodic_Messages					"periodic_messages"

#define Agent_GetSettings_Request				"agent_get_settings_request"
#define Agent_GetSettings_Response_Settings		"settings"
#define Agent_SetSettings_Request				"agent_set_settings_request"
#define Agent_SetSettings_Request_Settings		"settings"
#define Agent_SetSettings_Response_Result		"result"

#define Agent_ListPlugins_Request					"agent_list_plugins_request"
#define Agent_ListPlugins_Response_Plugins			"plugins"
#define Agent_ListPlugins_Response_Plugin_Handle	"plugin_handle"
#define Agent_ListPlugins_Response_Plugin_Name		"plugin_name"
#define Agent_LoadPlugin_Request					"agent_load_plugin_request"
#define Agent_LoadPlugin_Request_Plugin_Buffer		"plugin_buffer"
#define Agent_LoadPlugin_Request_Plugin_Name		"plugin_name"
#define Agent_LoadPlugin_Request_Plugin_Config		"plugin_config"
#define Agent_LoadPlugin_Response_Plugin_Handle		"plugin_handle"
#define Agent_UnloadPlugin_Request					"agent_unload_plugin_request"
#define Agent_UnloadPlugin_Request_Plugin_Name		"plugin_name"
#define Agent_UnloadPlugin_Request_Plugin_Handle	"plugin_handle"
#define Agent_UnloadPlugin_Response_Result			"result"


DWORD WINAPI AgentInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentCallback(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentGetSettings(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentSetSettings(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);


DWORD WINAPI AgentListPlugins(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentLoadPlugin(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI AgentUnloadPlugin(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);









#endif
