#!/usr/bin/env python3

import subprocess
import readline

def execute_command(command):

    try:
        result = subprocess.run(command.split(), capture_output=True, text=True)
        if result.stdout:
            print(result.stdout, end="")
        if result.stderr:
            print(result.stderr, end="")
    except FileNotFoundError:
        print(f"Command not found: {command.split()[0]}")
    except Exception as e:
        print(f"Error executing command: {e}")

def dsh():
    prompt = 'Dsh> '
    while True:
        try:
            command = input(prompt).strip()

            if not command:
                continue

            if command in ['exit', 'quit']:
                break

            execute_command(command)

        except KeyboardInterrupt:
            # Handle Ctrl+C
            print('^C')
        except EOFError:
            # Handle Ctrl+D
            break

if __name__ == "__main__":
    dsh()