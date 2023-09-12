#!/usr/bin/env python3

import subprocess
import readline
import os
import shlex
import signal

HISTORY_FILE = os.path.expanduser("~/.customshell_history")
HISTORY_LENGTH = 1000

def setup_readline():
    readline.parse_and_bind("tab: complete")
    
    if os.path.exists(HISTORY_FILE):
        readline.read_history_file(HISTORY_FILE)

    readline.set_history_length(HISTORY_LENGTH)

def complete(text, state):
    builtins = ['cd', 'help', 'exit', 'quit']
    return ([
        cmd for cmd in builtins if cmd.startswith(text)
    ] + [filename for filename in os.listdir() if filename.startswith(text)])[state]

def execute_command(command):
    cmd_list = shlex.split(command)

    if cmd_list[0] == "cd":
        handle_cd_command(cmd_list)
    elif cmd_list[0] == "help":
        display_help()
    else:
        run_system_command(cmd_list)

def handle_cd_command(cmd_list):
    try:
        path = cmd_list[1] if len(cmd_list) > 1 else os.path.expanduser("~")
        os.chdir(path)
    except OSError as e:
        print(f"Error changing directory: {e}")

def display_help():
    help_text = """
    CustomShell Built-in Commands:
    cd [directory]   - Change directory.
    help             - Display this help message.
    exit / quit      - Exit the shell.
    """
    print(help_text)

def run_system_command(cmd_list):
    try:
        result = subprocess.run(cmd_list, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout, end="")
        if result.stderr:
            print(result.stderr, end="")
    except FileNotFoundError:
        print(f"Command not found: {cmd_list[0]}")
    except Exception as e:
        print(f"Error executing command: {e}")

def sigint_handler(signal, frame):
    print("\n^C")
    print(f"{os.getcwd()} > ", end="")
    readline.redisplay()

def custom_shell():
    setup_readline()
    readline.set_completer(complete)
    signal.signal(signal.SIGINT, sigint_handler)

    while True:
        try:
            prompt = f"{os.getcwd()} > "
            command = input(prompt).strip()
            if not command:
                continue
            if command in ['exit', 'quit']:
                readline.write_history_file(HISTORY_FILE)
                break
            execute_command(command)
        except EOFError:  
            readline.write_history_file(HISTORY_FILE)
            break

if __name__ == "__main__":
    custom_shell()

