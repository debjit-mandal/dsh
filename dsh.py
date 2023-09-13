#!/usr/bin/env python3

import subprocess
import readline
import os
import shlex
import signal
import json
import logging

CONFIG_FILE = os.path.expanduser("~/.dsh_config.json")
HISTORY_FILE = os.path.expanduser("~/.dsh_history")
LOG_FILE = os.path.expanduser("~/.dsh_log.txt")

# Initialize logging
logging.basicConfig(filename=LOG_FILE, level=logging.INFO)

class dsh:

    def __init__(self):
        self.load_config()
        self.setup_readline()
        self.setup_signals()

    def load_config(self):
        default_config = {
            "aliases": {
                "ls": "ls --color=auto",
                "ll": "ls -la"
            },
            "history_length": 1000
        }

        if os.path.exists(CONFIG_FILE):
            with open(CONFIG_FILE, 'r') as f:
                self.config = json.load(f)
        else:
            self.config = default_config
            with open(CONFIG_FILE, 'w') as f:
                json.dump(default_config, f, indent=4)

        self.aliases = self.config["aliases"]
        readline.set_history_length(self.config["history_length"])

    def setup_readline(self):
        readline.parse_and_bind("tab: complete")
        if os.path.exists(HISTORY_FILE):
            readline.read_history_file(HISTORY_FILE)
        readline.set_completer(self.complete)

    def setup_signals(self):
        signal.signal(signal.SIGINT, self.sigint_handler)

    def complete(self, text, state):
        builtins = ['cd', 'help', 'exit', 'quit']
        return ([
            cmd for cmd in builtins if cmd.startswith(text)
        ] + [filename for filename in os.listdir() if filename.startswith(text)])[state]

    def execute_command(self, command):
        cmd_list = shlex.split(command)
        cmd = self.aliases.get(cmd_list[0], cmd_list[0])
        cmd_list[0] = cmd

        if cmd_list[0] == "cd":
            self.handle_cd_command(cmd_list)
        elif cmd_list[0] == "help":
            self.display_help()
        else:
            self.run_system_command(cmd_list)

    def handle_cd_command(self, cmd_list):
        try:
            path = cmd_list[1] if len(cmd_list) > 1 else os.getenv("HOME")
            os.chdir(path)
        except OSError as e:
            error_message = f"Error changing directory: {e}"
            print(error_message)
            logging.error(error_message)

    def display_help(self):
        help_text = """
    dsh Built-in Commands:
    cd [directory]   - Change directory.
    help             - Display this help message.
    exit / quit      - Exit the shell.
    """
        print(help_text)

    def run_system_command(self, cmd_list):
        try:
            result = subprocess.run(cmd_list, capture_output=True, text=True)
            if result.stdout:
                print(result.stdout, end="")
            if result.stderr:
                print(result.stderr, end="")
                logging.error(result.stderr)
        except FileNotFoundError:
            error_message = f"Command not found: {cmd_list[0]}"
            print(error_message)
            logging.error(error_message)
        except Exception as e:
            error_message = f"Error executing command: {e}"
            print(error_message)
            logging.error(error_message)

    def sigint_handler(self, signal, frame):
        print("\n^C")
        print(self.prompt(), end="")
        readline.redisplay()

    def prompt(self):
        return f"{os.getcwd()} > "

    def loop(self):
        while True:
            try:
                command = input(self.prompt()).strip()
                if not command:
                    continue
                if command in ['exit', 'quit']:
                    readline.write_history_file(HISTORY_FILE)
                    break
                logging.info(f"Executed command: {command}")
                self.execute_command(command)
            except EOFError:  
                readline.write_history_file(HISTORY_FILE)
                break

if __name__ == "__main__":
    shell = dsh()
    shell.loop()
