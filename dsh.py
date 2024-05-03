#!/usr/bin/env python3

import json
import logging
import os
import readline
import shlex
import signal
import subprocess
from pathlib import Path

CONFIG_PATH = Path("~/.dsh_config.json").expanduser()
HISTORY_PATH = Path("~/.dsh_history").expanduser()
LOG_PATH = Path("~/.dsh_log.txt").expanduser()

logging.basicConfig(
    filename=LOG_PATH,
    level=logging.INFO,
    format="%(asctime)s %(levelname)s %(message)s",
)


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
        try:
            if args:
                return self.run(args, context)
            else:
                print("No arguments provided. Usage:")
                print(self.help_text())
        except Exception as e:
            print(f"Error executing command: {str(e)}")

    def run(self, args, context):
        raise NotImplementedError

    @classmethod
    def help_text(cls):
        return cls.__doc__

# Help Command


@Command.register("help")
class HelpCommand(Command):
    """Display information about available commands. Usage: help [command_name]"""

    def run(self, args, context):
        if len(args) > 1:
            command = args[1]
            if command in Command.registry:
                print(Command.registry[command].help_text())
            else:
                print(f"No help available for {command}")
        else:
            print("Available commands:")
            for name, cmd in Command.registry.items():
                print(f"{name} - {cmd.help_text().split('.')[0]}")

# Example SetEnvCommand with detailed help


@Command.register("setenv")
class SetEnvCommand(Command):
    """Set an environment variable. Usage: setenv [variable] [value]"""

    def run(self, args, context):
        if len(args) < 3:
            print("Insufficient arguments. Usage: setenv [variable] [value]")
        else:
            context.env[args[1]] = args[2]
            print(f"Environment variable {args[1]} set to {args[2]}")

# Signal handling and other foundational aspects remain unchanged


@Command.register("getenv")
class GetEnvCommand(Command):

    def execute(self, args, context):
        print(context.env.get(args[1], "Variable not set."))


@Command.register("cd")
class ChangeDirectoryCommand(Command):

    def execute(self, args, context):
        os.chdir(args[1])


@Command.register("help")
class HelpCommand(Command):

    def execute(self, args, context):
        for cmd, cmd_obj in Command.registry.items():
            print(f"{cmd}: {cmd_obj.help_text()}")


@Command.register("reload_config")
class ReloadConfigCommand(Command):

    def execute(self, args, context):
        global config
        config = Configuration()
        print("Configuration reloaded.")


@Command.register("exit")
class ExitCommand(Command):

    def execute(self, args, context):
        print("Exiting...")
        raise SystemExit


class CommandParser:
    @staticmethod
    def parse(command):
        return shlex.split(command)


class Configuration:
    def __init__(self):
        self.config = self.load_config()

    def load_config(self):
        if CONFIG_PATH.exists():
            with open(CONFIG_PATH, "r") as file:
                return json.load(file)
        return {"aliases": {}}

    def get(self, key, default=None):
        return self.config.get(key, default)


config = Configuration()


class dsh:
    def __init__(self):
        self.setup_readline()
        self.setup_signals()

    def setup_readline(self):
        if HISTORY_PATH.exists():
            readline.read_history_file(HISTORY_PATH)
        history_length = config.get("history_length", 1000)
        readline.set_history_length(history_length)
        readline.set_completer(self.complete)
        readline.parse_and_bind("tab: complete")

    def complete(self, text, state):
        options = [i for i in Command.registry.keys() if i.startswith(text)]
        if state < len(options):
            return options[state]
        else:
            files = [f for f in os.listdir(".") if f.startswith(text)]
            if state < len(options) + len(files):
                return files[state - len(options)]
        return None

    def setup_signals(self):
        signal.signal(signal.SIGINT, self.signal_handler)

    def signal_handler(self, sig, frame):
        print("\nInterrupted!")

    def handle_aliases(self, cmd_list):
        cmd = cmd_list[0]
        alias_cmd = config.get("aliases").get(cmd, cmd)
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
