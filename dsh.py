#!/usr/bin/env python3

import os
import readline

def dsh():
    prompt = 'Dsh> '
    while True:
        try:
            # Read input from the user
            command = input(prompt)

            # Exit the shell if the command is 'exit' or 'quit'
            if command in ['exit', 'quit']:
                break

            # Execute the command
            os.system(command)

        except KeyboardInterrupt:
            # Handle Ctrl+C
            print('^C')
        except EOFError:
            # Handle Ctrl+D
            break

if __name__ == "__main__":
    dsh()