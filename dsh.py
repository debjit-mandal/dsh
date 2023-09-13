#!/usr/bin/env python3

import subprocess
import readline
import os
import shlex
import signal
import json
import logging
from pathlib import Path

CONFIG_PATH = Path("~/.dsh_config.json").expanduser()
HISTORY_PATH = Path("~/.dsh_history").expanduser()
LOG_PATH = Path("~/.dsh_log.txt").expanduser()

logging.basicConfig(filename=LOG_PATH, level=logging.INFO)

class CommandContext:
    def __init__(self, env):
        self.env = env

class Command:
    registry = {}

    @classmethod
    def register(cls, name):
        def wrapper(subclass):
            cls.registry[name] = subclass()
            return subclass
        return wrapper

    def execute(self, args, context):
        raise NotImplementedError

    @classmethod
    def help_text(cls):
        return cls.__doc__

@Command.register('setenv')
class SetEnvCommand(Command):
    """Set an environment variable: setenv VAR_NAME VAR_VALUE"""
    
    def execute(self, args, context):
        context.env[args[1]] = args[2]

@Command.register('getenv')
class GetEnvCommand(Command):
    """Get the value of an environment variable: getenv VAR_NAME"""
    
    def execute(self, args, context):
        print(context.env.get(args[1], "Variable not set."))

class CommandParser:
    @staticmethod
    def parse(command):
        return shlex.split(command)

class Configuration:
    def __init__(self):
        self.config = self.load_config()

    def load_config(self):
        if CONFIG_PATH.exists():
            with open(CONFIG_PATH, 'r') as file:
                return json.load(file)
        return {"aliases": {}}

    def get(self, key, default=None):
        return self.config.get(key, default)

class dsh:
    def __init__(self):
        self.config = Configuration()
        self.setup_readline()
        self.setup_signals()

    def setup_readline(self):
        if HISTORY_PATH.exists():
            readline.read_history_file(HISTORY_PATH)
        history_length = self.config.get("history_length", 1000)
        readline.set_history_length(history_length)

    def setup_signals(self):
        signal.signal(signal.SIGINT, self.signal_handler)

    def signal_handler(self, sig, frame):
        print("\nInterrupted!")
        
    def handle_aliases(self, cmd_list):
        cmd = cmd_list[0]
        alias_cmd = self.config.get("aliases").get(cmd, cmd)
        return shlex.split(alias_cmd) + cmd_list[1:]

    def execute_command(self, command):
        cmd_list = CommandParser.parse(command)
        cmd_list = self.handle_aliases(cmd_list)
        cmd = cmd_list[0]
        context = CommandContext(os.environ)

        if cmd in Command.registry:
            try:
                Command.registry[cmd].execute(cmd_list, context)
            except Exception as e:
                logging.error(f"Error in command {cmd}: {e}")
                print(f"Command error: {e}")
        else:
            self.run_system_command(cmd_list)

    def run_system_command(self, cmd_list):
        try:
            subprocess.run(cmd_list)
        except Exception as e:
            logging.error(f"Error executing system command {cmd_list[0]}: {e}")
            print(f"Error executing command: {e}")

    def loop(self):
        while True:
            try:
                command = input(f"{os.getcwd()} $ ")
                if command.strip():
                    self.execute_command(command)
            except EOFError:
                print("\nExiting...")
                break
        readline.write_history_file(HISTORY_PATH)

if __name__ == "__main__":
    shell = dsh()
    shell.loop()
