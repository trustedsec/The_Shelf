#pragma once
#ifndef __LIB_InteractiveCommandPrompt__
#define __LIB_InteractiveCommandPrompt__

#include <Windows.h>

#define InteractiveCommandPrompt_Configuration						"interactive_configuration"
#define InteractiveCommandPrompt_Configuration_ParentProcess		"parent_process"
#define InteractiveCommandPrompt_Configuration_CommandPrompt		"command_prompt"

#define InteractiveCommandPrompt_Request							"interactive_command_prompt_request"
//#define InteractiveCommandPrompt_Request_Entries					"interactive_commands"
#define InteractiveCommandPrompt_Request_CommandInput				"command_input"

//#define InteractiveCommandPrompt_Response							"interactive_response"
//#define InteractiveCommandPrompt_Response_Results					"interactive_results"
#define InteractiveCommandPrompt_Response_CommandOutput				"command_output"


DWORD WINAPI InteractiveCommandPromptInitialization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI InteractiveCommandPromptFinalization(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

DWORD WINAPI InteractiveCommandPromptCallbackInterface(
	IN	PBYTE	lpInputBuffer,
	IN	DWORD	dwInputBufferSize,
	OUT PBYTE*	lppOutputBuffer,
	OUT PDWORD	lpdwOutputBufferSize
);

#endif
