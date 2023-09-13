# dsh
A Command-Line Shell made using Python

To run it locally-

1. Fork this repository
   
2. `git clone https://github.com/debjit-mandal/dsh`

3. `cd dsh`

4. `chmod +x dsh.py`

5. `./dsh.py`

Create a `~/.dsh_config.json` file and add the necessary details and aliases:
```
{
        "aliases": {
            "ls": "ls --color=auto",
            "ll": "ls -l",
            "la": "ls -la"
        },
        "history_length":1000
}
```
You can add more aliases and configure it according to yourself.
Feel free to suggest any kind of improvements
