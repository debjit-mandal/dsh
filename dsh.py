#!/usr/bin/env python3

import os
import readline

def dsh():
    prompt = 'Dsh> '
    while True:
        try:
            command = input(prompt)

            if command in ['exit', 'quit']:
                break

            os.system(command)

        except KeyboardInterrupt:

            print('^C')
        except EOFError:
            break

if __name__ == "__main__":
    dsh()