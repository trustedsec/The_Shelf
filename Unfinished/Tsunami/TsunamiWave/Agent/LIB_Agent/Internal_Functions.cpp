#include <windows.h>
#include <stdio.h>

#pragma message( "Compiling with DEBUGGING messages hardcoded in " __FILE__ )
#define _VERBOSE
#define DEBUG_LEVEL DEBUG_VERBOSE
#include "DebugOutput.h"
#include "LIB_Utilities.h"

#include "Internal_Functions.h"


PPLUGIN_NODE g_Agent_lpPluginList = NULL;

PPLUGIN_NODE PluginInsert(const CHAR* szNewPluginName, const HANDLE hNewPlugin)
{
	PPLUGIN_NODE pNewPlugin = (PPLUGIN_NODE)malloc(sizeof(PLUGIN_NODE));
	if (NULL != pNewPlugin)
	{
		memset(pNewPlugin, 0, sizeof(PLUGIN_NODE));
		strcpy(pNewPlugin->szPluginName, szNewPluginName);
		pNewPlugin->hPlugin = hNewPlugin;
		pNewPlugin->lpNext = g_Agent_lpPluginList;
		g_Agent_lpPluginList = pNewPlugin;
	}
	return pNewPlugin;
}

BOOL PluginRemove(const CHAR* szRemovePluginName, const HANDLE hRemovePlugin)
{
	PPLUGIN_NODE pPreviousPlugin = NULL;
	PPLUGIN_NODE pCurrentPlugin = NULL;
	BOOL bFound = FALSE;

	if (g_Agent_lpPluginList)
	{
		pPreviousPlugin = NULL;
		pCurrentPlugin = g_Agent_lpPluginList;

		DBGPRINT(DEBUG_VERBOSE, TEXT("hRemovePlugin:       %p"), hRemovePlugin);
		DBGPRINT(DEBUG_VERBOSE, TEXT("szRemovePluginName:  %S"), szRemovePluginName);

		do
		{
			DBGPRINT(DEBUG_VERBOSE, TEXT("pCurrentPlugin->hPlugin:       %p"), pCurrentPlugin->hPlugin);
			DBGPRINT(DEBUG_VERBOSE, TEXT("pCurrentPlugin->szPluginName:  %S"), pCurrentPlugin->szPluginName);

			// Check if we want to remove this node
			if (
				( 
				 (NULL == szRemovePluginName) 
				  &&
				 (NULL==hRemovePlugin)
				)
				||
				( 
				 (0 == strcmp(szRemovePluginName, pCurrentPlugin->szPluginName))
				  &&
				 (NULL == hRemovePlugin)
				)
				||
				(
				 (NULL == szRemovePluginName)
				  &&
				 (pCurrentPlugin->hPlugin == hRemovePlugin)
				)
				||
				(
				 (0 == strcmp(szRemovePluginName, pCurrentPlugin->szPluginName))
				  &&
				 (pCurrentPlugin->hPlugin == hRemovePlugin)
				)
			)
			{
				bFound = TRUE;

				// Are we removing the list head? If so, pPreviousPlugin will be NULL
				if (NULL == pPreviousPlugin)
				{
					g_Agent_lpPluginList = pCurrentPlugin->lpNext;
					free(pCurrentPlugin);
					pCurrentPlugin = g_Agent_lpPluginList;
				}
				else
				{
					pPreviousPlugin->lpNext = pCurrentPlugin->lpNext;
					free(pCurrentPlugin);
					pCurrentPlugin = pPreviousPlugin->lpNext;
				}
			}
			else
			{
				pPreviousPlugin = pCurrentPlugin;
				pCurrentPlugin = pPreviousPlugin->lpNext;
			}
		} while (pCurrentPlugin);
	}

	return bFound;
}


HANDLE PluginFindHandle(const CHAR* szPluginName)
{
	PPLUGIN_NODE pPreviousPlugin = NULL;
	PPLUGIN_NODE pCurrentPlugin = NULL;

	if (g_Agent_lpPluginList)
	{
		pPreviousPlugin = NULL;
		pCurrentPlugin = g_Agent_lpPluginList;
		do
		{
			// Find first handler that matches input
			if (0 == strcmp(szPluginName, pCurrentPlugin->szPluginName))
			{
				return pCurrentPlugin->hPlugin;
			}

			pPreviousPlugin = pCurrentPlugin;
			pCurrentPlugin = pPreviousPlugin->lpNext;
		} while (pCurrentPlugin);
	}

	return NULL;
}