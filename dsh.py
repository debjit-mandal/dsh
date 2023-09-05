#!/usr/bin/env python3

import subprocess
import readline
import os
import shlex


def execute_command(command):

    try:
        cmd_list = shlex.split(command)

        if cmd_list[0] == "cd":
            try:
                path = cmd_list[1] if len(
                    cmd_list) > 1 else os.path.expanduser("~")
                os.chdir(path)
            except OSError as e:
                print(f"Error changing directory: {e}")
            return

        result = subprocess.run(cmd_list, capture_output=True, text=True)
        if result.stdout:
            print(result.stdout, end="")
        if result.stderr:
            print(result.stderr, end="")
    except FileNotFoundError:
        print(f"Command not found: {cmd_list[0]}")
    except Exception as e:
        print(f"Error executing command: {e}")


def dsh():
    while True:
        try:
            prompt = f"{os.getcwd()} > "
            command = input(prompt).strip()

            if not command:
                continue

            if command in ['exit', 'quit']:
                break

            execute_command(command)

        except KeyboardInterrupt:
            print('^C')
        except EOFError:
            break


if __name__ == "__main__":
    dsh()
