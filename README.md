# dsh
A Command-Line Shell made using C++

**To run it locally:**

1. Fork this repository
   
2. `git clone https://github.com/debjit-mandal/dsh`

3. `cd dsh`

4. `g++ -o dsh dsh.cpp -lreadline`

5. `./dsh`

## Available Commands:

Here is a list of commands supported by DSH along with their brief descriptions:

- **`awk`**: A program for pattern scanning and processing.
- **`bash`**: Executes a bash script or command.
- **`cat`**: Displays the content of a file.
- **`cd`**: Changes the current directory.
- **`chgrp`**: Changes the group ownership of a file.
- **`chmod`**: Changes file permissions.
- **`chown`**: Changes file owner and group.
- **`cp`**: Copies a file from one location to another.
- **`cron`**: Manages cron jobs.
- **`date`**: Displays or sets the system date and time.
- **`df`**: Reports disk space usage.
- **`diff`**: Compares files line by line.
- **`du`**: Analyzes disk space usage.
- **`echo`**: Echoes text to the terminal.
- **`env`**: Displays, sets, or gets environment variables.
- **`envlist`**: Lists all environment variables.
- **`exec`**: Executes scripts or other programs.
- **`find`**: Searches for files matching a pattern.
- **`free`**: Displays the amount of free and used memory in the system.
- **`g++`**: Compiles C++ source files.
- **`git`**: Executes Git commands for version control.
- **`grep`**: Searches for a text pattern within a file.
- **`gzip`**: Compresses or decompresses files using gzip.
- **`hexdump`**: Displays file content in hexadecimal format.
- **`http`**: Starts a simple HTTP server.
- **`htop`**: Provides detailed system performance information.
- **`ifconfig`**: Lists all network interface configurations.
- **`ifstat`**: Displays network interface statistics.
- **`init`**: Changes the runlevel of the system.
- **`inotify`**: Watches file system changes in real time.
- **`iptables`**: Administrates IP packet filter rules.
- **`kill`**: Sends a signal to a process.
- **`last`**: Shows a list of last logged in users.
- **`less`**: Views file contents interactively.
- **`ln`**: Creates a symbolic link.
- **`login`**: Logs in as a specified user.
- **`ls`**: Lists files in the current or specified directory.
- **`ll`**: Lists all files in detail in the current or specified directory.
- **`man`**: Displays user manual of any command.
- **`mkdir`**: Creates a new directory.
- **`mount`**: Mounts filesystems.
- **`mv`**: Moves or renames a file or directory.
- **`mysql`**: Executes MySQL commands.
- **`nano`**: Opens a file in the Nano text editor.
- **`nmap`**: Network exploration tool and security scanner.
- **`netstat`**: Shows network statistics.
- **`ps`**: Displays currently running processes.
- **`psaux`**: Detailed view of currently running processes.
- **`pwd`**: Prints the current directory.
- **`python`**: Executes Python scripts or commands.
- **`play`**: Plays audio files from the command line.
- **`rm`**: Deletes a specified file.
- **`rsync`**: Syncs files and directories between two locations.
- **`screen`**: Starts a screen session for managing multiple terminal sessions.
- **`sed`**: Performs text transformations.
- **`service`**: Manages system services.
- **`shutdown`**: Shuts down or reboots the system.
- **`sql`**: Executes SQL commands or scripts.
- **`ssh`**: Connects to a host via Secure Shell.
- **`sort`**: Sorts the contents of a file.
- **`sysinfo`**: Displays system information.
- **`tar`**: Manages archives for backup and restoration.
- **`tail`**: Follows the tail of a file (real-time update).
- **`tcpdump`**: Command-line packet analyzer.
- **`touch`**: Updates the access and modification times of a file.
- **`traceroute`**: Traces the route packets take to a network host.
- **`top`**: Displays real-time system resource usage.
- **`umount`**: Unmounts filesystems.
- **`uname`**: Prints system information.
- **`uniq`**: Filters or reports repeated lines in a file.
- **`uptime`**: Displays how long the system has been running.
- **`vim`**: Opens a file in Vim editor.
- **`wc`**: Counts lines, words, and characters in a file.
- **`watch`**: Executes a command repeatedly, displaying the output.
- **`wget`**: Downloads files from the internet.
- **`who`**: Displays who is logged on.

---

DSH can be customized by using a configuration file **.dshrc** which can be loaded at the start of each DSH session to configure environment settings, define aliases, set variables, customize the prompt, and more.

## Example `.dshrc` Configuration:

Here’s a basic .dshrc file with comments explaining each part:

```
# Set environment variables
setenv PATH /usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin
setenv EDITOR vim

# Aliases (needs implementation in the shell)
alias ll "ls -l"
alias la "ls -a"
alias l "ls -CF"

# Custom commands
echo Welcome to DSH!
```

---

Feel free to suggest any kind of improvements
