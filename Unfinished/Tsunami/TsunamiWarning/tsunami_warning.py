#!/usr/bin/env python3

import sys
import argparse
import os
import random
import json
import importlib
from cmd2 import with_argparser
from cmd2 import with_argparser_and_unknown_args
from cmd2 import Cmd
from cmd2 import utils
import tsunami
from cmd2 import ansi
import string
import threading
import time
import datetime
from glob import glob
import subprocess
import shutil

MODULE_NAME = 'TSunami'




def _complete_dir_with_ignore(path: str) -> bool:
    # print("path:",path)
    return os.path.isdir(path) and not os.path.basename(path).startswith(".") \
           and not os.path.basename(path).startswith("_")


def _complete_file_with_ignore(path: str) -> bool:
    # print("path:",path)
    return not os.path.basename(path).startswith(".") \
           and not os.path.basename(path).startswith("_") \
           and (os.path.isdir(path)
                or
                (
                        os.path.isfile(path)
                        and (
                                os.path.basename(path).endswith(".json")
                                or os.path.basename(path).endswith(".bin")
                        )
                )
                )


def _complete_payload_file_with_ignore(path: str) -> bool:
    # print("path:",path)
    return not os.path.basename(path).startswith(".") \
           and not os.path.basename(path).startswith("_")



ALERT_TARGETS = []


def ParseTsunamiWarningCommandLine():
    """Parse the command-line arguments"""
    # Create and add arguments to the parser
    parser = argparse.ArgumentParser(description=MODULE_NAME)
    parser.add_argument("--agent_directory", dest="agents_dir", type=str, required=False,
                        help="Agents directory for the server")
    parser.add_argument("--command_directory", dest="commands_dir", type=str, required=False,
                        help="Commands directory for the server")
    parser.add_argument("--install_directory", dest="installs_dir", type=str, required=False,
                        help="Installs directory for the server")
    parser.add_argument("--payload_directory", dest="payloads_dir", type=str, required=False,
                        help="Payloads directory for the server")
    parser.add_argument("--debug", dest="debug", action='store_true',
                        help="Display debugging statements")
    parser.add_argument("--verbose", "-v", dest="verbose", action='store_true',
                        help="Display verbose print statements")
    parser.set_defaults(
        debug=False,
        verbose=False,
        agents_dir = tsunami.AGENTS_DIRECTORY,
        commands_dir = tsunami.COMMANDS_DIRECTORY,
        installs_dir = tsunami.INSTALLS_DIRECTORY,
        payloads_dir = tsunami.PAYLOADS_DIRECTORY
    )
    # Parse the arguments
    args = parser.parse_args()
    # Sanity check on arguments
    if not os.path.isdir( args.agents_dir ):
        raise FileNotFoundError( args.agents_dir )
    if not os.path.isdir( args.commands_dir ):
        raise FileNotFoundError( args.commands_dir )
    if not os.path.isdir( args.installs_dir ):
        raise FileNotFoundError( args.installs_dir )
    if not os.path.isdir( args.payloads_dir ):
        raise FileNotFoundError( args.payloads_dir )
    return args


class TsunamiPrompt(Cmd):

    def __init__(self):

        args = ParseTsunamiWarningCommandLine()

        super().__init__(allow_cli_args=False)
        self.intro = "TSunami Command Prompt\n\n  Type 'help' or '?' for a list of commands\n"

        if args.debug == True:
            self.debug = True
        if args.verbose == False:
            self.quiet = True
            
        self.commands_dir = args.commands_dir
        self.agents_dir = args.agents_dir
        self.installs_dir = args.installs_dir
        self.payloads_dir = args.payloads_dir
        self.server_dir = os.getcwd()
        self.primary_module_name = ""
        self.primary_options = dict()
        self.original_primary_options = dict()
        self.secondary_module_name = ""
        self.secondary_options = dict()
        self.original_secondary_options = dict()
        self.missing_required_count = 0

        self.pfeedback("commands_dir: {:s}".format(self.commands_dir))
        self.pfeedback("agents_dir:   {:s}".format(self.agents_dir))
        self.pfeedback("installs_dir: {:s}".format(self.installs_dir))
        self.pfeedback("payloads_dir: {:s}".format(self.payloads_dir))
        self.pfeedback("server_dir:   {:s}".format(self.server_dir))

        self._stop_thread = False
        self._alerter_thread = threading.Thread()
        self._last_alert_time = time.time()

        self._interactive_threads = []

        self.register_preloop_hook(self._preloop_hook)
        self.register_postloop_hook(self._postloop_hook)


        self._set_prompt()




    def _set_prompt(self):
        """Set prompt so it displays the current use command."""
        if self.secondary_module_name == "":
            self.prompt = ansi.style('TSunami $ ', fg='green')
        else:
            self.prompt = ansi.style('TSunami(', fg='green') + \
                          ansi.style('{!r}'.format(self.secondary_module_name), fg='yellow') + \
                          ansi.style(') $ ', fg='green')

    def postcmd(self, stop: bool, line: str) -> bool:
        """Hook method executed just after a command dispatch is finished.
        :param stop: if True, the command has indicated the application should exit
        :param line: the command line text for this command
        :return: if this is True, the application will exit after this command and the postloop() will run
        """
        """Override this so prompt always displays updated command prompt."""
        self._set_prompt()
        return stop

    def _preloop_hook(self) -> None:
        """ Start the alerter thread """
        # This runs after cmdloop() acquires self.terminal_lock, which will be locked until the prompt appears.
        # Therefore this is the best place to start the alerter thread since there is no risk of it alerting
        # before the prompt is displayed. You can also start it via a command if its not something that should
        # be running during the entire application. See do_start_alerts().
        self._stop_thread = False
        self._alerter_thread = threading.Thread(name='alerter', target=self._alerter_thread_func)
        self._alerter_thread.start()

    def _postloop_hook(self) -> None:
        """ Stops the alerter thread """
        # After this function returns, cmdloop() releases self.terminal_lock which could make the alerter
        # thread think the prompt is on screen. Therefore this is the best place to stop the alerter thread.
        # You can also stop it via a command. See do_stop_alerts().
        self._stop_thread = True
        if self._alerter_thread.is_alive():
            self._alerter_thread.join()




    def _get_current_agent_password_from_dict(self, main_options: dict) -> str:
        current_password = None
        current_agent_directory = self._get_current_agent_directory_from_dict(main_options)
        current_config_filename = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, tsunami.AGENT_CONFIG)
        if not os.path.isfile(current_config_filename):
            self.perror("A configuration file '{:s}' for agent_id {:d} and binary_id {:d} does NOT exist".format(tsunami.AGENT_CONFIG, main_options[tsunami.MESSAGE_CONFIG_AGENT_ID], main_options[tsunami.MESSAGE_CONFIG_BINARY_ID]))
        else:
            with open(current_config_filename, "r") as input_fd:
                current_config_json = input_fd.read()
            current_config_dict = json.loads(current_config_json)
            if tsunami.AGENT_PASSWORD_ENTRY not in current_config_dict:
                self.perror(
                    "The password is not set in the configuration file '{:s} for agent_id {:d} and binary_id {:d}".format(tsunami.AGENT_CONFIG, main_options[tsunami.MESSAGE_CONFIG_AGENT_ID], main_options[tsunami.MESSAGE_CONFIG_BINARY_ID]))
            else:
                current_password = current_config_dict[tsunami.AGENT_PASSWORD_ENTRY]
        return current_password

    def _get_agent_password_from_path(self, path: str) -> str:
        """Helper method to determine the agent password from the configuration json in the specified path.
               :param path: the current path
               :return: str, the password from the current path or None if it could not determine a password
               """
        current_password = None
        full_path = os.path.abspath(path)
        agents_root_dir = os.path.join(self.server_dir, self.agents_dir)
        if not full_path.startswith(agents_root_dir):
            self.perror("Invalid path for agent")
        else:
            relative_agent_path = os.path.relpath(full_path, agents_root_dir)
            rel_agent_list = relative_agent_path.split(os.path.sep)
            binary_id = rel_agent_list[0]
            agent_id = rel_agent_list[1]
            current_agent_directory = os.path.join(agents_root_dir, binary_id, agent_id)
            current_config_filename = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, tsunami.AGENT_CONFIG)
            if not os.path.isfile(current_config_filename):
                self.perror("A configuration file '{:s}' for agent_id {:d} and binary_id {:d} does NOT exist".format(
                    tsunami.AGENT_CONFIG, self.primary_options[tsunami.MESSAGE_CONFIG_AGENT_ID],
                    self.primary_options[tsunami.MESSAGE_CONFIG_BINARY_ID]))
            else:
                with open(current_config_filename, "r") as input_fd:
                    current_config_json = input_fd.read()
                current_config_dict = json.loads(current_config_json)
                if tsunami.AGENT_PASSWORD_ENTRY not in current_config_dict:
                    self.perror(
                        "The password is not set in the configuration file '{:s}' for agent_id {:d} and binary_id {:d}".format(
                            tsunami.AGENT_CONFIG, self.primary_options[tsunami.MESSAGE_CONFIG_AGENT_ID],
                            self.primary_options[tsunami.MESSAGE_CONFIG_BINARY_ID]))
                else:
                    current_password = current_config_dict[tsunami.AGENT_PASSWORD_ENTRY]
        return current_password

    def _get_current_output_filename_from_dict(self, main_options: dict) -> str:
        current_output_filename = None
        current_agent_directory = self._get_current_agent_directory_from_dict(main_options)
        if 'command' in main_options.keys():
            current_message_id = main_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID]
            if isinstance(current_message_id, str):
                self.perror("The message_id not set")
            else:
                message_id_string = '{:08x}'.format(current_message_id)
                current_output_filename = os.path.join(current_agent_directory,
                                                       tsunami.DOWNLOADS_DIRECTORY,
                                                       message_id_string + tsunami.MESSAGE_EXTENSION)
                if os.path.isfile(current_output_filename):
                    self.pwarning("Message {:d} for binary_id {:d} with agent_id {:d} already exists".format(current_message_id, main_options[tsunami.MESSAGE_CONFIG_BINARY_ID], main_options[tsunami.MESSAGE_CONFIG_AGENT_ID]))
        elif 'install' in main_options.keys():
            current_output_filename = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, main_options['install'])
        else:
            self.perror("Do not know how to generate an output filename from given options")
        return current_output_filename

    def _get_current_agent_directory_from_dict(self, main_options: dict) -> str:
        current_agent_directory = None
        if tsunami.MESSAGE_CONFIG_BINARY_ID not in main_options:
            self.perror("binary_id is missing")
        elif not isinstance(main_options[tsunami.MESSAGE_CONFIG_BINARY_ID], int):
            self.perror("binary_id is not set (try 'options')")
        elif tsunami.MESSAGE_CONFIG_AGENT_ID not in main_options:
            self.perror("agent_id is missing")
        elif not isinstance(main_options[tsunami.MESSAGE_CONFIG_AGENT_ID], int):
            self.perror("agent_id is not set (try 'options')")
        else:
            agent_id_string = str(main_options[tsunami.MESSAGE_CONFIG_AGENT_ID])
            binary_id_string = str(main_options[tsunami.MESSAGE_CONFIG_BINARY_ID])
            # Create the file destination
            current_agent_directory = os.path.join(self.agents_dir, binary_id_string, agent_id_string)
        return current_agent_directory

    def _check_required(self, options: dict) -> int:
        """Helper method to determine if there are any required options not set.
        :param options: the dictionary to check (could be primary or secondary options)
        :return: int, the number of required options remaining to set
        """
        required = 0
        for key, value in options.items():
            # If it's a dictionary, then recursively call print_json
            if isinstance(value, dict):
                required = required + self._check_required(value)
                continue
            # Else, not a new dictionary, so print the value
            elif isinstance(value, str):
                value_type = ''
                if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                    value_args = value.split()
                    value_prompt = value_args[0][0]
                    value_type = value_args[0][1:]
                    value_required = value_args[1]
                    value_description = value_args[3]
                    # print("value_prompt", value_prompt)
                    # print("value_type",value_type)
                    # print("value_required", value_required)
                    # print("value_description", value_description)
                    if "required" == value_required:
                        required = required + 1
                        self.perror("The option '{:s}' is required (try 'options')".format(key))
        return required

    def _check_optional_and_auto(self, options: dict) -> dict:
        """Helper method to determine if there are any optional options and remove them, and if there are any auto
        options and automatically fill them.
        :param options: the dictionary to check (could be primary or secondary options)
        :return: dict, the new stripped down and auto-filled options dictionary
        """
        optional_count = 0
        auto_count = 0
        random_count = 0
        auto_calc_value = 0
        new_options = options.copy()
        for key, value in options.items():
            # If it's a dictionary, then recursively call print_json
            if isinstance(value, dict):
                new_options = self._check_optional_and_auto(new_options)
                continue
            # Else, not a new dictionary, so print the value
            elif isinstance(value, str):
                value_type = ''
                if value.startswith('?') or value.startswith('!') or value.startswith('#'):
                    value_args = value.split()
                    value_prompt = value_args[0][0]
                    value_type = value_args[0][1:]
                    value_required = value_args[1]
                    value_description = value_args[3]
                    # print("value_prompt", value_prompt)
                    # print("value_type", value_type)
                    # print("value_required", value_required)
                    # print("value_description", value_description)
                    if "optional" == value_required:
                        optional_count = optional_count + 1
                        self.pwarning("The option '{:s}' is not set, but it is optional (try 'options')".format(key))
                        new_options.pop(key)
                    if "auto" == value_required:
                        random_count = random_count + 1
                        self.pfeedback("Auto-incrementing the value for option '{:s}'...".format(key))
                        if 'command' in self.primary_options.keys():
                            current_agent_directory = self._get_current_agent_directory_from_dict(self.primary_options)
                            auto_calc_file = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, key)
                        elif 'install' in self.primary_options.keys():
                            current_config_directory = os.path.join(self.installs_dir, self.primary_options['install'])
                            auto_calc_file = os.path.join(current_config_directory, key)
                        if not os.path.isfile(auto_calc_file):
                            self.pwarning("Auto-increment file for option '{:s}' is not found".format(key))
                            auto_calc_value = 0
                        else:
                            auto_calc_string = "0"
                            with open(auto_calc_file, "r") as input_fd:
                                auto_calc_string = input_fd.read()
                            auto_calc_value = int(auto_calc_string)
                            auto_calc_value = auto_calc_value + 1
                        self.pfeedback("Used auto incremented value of {:d} for option '{:s}'".format(auto_calc_value, key))
                        with open(auto_calc_file, "w") as output_fd:
                            output_fd.write(str(auto_calc_value))
                        new_options[key] = auto_calc_value
                    if "random" == value_required:
                        random_count = random_count + 1
                        random_value = None
                        self.pfeedback("Generating a random value for option '{:s}'...".format(key))
                        if value_type.startswith('string'):
                            value_type_size = 38
                            value_type_size_str = value_type[len("string"):]
                            if len(value_type_size_str) > 0:
                                # the size of the string buffer is specified
                                value_type_size = int(value_type_size_str)
                            value_type_range = string.ascii_letters + string.punctuation
                            random_value = ''.join(random.choice(value_type_range) for i in range(value_type_size))
                        elif value_type.startswith('int'):
                            value_type_range_str = value_type[len("int"):]
                            value_type_range = value_type_range_str.split("-")
                            random_value = random.randint(value_type_range[0], value_type_range[1])
                        else:
                            self.perror("Do not know how to create a random '{:s}'".format(value_type))
                            return None
                        self.pfeedback("Generated random value of {}".format(random_value))
                        new_options[key] = random_value
        return new_options


    def _get_alerts(self):
        """
        Reports alerts
        :return: the list of alerts
        """
        global ALERT_TARGETS
        alerts = []
        #print("self._last_alert_time      ",self._last_alert_time)
        for binary_id, agent_id in ALERT_TARGETS:
            alert_target_path = os.path.join(self.agents_dir,str(binary_id),str(agent_id))
            # Check uploads
            alert_target_path_uploads = os.path.join(alert_target_path,tsunami.UPLOADS_DIRECTORY)
            #[(f_name, datetime.datetime.fromtimestamp(os.path.getctime(f_name))) for f_name in glob('*.csv')]
            for file in os.listdir(alert_target_path_uploads):
                full_path = os.path.join(alert_target_path_uploads,file)
                if os.path.isfile(full_path):
                    if file.endswith(".bin"):
                        #print("os.path.getmtime(full_path)", os.path.getmtime(full_path))
                        #print("os.path.getatime(full_path)", os.path.getatime(full_path))
                        #print("os.path.getctime(full_path)", os.path.getctime(full_path))
                        if (os.path.getmtime(full_path) > self._last_alert_time):
                            alerts.append(((binary_id,agent_id),full_path))
        self._last_alert_time = time.time()
        return alerts


    def _generate_alert_str(self) -> str:
        """
        Combines alerts into one string that can be printed to the terminal
        :return: the alert string
        """
        alert_str = ''
        alerts = self._get_alerts()
        for agent, alert_msg in alerts:
            alert_str += "Message received from {:s}:{:s} - {:s}".format(agent[0],agent[1],alert_msg)
        return alert_str


    def _alerter_thread_func(self) -> None:
        """ Prints alerts and updates the prompt any time the prompt is showing """
        while not self._stop_thread:
            # Always acquire terminal_lock before printing alerts or updating the prompt
            # To keep the app responsive, do not block on this call
            if self.terminal_lock.acquire(blocking=False):
                # Get any alerts that need to be printed
                alert_str = self._generate_alert_str()
                # Check if we have alerts to print
                if alert_str:
                    # new_prompt is an optional parameter to async_alert()
                    self.async_alert(ansi.style(alert_str, fg='cyan'), self.prompt)
                # Don't forget to release the lock
                self.terminal_lock.release()
            time.sleep(2.0)



    argparser_start_alerts = argparse.ArgumentParser(
        description="Start alerting for messages received from a specified binary_id and agent_id.",
        epilog="Example:\n\tstart_alerts 12345 6789")
    argparser_start_alerts.add_argument("alert_target", nargs="+", type=str, help="The binary_id and agent_id to alert on")

    @with_argparser(argparser_start_alerts)
    def do_start_alerts(self, start_alerts_args):
        """ Starts the alerter thread """
        global ALERT_TARGETS

        if self._alerter_thread.is_alive():
            self.pfeedback("The alert thread is already started")
        else:
            self._stop_thread = False
            self._last_alert_time = time.time()
            self._alerter_thread = threading.Thread(name='alerter', target=self._alerter_thread_func)
            self._alerter_thread.start()
        if (len(start_alerts_args.alert_target) == 1) or (len(start_alerts_args.alert_target) == 2):
            binary_id = start_alerts_args.alert_target[0]
            agent_id = 0
            if (len(start_alerts_args.alert_target) == 2):
                agent_id = agent_id = start_alerts_args.alert_target[1]

            ALERT_TARGETS.append((binary_id,agent_id))
        else:
            self.perror("Unknown number of arguments for start_alerts. (try help 'start_alerts')")

    argparser_stop_alerts = argparse.ArgumentParser(
        description="Stop alerting for messages received from a specified binary_id and agent_id.",
        epilog="Example:\n\tstop_alerts 12345 6789")
    argparser_stop_alerts.add_argument("alert_target", nargs="*", type=str,
                                        help="The binary_id and agent_id to stop alerting on")

    @with_argparser(argparser_stop_alerts)
    def do_stop_alerts(self, stop_alerts_args):
        """ Stops the alerter thread """
        global ALERT_TARGETS
        if (len(stop_alerts_args.alert_target) == 0):
            ALERT_TARGETS = []
        elif (len(stop_alerts_args.alert_target) == 1) or (len(stop_alerts_args.alert_target) == 2):
            binary_id = stop_alerts_args.alert_target[0]
            agent_id = 0
            if (len(stop_alerts_args.alert_target) == 2):
                agent_id = agent_id = stop_alerts_args.alert_target[1]
            #ALERT_TARGETS.pop((binary_id, agent_id))
            ALERT_TARGETS.remove((binary_id, agent_id))
        else:
            self.perror("Unknown number of arguments for start_alerts. (try help 'start_alerts')")
        if len(ALERT_TARGETS) == 0:
            self._stop_thread = True
            if self._alerter_thread.is_alive():
                self._alerter_thread.join()
            else:
                self.pfeedback("The alert thread is already stopped")


    argparser_back = argparse.ArgumentParser(
        description="Back up / stop using the current C2 command",
        epilog="Example: back")

    # argparser_back.add_argument("level", help="back up to <level>")
    @with_argparser(argparser_back)
    def do_back(self, back_args):
        self.primary_module_name = ""
        self.primary_options = dict()
        self.original_primary_options = dict()
        self.secondary_options = dict()
        self.original_secondary_options = dict()
        self.secondary_module_name = ""

    argparser_list = argparse.ArgumentParser(
        description="List the directory of server including commands, agents, etc.  The agents directory includes the messages queued, sent and received, as well as all admin information like the configuration settings.",
        epilog="Example: list commands")
    argparser_list.add_argument("list_dir", nargs='?', default=".", type=str, help="The directory to list")

    @with_argparser(argparser_list)
    def do_list(self, list_args):
        list_root = self.server_dir
        list_dir = os.path.join(list_root, list_args.list_dir)
        print(ansi.style("  {}:".format(os.path.basename(os.path.abspath(list_dir))), fg='blue'))

        for f in os.listdir(list_dir):
            if os.path.isdir(os.path.join(list_dir, f)) and not f.startswith(".") and not f.startswith("_"):
                print(ansi.style("    {}\\".format(os.path.basename(f)), fg='cyan'))
            elif os.path.abspath(list_dir) != os.path.abspath(list_root) and not f.startswith(".") and not f.startswith(
                    "_"):
                print(ansi.style("    {}".format(os.path.basename(f)), fg='green'))

    def complete_list(self, text, line, begidx, endidx):
        return self.path_complete(text, line, begidx, endidx, path_filter=_complete_dir_with_ignore)

    argparser_options = argparse.ArgumentParser(
        description="List the options / keys for the current namespace, or set an options key value pair, or unset and option key.",
        epilog="Example:\n\toptions (list available options)\n\toptions binary_id (unset the binary_id)\n\topitons binary_id 12345 (set binary_id to 12345)")
    argparser_options.add_argument("options", nargs="*", type=str, help="The options to list/set/unset")

    @with_argparser(argparser_options)
    def do_options(self, options_args):
        if len(options_args.options) == 0:
            if self.primary_module_name != "":
                if (len(self.primary_options.items())) > 0:
                    print(ansi.style("  {:s} {}:".format(self.primary_module_name, "options"), fg='blue'))
                    tsunami.print_json(self.primary_options, 2)
                else:
                    print(ansi.style("  {:s} has no options:".format(self.primary_module_name), fg='blue'))
            else:
                self.perror("Primary module not specified yet (try 'use')")
                return
            if (len(self.secondary_options.items())) > 0:
                print(ansi.style("  {:s} {}:".format(self.secondary_module_name, "options"), fg='blue'))
                tsunami.print_json(self.secondary_options, 2)
            else:
                print(ansi.style("  {:s} has no options".format(self.secondary_module_name), fg='blue'))

        elif len(options_args.options) == 1:
            set_key = options_args.options[0]
            self.pfeedback("Unsetting the option: {:s}".format(set_key))
            if set_key in self.primary_options:
                set_dict = self.primary_options
                original_set_dict = self.original_primary_options
            elif set_key in self.secondary_options:
                set_dict = self.secondary_options
                original_set_dict = self.original_secondary_options
            else:
                self.perror("Option '{:s}' not in options (try 'options')".format(set_key))
                return
            set_dict[set_key] = original_set_dict[set_key]
        elif len(options_args.options) == 2:
            set_key = options_args.options[0]
            set_value = options_args.options[1]
            self.pfeedback("Setting {:s} using {:s}".format(set_key,set_value))
            if set_key in self.primary_options:
                set_dict = self.primary_options
                original_set_dict = self.original_primary_options
            elif set_key in self.secondary_options:
                set_dict = self.secondary_options
                original_set_dict = self.original_secondary_options
            else:
                self.perror("Option '{:s}' is not found in current options (try 'options')".format(set_key))
                return
            # Get the original value which contains the prompt style, type, and description of the value
            original_value = original_set_dict[set_key]
            value_args = original_value.split()
            value_prompt = value_args[0][0]
            value_type = value_args[0][1:]
            try:
                if "int" == value_type:
                    set_dict[set_key] = int(set_value)
                elif value_type.startswith("string"):
                    value_type_size_str = value_type[len("string"):]
                    if len(value_type_size_str) > 0:
                        # the size of the string buffer is specified
                        value_type_size = int(value_type_size_str)
                        if len(set_value) > value_type_size:
                            self.perror(
                                "The option for key '{:s}' is too large (try 'show <submodule used>')".format(set_key))
                    set_dict[set_key] = str(set_value)
                elif "file" == value_type:
                    file_input = str(set_value)
                    file_contents = ''
                    if not os.path.isfile(file_input):
                        self.perror("File '{:s}' not found for option '{:s}'".format(set_value, set_key))
                        return

                    if value_prompt == '!':
                        with open(file_input, 'rb') as file_input_fd:
                            file_contents = file_input_fd.read()
                        self.pfeedback(
                            "Encoding the file contents of '{:s}' for option '{:s}'".format(set_value, set_key))
                        file_contents = tsunami.cJSON_CreateBytes(file_contents)
                    else:
                        with open(file_input, 'r') as file_input_fd:
                            file_contents = file_input_fd.read()
                    set_dict[set_key] = file_contents
                elif "list_string" == value_type:
                    if isinstance(set_dict[set_key], list):
                        self.pfeedback("Appending string list")
                        key_list = set_dict[set_key]
                    else:
                        self.pfeedback("Initializing string list")
                        key_list = []
                    key_list.append(str(set_value))
                    set_dict[set_key] = key_list
                elif "list_int" == value_type:
                    if isinstance(set_dict[set_key], list):
                        self.pfeedback("Appending int list")
                        key_list = set_dict[set_key]
                    else:
                        self.pfeedback("Initializing int list")
                        key_list = []
                    key_list.append(int(set_value))
                    set_dict[set_key] = key_list
                elif "list_command" == value_type:
                    use_secondary_config_file = str(set_value)
                    #print("use_secondary_config_file",use_secondary_config_file)
                    if not os.path.isfile(use_secondary_config_file):
                        use_secondary_config_file = use_secondary_config_file + tsunami.CONFIG_EXTENSION
                    if not os.path.isfile(use_secondary_config_file):
                        self.perror("Used unknown module: '{:s}'".format(set_value))
                        return
                    # Determine the primary module used from the path of the submodule
                    use_primary_config_file = os.path.split(use_secondary_config_file)[0]
                    primary_module_name = os.path.split(use_primary_config_file)[1]
                    use_primary_config_file = os.path.join(use_primary_config_file, tsunami.BASELINE_CONFIG)
                    # Load primary module settings
                    if os.path.isfile(use_primary_config_file):
                        config_file_contents = ''
                        with open(use_primary_config_file, "r") as input_fd:
                            config_file_contents = input_fd.read()
                        primary_options = json.loads(config_file_contents)
                    # Set the submodule name
                    secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
                    # Load secondary module settings
                    config_file_contents = ''
                    # self.pfeedback("Opening secondary module options: {:s}".format(use_secondary_config_file))
                    with open(use_secondary_config_file, "r") as input_fd:
                        config_file_contents = input_fd.read()
                    secondary_options = json.loads(config_file_contents)
                    primary_options[primary_module_name] = secondary_module_name
                    # Set the message_id to 0
                    primary_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID] = 0
                    primary_options.pop(tsunami.MESSAGE_CONFIG_BINARY_ID)
                    primary_options.pop(tsunami.MESSAGE_CONFIG_AGENT_ID)
                    # Prompt to fill-in the command
                    secondary_options = tsunami.prompt_json(secondary_options)
                    secondary_options_string = json.dumps(secondary_options)
                    secondary_options_string_encoded = tsunami.cJSON_CreateBytes(secondary_options_string.encode())
                    primary_options.update({secondary_module_name: secondary_options_string_encoded})
                    # Create the completely filled in options string from JSON
                    #command_json_string = json.dumps(primary_options)
                    if isinstance(set_dict[set_key], list):
                        self.pfeedback("Appending command list")
                        key_list = set_dict[set_key]
                    else:
                        self.pfeedback("Initializing command list")
                        key_list = []
                    #key_list.append(command_json_string)
                    key_list.append(primary_options)
                    set_dict[set_key] = key_list
                else:
                    self.perror("Do not know how to set option '{:s}' with type '{:s}' (try 'back')".format(set_key, value_type))
                    return
            except Exception as e:
                self.perror("Value type mismatch (try 'options')")
                return
        else:
            self.perror("Invalid number of arguments (try 'help options')")

    def complete_options(self, text, line, begidx, endidx):
        # print("text",text)
        # print("line", line)
        # print("begidx", begidx)
        # print("endidx", endidx)
        line_args = line.split()
        if len(line_args) == 2:
            total_options = self.primary_options.copy()
            total_options.update(self.secondary_options)
            match_against = list(total_options.keys())
            return utils.basic_complete(text, line, begidx, endidx, match_against)
        else:
            return self.path_complete(text, line, begidx, endidx, path_filter=_complete_payload_file_with_ignore)

    argparser_run = argparse.ArgumentParser(
        description="Run the current module you are using to generate a command or install",
        epilog="Example: run")

    # argparser_run.add_argument("output", help="The output file to save the run to")
    @with_argparser(argparser_run)
    def do_run(self, run_args):
        self.pfeedback("Checking for required key/values in primary settings...")
        missing_required_count = self._check_required(self.primary_options)
        if 0 < missing_required_count:
            return
        self.pfeedback("Checking for required key/values in secondary settings...")
        missing_required_count = self._check_required(self.secondary_options)
        if 0 < missing_required_count:
            return
        if 'interactive' in self.primary_options.keys():
            self.primary_options[tsunami.AGENT_PASSWORD_ENTRY] = self._get_current_agent_password_from_dict(
                self.primary_options)
            # self.primary_options['agent_directory'] = os.path.abspath(
            #     self._get_current_agent_directory_from_dict(self.primary_options)
            # )
        self.pfeedback("Checking for optional and auto-incrementing key/values in primary settings...")
        new_main_options = self._check_optional_and_auto(self.primary_options)
        self.pfeedback("Checking for optional and auto-incrementing key/values in secondary settings...")
        new_secondary_options = self._check_optional_and_auto(self.secondary_options)
        # Run behaves differently for commands vs installs
        if 'command' in new_main_options.keys():
            seconary_options_type = new_main_options['command']
            self.poutput("Creating {:s}...".format(seconary_options_type))
            # Create the output filename
            current_agent_directory = self._get_current_agent_directory_from_dict(new_main_options)
            message_id_string = '{:08x}'.format(new_main_options[tsunami.MESSAGE_CONFIG_MESSAGE_ID])
            output_filename = os.path.join(current_agent_directory, tsunami.DOWNLOADS_DIRECTORY, message_id_string + tsunami.MESSAGE_EXTENSION)
            if os.path.isfile(output_filename):
                self.pwarning("Command with message_id '{:s}' for binary_id '{:d}' with agent_id '{:d}' already exists".format(message_id_string, new_main_options[tsunami.MESSAGE_CONFIG_BINARY_ID], new_main_options[tsunami.MESSAGE_CONFIG_AGENT_ID]))
            # Create the final input message string from JSON
            print(new_secondary_options)
            secondary_options_string = json.dumps(new_secondary_options)
            self.pfeedback("Secondary options:\n{:s}".format(secondary_options_string[:120]))
            # Convert the input message into a JSON byte buffer
            secondary_options_string_encoded = tsunami.cJSON_CreateBytes(secondary_options_string.encode())
            self.pfeedback("Encoded secondary options:\n{:s}".format(secondary_options_string_encoded[:120]))
            new_main_options.update({seconary_options_type: secondary_options_string_encoded})
            # Create the completely filled in options string from JSON
            input_buffer = json.dumps(new_main_options).encode()
            self.pfeedback("Input buffer:\n{}".format(input_buffer[:120]))
            # Compress, Encrypt, and Encode the command_string
            password = self._get_current_agent_password_from_dict(new_main_options)
            if password is None:
                self.perror("Agent's password is not defined, so cannot create output")
                return
            self.pfeedback("Compressing, encrypting, and encoding the buffer...")
            output_buffer = tsunami.CompressEncryptEncode(input_buffer, password.encode())
            # Write the encoded, encrypted, compressed encoded_command_string to file
            if output_filename is None:
                self.perror("Output name could not be determined, so cannot create output")
                return
            self.pfeedback("Writing to output to file: {:s}...".format(output_filename))
            with open(output_filename, 'wb') as outputfd:
                outputfd.write(output_buffer)
            self.poutput("Saved encoded/encrypted/compressed command message to "+ansi.style("{:s}".format(output_filename), fg='blue'))
        elif 'install' in new_main_options.keys():
            seconary_options_type = new_main_options['install']
            self.poutput("Creating {:s}...".format(seconary_options_type))
            new_main_options.update(new_secondary_options)
            #self.primary_options = new_main_options
            # Get the input/blank install file
            input_file = os.path.join(self.installs_dir, self.primary_options['install'], self.primary_options['install'])
            # Create the output filename
            current_agent_directory = self._get_current_agent_directory_from_dict(new_main_options)
            output_filename = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, seconary_options_type)
            if os.path.isfile(output_filename):
                self.pwarning("Install for binary_id '{:d}' with agent_id '{:d}' already exists".format(new_main_options[tsunami.MESSAGE_CONFIG_BINARY_ID], new_main_options[tsunami.MESSAGE_CONFIG_AGENT_ID]))
            # Create the agent's server directories if necessary
            if not os.path.isdir(os.path.join(current_agent_directory, tsunami.DOWNLOADS_DIRECTORY)):
                os.makedirs(os.path.join(current_agent_directory, tsunami.DOWNLOADS_DIRECTORY))
            if not os.path.isdir(os.path.join(current_agent_directory, tsunami.UPLOADS_DIRECTORY)):
                os.makedirs(os.path.join(current_agent_directory, tsunami.UPLOADS_DIRECTORY))
            if not os.path.isdir(os.path.join(current_agent_directory, tsunami.SENTS_DIRECTORY)):
                os.makedirs(os.path.join(current_agent_directory, tsunami.SENTS_DIRECTORY))
            if not os.path.isdir(os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY)):
                os.makedirs(os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY))
            # Write the config file to the destination agent's admin folder
            config_file = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, tsunami.AGENT_CONFIG)
            self.pfeedback("Creating {:s}...".format(config_file))
            # Remove the binary filename from the options
            new_main_options.pop('install')
            default_password = new_main_options[tsunami.INSTALL_DEFAULT_PASSWORD]
            new_main_options.pop(tsunami.INSTALL_DEFAULT_PASSWORD)
            config_buffer = json.dumps(new_main_options)
            with open(config_file, 'w') as outputfd:
                outputfd.write(config_buffer)
            # Create initial message_id auto increment file
            messageid_file = os.path.join(current_agent_directory, tsunami.ADMIN_DIRECTORY, tsunami.MESSAGE_CONFIG_MESSAGE_ID)
            self.pfeedback("Creating {:s} with initial value of {:s}...".format(messageid_file, tsunami.INITIAL_MESSAGE_ID))
            with open(messageid_file, 'w') as outputfd:
                outputfd.write(tsunami.INITIAL_MESSAGE_ID)
            self.pfeedback("Compressing, encrypting, encoding config buffer...")
            # Compress, Encrypt, and Encode the configuration_buffer
            password = self._get_current_agent_password_from_dict(new_main_options)
            if password is None:
                self.perror("Agent's password is not defined, so cannot create output")
                return
            encoded_configuration_buffer = tsunami.CompressEncryptEncode(config_buffer.encode(), password.encode())
            self.pfeedback("Updating binary with config buffer...")
            # Update the binary with the encoded_configuration_buffer
            tsunami.UpdateBinary(input_file, output_filename, encoded_configuration_buffer, tsunami.INSTALL_DEFAULT_MAGIC_NUMBER, tsunami.INSTALL_CONFIG_BUFSIZE)
            self.pfeedback("Updating binary with new password...")
            # Update the binary with the args.password
            tsunami.UpdateBinary(output_filename, output_filename, password.encode(), default_password.encode(), len(default_password))
            self.poutput("Saved encoded/encrypted/compressed install to "+ansi.style("{:s}".format(output_filename), fg='blue'))
        elif 'interactive' in new_main_options.keys():
            seconary_options_type = new_main_options['interactive']
            self.poutput("Running interactive {:s}...".format(seconary_options_type))
            # new_main_options.update(new_secondary_options)
            # self.pfeedback("{:s}: {:d}".format(tsunami.MESSAGE_CONFIG_BINARY_ID, new_main_options[tsunami.MESSAGE_CONFIG_BINARY_ID]))
            # self.pfeedback("{:s}: {:d}".format(tsunami.MESSAGE_CONFIG_AGENT_ID, new_main_options[tsunami.MESSAGE_CONFIG_AGENT_ID]))
            # self.pfeedback("{:s}: {:s}".format(tsunami.AGENT_PASSWORD_ENTRY, new_main_options[tsunami.AGENT_PASSWORD_ENTRY]))
            # # Get the interactive script
            script_file = os.path.join('interactive', self.primary_options['interactive'] + ".py")
            script_file = os.path.abspath(script_file)
            self.pfeedback("script_file: {:s}".format(script_file))
            if not os.path.isfile(script_file):
                self.perror("Interactive script file '{:s}' not found.".format(script_file))
                return
            agent_directory = os.path.abspath(self._get_current_agent_directory_from_dict(self.primary_options))
            self.pfeedback("agent_directory: {:s}".format(agent_directory))
            if not os.path.isdir(agent_directory):
                self.perror("Interactive agent directory '{:s}' not found.".format(agent_directory))
                return
            interactive_thread = threading.Thread(name='interactive'+str(len(self._interactive_threads)), target=self._interactive_thread_func, args=(script_file,agent_directory))
            self._interactive_threads.append(interactive_thread)
            self.pfeedback("interactive_thread.start()...")
            interactive_thread.start()
            self.pfeedback("interactive_thread.start() returned.")


        else:
            self.perror("Do not know how to run current module")


    argparser_show = argparse.ArgumentParser(
        description="Display the contents of file in the current context. This can display the encrypted-encoded messages for a specific agent as well as the JSON settings.",
        epilog="Example:\n\tshow agents/12345/6789/downloads/0001BA76.bin")
    argparser_show.add_argument("show_file", type=str, help="The file to show")

    @with_argparser(argparser_show)
    def do_show(self, show_args):
        show_root = self.server_dir
        full_show_file = os.path.join(show_root, show_args.show_file)
        print(ansi.style("  {}:".format(os.path.basename(os.path.abspath(full_show_file))), fg='blue'))
        if os.path.basename(full_show_file).endswith(tsunami.MESSAGE_EXTENSION):
            show_agent_password = self._get_agent_password_from_path(full_show_file)
            if show_agent_password is None:
                self.perror("Agent's password is not defined")
                return
            show_message_encoded = ""
            with open(full_show_file, "rb") as input_fd:
                show_message_encoded = input_fd.read()
            show_message_decoded = tsunami.DecodeDecryptDecompress(show_message_encoded, show_agent_password.encode())
            show_main_options = json.loads(show_message_decoded)
            tsunami.print_json(show_main_options, 2)
        elif os.path.basename(full_show_file).endswith(".json"):
            with open(full_show_file, "rb") as input_fd:
                show_message = input_fd.read()
            show_main_options = json.loads(show_message)
            tsunami.print_json(show_main_options, 2)
        else:
            self.perror("Unknown file type to show")
            return

    def complete_show(self, text, line, begidx, endidx):
        return self.path_complete(text, line, begidx, endidx, path_filter=_complete_file_with_ignore)

    argparser_use = argparse.ArgumentParser(
        description="Use the specified submodule.  You can use a command or installer.",
        epilog="Example:\n\tuse survey_host_request")
    argparser_use.add_argument("use_submodule", help="The submodule to use")

    @with_argparser(argparser_use)
    def do_use(self, use_args):
        use_secondary_config_file = use_args.use_submodule
        if not os.path.isfile(use_secondary_config_file):
            use_secondary_config_file = use_secondary_config_file + tsunami.CONFIG_EXTENSION
        if not os.path.isfile(use_secondary_config_file):
            self.perror("Used unknown module: '{:s}'".format(use_args.use_submodule))
            return
        # Determine the primary module used from the path of the submodule
        use_primary_config_file = os.path.split(use_args.use_submodule)[0]
        self.primary_module_name = os.path.split(use_primary_config_file)[1]
        use_primary_config_file = os.path.join(use_primary_config_file, tsunami.BASELINE_CONFIG)
        # Load primary module settings
        if os.path.isfile(use_primary_config_file):
            config_file_contents = ''
            with open(use_primary_config_file, "r") as input_fd:
                config_file_contents = input_fd.read()
            self.primary_options = json.loads(config_file_contents)
            self.original_primary_options = self.primary_options.copy()
        # Set the submodule name
        self.secondary_module_name = os.path.splitext(os.path.basename(use_secondary_config_file))[0]
        # Load secondary module settings
        config_file_contents = ''
        # self.pfeedback("Opening secondary module options: {:s}".format(use_secondary_config_file))
        with open(use_secondary_config_file, "r") as input_fd:
            config_file_contents = input_fd.read()
        self.secondary_options = json.loads(config_file_contents)
        self.original_secondary_options = self.secondary_options.copy()
        self.primary_options[self.primary_module_name] = self.secondary_module_name

    def complete_use(self, text, line, begidx, endidx):
        return self.path_complete(text, line, begidx, endidx, path_filter=_complete_file_with_ignore)

    argparser_exit = argparse.ArgumentParser(
        description="Exit the TSunami command-line interpreter",
        epilog="Example:\n\texit")

    @with_argparser(argparser_exit)
    def do_exit(self, exit_args):
        print("\nTSunami waving good bye\n")
        return True

    do_EOF = do_exit

    def default(self, statement):
        if statement.command.startswith("--"):
            new_command = statement.command[2:] + " True"
            #print("new_command:", new_command)
            self.do_set(new_command)
        else:
            self.perror("'{:s}' is an unrecognized command, alias, or macro (try 'help')".format(statement.command))
        return


    def _interactive_thread_func(self, script_file, agent_dir) -> None:
        interactive_subprocess = None
        if os.name == 'nt':
            # console = ['cmd.exe', '/c', 'start'] # or something
            # cmd = ['python', script_file, agent_dir]
            popen_args = ['cmd.exe', '/c', 'start', 'python', script_file, agent_dir]
        else:
            cmd = ['python3', script_file, agent_dir]
            console = shutil.which("gnome-terminal")
            if console != None:
                # console = [ console, '--' ]
                popen_args = [ console, '--' ] + cmd
            else:
                console = shutil.which("konsole")
                if console != None:
                    #console = [ console, '-x' ]
                    popen_args = [console, '-x'] + cmd
                else:
                    console = shutil.which("xterm")
                    if console != None:
                        #console = [console, '-e'] # specify your favorite terminal here
                        popen_args = [console, '-e'] + cmd
                    else:
                        self.perror("Unknown terminal emulator")
                        return

        self.pfeedback("Executing interactive_thread with {:s}".format(' '.join(popen_args)))
        time.sleep(2)
        interactive_subprocess = subprocess.Popen(popen_args)
        if interactive_subprocess:
            interactive_subprocess.wait()


if __name__ == "__main__":
    app = TsunamiPrompt()
    app.set_window_title("Tsunami Warning")
    sys.exit(app.cmdloop())
