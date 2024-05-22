# TSunami

This is a custom C2 framework that was developed and intended to be a fallback agent if our primary agent was to ever get burned.  
After an initial development period the project was determined to be non-viable and put on a backburner to potentially be picked up later.  At this point we know we won't be resuming development on this project so we're releasing it here.

MIT as it was all in house development.

# TSunami Framework
The TSunami Framework is a server-client framework for deployable, remotely controllable agents. The agents are designed to provide basic operations to the operator via the server. The server provides basic command and control to the operator via a command-line interface. The framework should eventually support a variety of agents all conforming to a basic message passing format as well as a variety of command and control servers utilizing different underlying message passing protocols. The ultimate goal is to provide a common command and control interface to the operator utilizing a command message passing framework to abstract the underlying details.

At present, there is only one agent, TSunamiWave, and one server, TSunamiWarning.
## TSunami Messages
All TSunami messages use JSON for easy integration. The standard JSON has been extended to include a Bytes type which can include binary data which has been Base64 encoded. All TSunami messages currently contain a JSON string variable containing the message or command type, 'command'. They also include a JSON number representing a message ID, 'message_id'. The command type has a corresponding JSON bytes type representing the module/message handler specific message buffer. Currently, any responses are appended to the original TSunami request message for easy request-response matching.
```JSON
{
"binary_id": 12345,
"agent_id": 6789,
"command": "message_handler_request",
"message_id": 1,
"message_handler_request": "64==ABCDefgh123="
}
```

# TSunamiWarning Server
Ths TSunamiWarning server is the first server designed to support the TSunami framework. It was writting in python3 and developed to run on any LAMP instance. The agent was written on a Windows machine using PyCharm and could theoretically run on any machine supporting php and python.

## Coding Design
The server design is pretty simple. The actual server piece is a php script, while the second piece of the server is the command-line interface which is written in python.

### PHP Server
The server's php script is named upload.php. The PHP script is extremely simple and pretty much only does file IO operations. It does not make use of any third-party code and does not perform any encryption or encoding.

### Python CLI
The server's command-line interface is written in python3. Some of the functions have been put in the tsunami.py module for possible reuse, while most execution takes place in the tsunami_warning.py module. The server relies on several third party python modules as listed in the requirements.pip file. Also, the Crypto, wincrypto, and lznt1 modules were modified to add python3 compatability. The public libraries do not support python3, so these local versions need to be copied to the python site-libs. Since the server's main task is to create messages, it performs a lot of JSON manipulation.

The server utilizes a folder hierarchy as part of the command-line interface. For instance, all supported commands are stored as JSON files in the command subdirectory. The command's JSON file describes the options avaiable and required for each specific command, while a baseline JSON file describes the options available and required for ALL commands. There is a similar folder for all supported installs and interactive modules. There is also a folder for the payloads which can be utilized. And finally, there is an agent folder for all the corresponding messages and admin information.

## Concept of Operations
The PHP server and python command-line interface should operate independently. They interact through the creation of JSON files (most likely compressed, encrypted, and encoded). This independent operation technically allows the two pieces to be on two separate machines.

### PHP Server
The PHP server accepts HTTP requests from the client. It parses this request to determine the agent calling, writes any messages from the client to file, checks to see if there are any messages queued for the agent, and replies with these messages. The message itself is compressed, encrypted, and encoded, but the PHP script doesn't perform any of these operations and doesn't care, it simply reads and writes files.

1. Receive POST request from agent
2. Parse HTTP header's cookie variable to get the agent's binary_id and agent_id
3. Create the upload/download/sents directory for the agent if they do not exist
	* Note: php server uid must have permissions to create these directories
4. Create callback.log entry for the request
	* The entry contains binary_id, agent_id, ip_address, datetime
5. Write the POST body to file in the agent's upload folder
	* The file name is created using the date, binary_id, agent_id, ip_address
6. Scan the download directory for any messages queued to send to agent
	* Read files, and write them to the HTTP response
7. Move any files sent as part of the HTTP response to the agent's sents folder

### Python CLI
The command-line interface really just generates and parses the TSunami messages which are all JSON based. The command-line relies on the folder structure to help describe and implement this command-line interface. It does not require any command-line arguments, but the python script does support --debug and --verbose for additional information.
1. Start the command-line interface
2. Perform an operation
	* Use a command, install, or interactive
	* List supported command, install, or interactive
	* List agent or agent messages
	* Show agent message
	* Help
3. Read/Write message
	* Typically as part of the operation performed a message JSON file will be read or written to file
4. Loop until Exit
 
## Commands
The command-line interface extends the cmd2 python module. This module has some built-in command support, e.g., history and alias. The cmd2 module also allows for custom tab completion and help. In addition to the built-in cmd2 commands, the server contains the following commands:

### help
``help`` is used to display usage and helpful information about a command. Example:
```
help list
```

### list
``list`` is used to list objects in a folder. This is usually used to list commands or agent messages sent, received, or queued. The list command does support tab completion. Example:
```
list command
list agent/12345/6789/uploads
list agent/12345/6789/sents
```

### show
``show`` is used to display the contents of files. This is usually used to display encoded message files (BINs) or agent configuration files (JSONs). The show command supports tab completion. Example:
```
show agent/12345/6789/sents/80000001.bin
show agent/12345/6789/admin/AgentConfig.json
```

### use
``use`` is used to select the operation to perform. This is usually used to select a command message to start creating or an installer to start generating or an interactive plug-in to start using. The use command supports tab completion. Example:
```
use command/file_basic_upload.json
use install/TrustedWave_x64.exe
use interactive/command_prompt.json
```

### back
 ``back`` is used to unselect the current operation. This is usually used once the current operation has been performed (once you've ``run`` the command), or it is used to back up and select a different command. There are no arguments to this command. Example:
```
back
```

### options
``options`` is used to list, set, or unset an option for the current operation. The options command can list the available options/variables that can be set for the selected operation by simply running ``options`` without any arguments. It can then be used to set command or installer arguments given the options/variable name to set and the corresponding value. Finally, the options command can also be used to unset a currently defined options/variable by supplying the option name without a value. The options command does support tab completion for option/variable names within the currently selected operation. Example:
```
options
options agent_id 6789
options agent_id
```
Note: When options is run without any arguments the available options/variables/settings are listed with a description of the setting, the type of input (int, string, filename, etc.), and whether or not the options/setting is required.

### run
``run`` is used to actually generate/run the currently selected option. For instance, if a command is being 'use'd, then the corresponding message file is generated (compressed, encrypted, encoded) and written to file under the downloads directory waiting for the agent to callback and download. If an installer is being 'use'd, then the corresponding binary is generated and written to file under the admin directory of the agent. If an interactive plugin is being 'use'd, then a separate terminal window is opened to execute the interactive plugin. Before generating any files, the CLI will check if all required options have been filled in and will automatically generate/fill-in any options marked as auto. The command does not take any arguments.  Example:
```
run
```

### start_alerts/stop_alerts
``start_alerts`` is used to monitor for messages received from a specific agent (including the periodic callback messages). This is usually used when you are interactive with an agent and want an alert when a message is received. The alert is a single line of text above the operator's current command prompt which lists the agent that called back and where that message was written. The message can then be displayed using the ``show`` command. The ``start_alerts`` command takes a ``binary_id`` and ``agent_id`` as arguments. Example:
```
start_alerts 12345 6789
```
``stop_alerts`` is used to stop monitoring for messages (to stop alerts). This is usually done when you are done interacting with an agent or find the alerts overwhelming. The ``stop_alerts`` command can take a ``binary_id`` and ``agent_id`` as arguments or can stop all alerts when run without any arguements. Example:
```
stop_alerts 12345 6789
stop_alerts
```

### exit
``exit`` is used to exit the command-line interface. This is usually performed when you are done creating messages, installers, or interacting with an agent interactively. Exiting the command-line interface will not stop the server. Any messages queued to be sent will still be sent when the agent calls back, and any messages received from agents will be written to disk; however, all alerts will be stopped and will NOT be displayed the next time you start the command-line interface. Example:
```
exit
```

# TSunamiWave Agent
The TSunamiWave agent is the first agent designed to support the TSunami framework. It was written in C and developed to run on Windows 7+. The agent was written using Visual Studio and compiled using the static libraries from the WinDDK 7. The build results in a DLL or an EXE and can be built for either x86 or x64. The code only uses Windows libraries and private code--no third party libraries--although it did borrow heavily from the cJSON library (the codes has been extended and modified).

## Coding Design
The agent will be built using a main agent module and some necessary/linked modules. The agent will include additional modules to provide functionality. Finally, the agent will also include some embedded configuration settings for itself and the additional modules.
* Linked libraries
	* Utilities
	* Encoding
	* Encryption
	* Compression
	* Mixed Strings
	* Configuration storage
	* Loader
		* From memory DLL loading
	* Message handler manager
		* Register/unregister new message handlers
		* Dispatch messages
	* C2 module
		* Basic send/receive using wininet
	* File operations
		* Basic upload/download
		* File delete
	* Surveys
		* Registry
		* Process
		* File
		* Network
		* Host
	* Triggers
		* Sleep
	* Binary execution
		* Generic load library
		* Generic create process
	* Agent operations
		* Get/set settings
	* Load/list/unload modules
* Additional modules
	* Interactive commands
	* Basic Windows command-prompt
* Embedded configuration settings
	* Agent ID
	* Binary ID
	* Initial commands
		* Initial delay
		* Initial survey
	* Periodic commands
		* Periodic delay
		* Periodic survey
	* Callback server
	* Callback user agent

## Concept of Operations
The agent will load configuration settings embedded in data section (eventually hope to have dynamic storage on disk for subsequent reboots/updates). The agent will load any linked module libraries. The agent will perform any initial commands. Finally, the agent will enter into the periodic command and control loop.
1. Load configuration
	1. Load configuration settings embedded in the agent at build time
	2. *TODO* Overwrite the configuration settings with any stored configuration settings, e.g., from the registry or file on disk.
2. Load Additional Modules
	* Register library modules linked into the binary
	* *TODO* Load/register subsequently uploaded DLL modules stored on disk
3. Perform initial commands
	* Initial delay
	* Initial survey
	* *TODO* Validation checks for already running instance or analyst tools or domain/host validation
4. Enter Periodic Command and Control Loop
	1. Send any queued messages to the server
	2. Receive any queued message from the server (currently 1 message per callback)
	3. Process message
		* Find correct message handler
		* Pass message and wait for response (*TODO* synchonous/threaded calls)
	4. Queue response message
	5. Perform periodic messages
		* Periodic delay
		* Periodic survey
		* Note: The message_id of the agent's initial commands is 0. The message_id of the periodic messages is the iteration count of the number of times through the periodic loop.
		* 
## Plug-in Interface
The plug-in interface will be used for build-time and run-time modules. It shall allow for easy integration, modification and extension of the agent and capabilities. The plug-in interface will consist of an initialization, registered callbacks, and finalization. For simplicity all interfaces use the same function prototype:
```C
typedef DWORD(WINAPI* f_Interface)(
IN PBYTE lpInputBuffer,
IN DWORD dwInputBufferSize,
OUT PBYTE* lppOutputBuffer,
OUT PDWORD lpdwOutputBufferSize
);
```

### Initialization
When a plug-in is loaded, the central agent will first call the plug-in's initialization function. This initialization function will allow the plug-in to perform any necessary initialization steps and register any callback functions (providing the corresponding message type).

### Callback
When a plug-in is loaded and initialized, it will be registered to receive messages. The central agent module will process each message. It will go through the list of registered callback message handler functions, and call each callback function registered to receive the message type pasing the messageto the callback function. The agent will then wait for any output/response to the function.

### Finalization
When the agent is terminating or when a plug-in has been unloaded, the agent will call each plug-in's finalization function. The finalization function will allow the plug-in to perform any necessary finalization steps including unregistering any message handlers.

# TODOs
 - [ ] Perform message ID checking
 - [ ] Do we really need to send back the request as part of the response
 - [ ] MixText across all modules
 - [ ] Store settings for across reboots/updates
 - [ ] Any validation steps, e.g., another instance running, analyst tools running, correct host/domain/host
 - [ ] Persistence
 - [ ] VirtualAlloc / malloc : choose one
 - [ ] More file operations, e.g., copy, move
 - [ ] Create more triggers, e.g., specific datetime, specific message received, specific domain/website reachable
 - [ ] Alert if haven't received a callback from the agent in scheduled amount of time
 - [ ] Set the agent ID and binary ID for use between command creations on the command-line user interface
 
