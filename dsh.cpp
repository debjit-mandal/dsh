#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <algorithm>
#include <cstring>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <string>
#include <set>
#include <bits/stdc++.h>

class Command
{
public:
    virtual void execute(const std::vector<std::string> &args) = 0;
    virtual std::string helpText() = 0;
};

class CommandRegistry
{
private:
    std::map<std::string, Command *> commands;
    std::map<std::string, std::string> aliases;

public:
    ~CommandRegistry()
    {
        for (auto cmd : commands)
        {
            delete cmd.second;
        }
    }
    void registerCommand(const std::string &commandName, Command *command)
    {
        commands[commandName] = command;
    }
    Command *getCommand(const std::string &commandName)
    {
        if (aliases.count(commandName)) {
            return commands[aliases[commandName]];
        }
        auto it = commands.find(commandName);
        if (it != commands.end())
        {
            return it->second;
        }
        return nullptr;
    }
    void listCommands()
    {
        std::cout << "Available commands:\n";
        for (auto &command : commands)
        {
            std::cout << command.first << " - " << command.second->helpText() << "\n";
        }
    }
    void registerAlias(const std::string& aliasName, const std::string& commandName) {
        aliases[aliasName] = commandName;
    }
};

void loadDshrc(const std::string& path, CommandRegistry& registry) {
    std::ifstream file(path);
    std::string line;
    while (getline(file, line)) {
        std::istringstream iss(line);
        std::vector<std::string> tokens(std::istream_iterator<std::string>{iss}, {});
        if (!tokens.empty()) {
            if (tokens[0] == "alias" && tokens.size() >= 3) {
                std::string alias = tokens[1];
                std::string actualCommand = line.substr(line.find("\"") + 1, line.rfind("\"") - line.find("\"") - 1);
                registry.registerAlias(alias, actualCommand);
            } else {
                Command* cmd = registry.getCommand(tokens[0]);
                if (cmd) {
                    cmd->execute(tokens);
                } else {
                    std::cout << "Unknown command or alias in .dshrc: " << tokens[0] << "\n";
                }
            }
        }
    }
}


class ListFilesCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        const char *directory = (args.size() > 1) ? args[1].c_str() : ".";
        DIR *dir;
        struct dirent *ent;
        if ((dir = opendir(directory)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                std::cout << ent->d_name << "\n";
            }
            closedir(dir);
        }
        else
        {
            perror("Unable to list directory");
        }
    }
    std::string helpText() override
    {
        return "Lists files in the current or specified directory. Usage: ls [directory]";
    }
};

class ListFilesDetailCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        const char *directory = ".";
        if (args.size() > 1)
            directory = args[1].c_str();

        DIR *dir;
        struct dirent *ent;
        struct stat st;

        if ((dir = opendir(directory)) != NULL)
        {
            while ((ent = readdir(dir)) != NULL)
            {
                std::string fullPath = std::string(directory) + "/" + std::string(ent->d_name);
                if (stat(fullPath.c_str(), &st) == -1)
                {
                    continue;
                }

                std::cout << (S_ISDIR(st.st_mode) ? 'd' : '-')
                          << ((st.st_mode & S_IRUSR) ? 'r' : '-')
                          << ((st.st_mode & S_IWUSR) ? 'w' : '-')
                          << ((st.st_mode & S_IXUSR) ? 'x' : '-')
                          << ((st.st_mode & S_IRGRP) ? 'r' : '-')
                          << ((st.st_mode & S_IWGRP) ? 'w' : '-')
                          << ((st.st_mode & S_IXGRP) ? 'x' : '-')
                          << ((st.st_mode & S_IROTH) ? 'r' : '-')
                          << ((st.st_mode & S_IWOTH) ? 'w' : '-')
                          << ((st.st_mode & S_IXOTH) ? 'x' : '-')
                          << ' ' << st.st_nlink
                          << ' ' << getpwuid(st.st_uid)->pw_name
                          << ' ' << getgrgid(st.st_gid)->gr_name
                          << ' ' << st.st_size
                          << ' ' << ctime(&st.st_mtime)
                          << ent->d_name << "\n";
            }
            closedir(dir);
        }
        else
        {
            perror("Unable to list directory");
        }
    }
    std::string helpText() override
    {
        return "Lists all files in detail. Usage: lsal [directory]";
    }
};

class ChangeDirectoryCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Insufficient arguments. Usage: cd [directory]\n";
        }
        else if (chdir(args[1].c_str()) != 0)
        {
            perror("cd failed");
        }
    }
    std::string helpText() override
    {
        return "Change the current directory. Usage: cd [directory]";
    }
};

class PrintWorkingDirectoryCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            std::cout << cwd << "\n";
        }
        else
        {
            perror("pwd failed");
        }
    }
    std::string helpText() override
    {
        return "Prints the current directory. Usage: pwd";
    }
};

class HelpCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() == 1)
        {
            std::cout << "Display information about available commands. Usage: help [command_name]\n";
        }
        else
        {
            std::cout << "No help available for \"" << args[1] << "\"" << std::endl;
        }
    }
    std::string helpText() override
    {
        return "Display information about available commands. Usage: help [command_name]";
    }
};

class CopyFileCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: cp [source] [destination]\n";
            return;
        }
        std::ifstream src(args[1], std::ios::binary);
        std::ofstream dst(args[2], std::ios::binary);
        dst << src.rdbuf();
        src.close();
        dst.close();
    }
    std::string helpText() override
    {
        return "Copies a file. Usage: cp [source] [destination]";
    }
};

class DeleteFileCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: rm [file]\n";
            return;
        }
        if (remove(args[1].c_str()) != 0)
        {
            perror("Error deleting file");
        }
        else
        {
            std::cout << "File deleted successfully\n";
        }
    }
    std::string helpText() override
    {
        return "Deletes a file. Usage: rm [file]";
    }
};

class EchoCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        for (size_t i = 1; i < args.size(); ++i)
        {
            std::cout << args[i] << " ";
        }
        std::cout << "\n";
    }
    std::string helpText() override
    {
        return "Echoes text to the terminal. Usage: echo [text]";
    }
};

class MakeDirectoryCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: mkdir [directory]\n";
            return;
        }
        if (mkdir(args[1].c_str(), 0777) != 0)
        {
            perror("Error creating directory");
        }
        else
        {
            std::cout << "Directory created\n";
        }
    }
    std::string helpText() override
    {
        return "Creates a directory. Usage: mkdir [directory]";
    }
};

class MoveFileCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: mv [source] [destination]\n";
            return;
        }
        if (rename(args[1].c_str(), args[2].c_str()) != 0)
        {
            perror("Error moving file");
        }
        else
        {
            std::cout << "File moved\n";
        }
    }
    std::string helpText() override
    {
        return "Moves a file. Usage: mv [source] [destination]";
    }
};

class SystemInfoCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("uname -a"); // Simple call to 'uname -a', adjust as needed for more info
    }
    std::string helpText() override
    {
        return "Displays system information. Usage: sysinfo";
    }
};

class SetEnvCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Insufficient arguments. Usage: setenv [variable] [value]\n";
        }
        else
        {
            std::cout << "Setting environment variable " << args[1] << " to " << args[2] << "\n";
            setenv(args[1].c_str(), args[2].c_str(), 1);
        }
    }
    std::string helpText() override
    {
        return "Set an environment variable. Usage: setenv [variable] [value]";
    }
};

class CatCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: cat [file]\n";
            return;
        }
        std::ifstream file(args[1]);
        std::string line;
        if (file.is_open())
        {
            while (getline(file, line))
            {
                std::cout << line << "\n";
            }
            file.close();
        }
        else
        {
            std::cout << "Unable to open file\n";
        }
    }
    std::string helpText() override
    {
        return "Displays the content of a file. Usage: cat [file]";
    }
};

class GrepCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: grep [pattern] [file]\n";
            return;
        }
        std::ifstream file(args[2]);
        std::string line;
        if (file.is_open())
        {
            while (getline(file, line))
            {
                if (line.find(args[1]) != std::string::npos)
                {
                    std::cout << line << "\n";
                }
            }
            file.close();
        }
        else
        {
            std::cout << "Unable to open file\n";
        }
    }
    std::string helpText() override
    {
        return "Searches for a text pattern within a file. Usage: grep [pattern] [file]";
    }
};

class TopCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("top -b -n 1"); // Runs 'top' command in batch mode for a single iteration
    }
    std::string helpText() override
    {
        return "Displays real-time system resource usage. Usage: top";
    }
};

class IfconfigCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("ifconfig"); // Assumes 'ifconfig' is installed, consider 'ip addr' on modern systems
    }
    std::string helpText() override
    {
        return "Lists all network interface configurations. Usage: ifconfig";
    }
};

class FindCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: find [directory] [pattern]\n";
            return;
        }
        std::string command = "find " + args[1] + " -name \"" + args[2] + "\"";
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Search for files matching a pattern. Usage: find [directory] [pattern]";
    }
};

class WgetCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: wget [url]\n";
            return;
        }
        std::string command = "wget " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Download files from the internet. Usage: wget [url]";
    }
};

class HexDumpCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: hexdump [file]\n";
            return;
        }
        std::string command = "hexdump -C " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Display file content in hexadecimal format. Usage: hexdump [file]";
    }
};

class PsCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("ps aux");
    }
    std::string helpText() override
    {
        return "Display currently running processes. Usage: ps";
    }
};

class NetstatCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("netstat -tuln");
    }
    std::string helpText() override
    {
        return "Show network statistics. Usage: netstat";
    }
};

class ShutdownCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() > 1 && args[1] == "reboot")
        {
            system("reboot");
        }
        else
        {
            system("shutdown now");
        }
    }
    std::string helpText() override
    {
        return "Shut down or reboot the system. Usage: shutdown [reboot]";
    }
};

class TailCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: tail [file]\n";
            return;
        }
        std::string command = "tail -f " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Follows the tail of a file. Usage: tail [file]";
    }
};

class NanoCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: nano [file]\n";
            return;
        }
        std::string command = "nano " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Open a file in the Nano editor. Usage: nano [file]";
    }
};

class HttpCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string port = "8000"; // Default port
        if (args.size() > 1)
        {
            port = args[1];
        }
        std::string command = "python -m http.server " + port;
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Starts a simple HTTP server. Usage: http [port]";
    }
};

class ChmodCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: chmod [permissions] [file]\n";
            return;
        }
        std::string command = "chmod " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Changes file permissions. Usage: chmod [permissions] [file]";
    }
};

class ChownCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: chown [owner][:group] [file]\n";
            return;
        }
        std::string command = "chown " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Changes file owner and group. Usage: chown [owner][:group] [file]";
    }
};

class SortCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: sort [file]\n";
            return;
        }
        std::string command = "sort " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Sorts the contents of a file. Usage: sort [file]";
    }
};

class UniqCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: uniq [file]\n";
            return;
        }
        std::string command = "uniq " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Filters or reports repeated lines in a file. Usage: uniq [file]";
    }
};

class WcCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: wc [file]\n";
            return;
        }
        std::string command = "wc " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Counts lines, words, and characters in a file. Usage: wc [file]";
    }
};

class DfCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("df -h");
    }
    std::string helpText() override
    {
        return "Reports disk space usage. Usage: df";
    }
};

class PingCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: ping [host]\n";
            return;
        }
        std::string command = "ping -c 4 " + args[1]; // Ping 4 times by default
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Checks network connectivity to a host. Usage: ping [host]";
    }
};

class EnvCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() == 1)
        {
            system("printenv");
        }
        else if (args.size() == 3 && args[1] == "set")
        {
            setenv(args[2].c_str(), args[3].c_str(), 1);
        }
        else if (args.size() == 2)
        {
            std::string value = getenv(args[1].c_str()) ? getenv(args[1].c_str()) : "Not set";
            std::cout << args[1] << "=" << value << "\n";
        }
        else
        {
            std::cout << "Usage: env [set var value | var]\n";
        }
    }
    std::string helpText() override
    {
        return "Displays, sets, or gets environment variables. Usage: env [set var value | var]";
    }
};

class LnCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: ln [target] [linkname]\n";
            return;
        }
        std::string command = "ln -s " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Creates a symbolic link. Usage: ln [target] [linkname]";
    }
};

class ChgrpCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: chgrp [group] [file]\n";
            return;
        }
        std::string command = "chgrp " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Changes the group ownership of a file. Usage: chgrp [group] [file]";
    }
};

class UptimeCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("uptime");
    }
    std::string helpText() override
    {
        return "Displays how long the system has been running. Usage: uptime";
    }
};

class FreeCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("free -h");
    }
    std::string helpText() override
    {
        return "Displays the amount of free and used memory in the system. Usage: free";
    }
};

class WhoCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("who");
    }
    std::string helpText() override
    {
        return "Displays who is logged on. Usage: who";
    }
};

class TracerouteCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: traceroute [host]\n";
            return;
        }
        std::string command = "traceroute " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Traces the route packets take to a network host. Usage: traceroute [host]";
    }
};

class BashCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "bash ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes a bash script or command. Usage: bash [command]";
    }
};

class GzipCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: gzip [option] [file]\n";
            return;
        }
        std::string command = "gzip " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Compress or decompress files using gzip. Usage: gzip [option] [file]";
    }
};

class KillCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: kill [pid]\n";
            return;
        }
        std::string command = "kill " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Send a signal to a process. Usage: kill [pid]";
    }
};

class AwkCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "awk ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Program for pattern scanning and processing. Usage: awk [program] [file...]";
    }
};

class UnameCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "uname ";
        if (args.size() > 1)
        {
            command += args[1];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Prints system information. Usage: uname [option]";
    }
};

class LessCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: less [file]\n";
            return;
        }
        std::string command = "less " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "View file contents interactively. Usage: less [file]";
    }
};

class DateCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() == 1)
        {
            system("date");
        }
        else if (args.size() == 2)
        {
            std::string command = "date -s \"" + args[1] + "\"";
            system(command.c_str());
        }
        else
        {
            std::cout << "Usage: date [\"new date and time\"]\n";
        }
    }
    std::string helpText() override
    {
        return "Displays or sets the system date and time. Usage: date [\"YYYY-MM-DD HH:MM:SS\"]";
    }
};

class MountCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: mount [source] [target]\n";
            return;
        }
        std::string command = "mount " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Mounts filesystems. Usage: mount [source] [target]";
    }
};

class UmountCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: umount [target]\n";
            return;
        }
        std::string command = "umount " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Unmounts filesystems. Usage: umount [target]";
    }
};

class InitCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: init [runlevel]\n";
            return;
        }
        std::string command = "init " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Changes the runlevel of the system. Usage: init [runlevel]";
    }
};

class LastCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("last");
    }
    std::string helpText() override
    {
        return "Shows a list of last logged in users. Usage: last";
    }
};

class NmapCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "nmap ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Network exploration tool and security scanner. Usage: nmap [options]";
    }
};

class PsAuxCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("ps aux");
    }
    std::string helpText() override
    {
        return "Detailed view of currently running processes. Usage: ps aux";
    }
};

class TcpdumpCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "tcpdump ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Command-line packet analyzer. Usage: tcpdump [options]";
    }
};

class TouchCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: touch [file]\n";
            return;
        }
        std::string command = "touch " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Updates the access and modification times of a file. Usage: touch [file]";
    }
};

class ManCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: man [command]\n";
            return;
        }
        std::string command = "man " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Displays user manual of any command. Usage: man [command]";
    }
};

class RsyncCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "rsync ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Syncs files and directories between two locations. Usage: rsync [options] [source] [destination]";
    }
};

class SqlCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "sqlite3 ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes SQL commands or scripts. Usage: sql [database] [SQL command]";
    }
};

class GitCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "git ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes Git commands for version control. Usage: git [command]";
    }
};

class PythonCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "python3 ";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes Python scripts or commands. Usage: python [script or command]";
    }
};

class EnvListCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("printenv");
    }
    std::string helpText() override
    {
        return "Lists all environment variables. Usage: envlist";
    }
};

class GppCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: g++ [source file]\n";
            return;
        }
        std::string command = "g++ " + args[1] + " -o " + args[1].substr(0, args[1].find('.')) + " ";
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Compiles C++ source files. Usage: g++ [source file]";
    }
};

class EncryptCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: encrypt [file] [key]\n";
            return;
        }
        std::string command = "openssl enc -aes-256-cbc -salt -in " + args[1] + " -out " + args[1] + ".enc -k " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Encrypts a file. Usage: encrypt [file] [key]";
    }
};

class DiffCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: diff [file1] [file2]\n";
            return;
        }
        std::string command = "diff " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Compares files line by line. Usage: diff [file1] [file2]";
    }
};

class IfstatCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("ifstat"); // Assumes ifstat is installed and available
    }
    std::string helpText() override
    {
        return "Displays network interface statistics. Usage: ifstat";
    }
};

class HtopCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        system("htop"); // Assumes htop is installed
    }
    std::string helpText() override
    {
        return "Provides detailed system performance information. Usage: htop";
    }
};

class VimCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: vim [file]\n";
            return;
        }
        std::string command = "vim " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Opens a file in Vim editor. Usage: vim [file]";
    }
};

class SedCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: sed [expression] [file]\n";
            return;
        }
        std::string command = "sed '" + args[1] + "' " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Performs text transformations. Usage: sed [expression] [file]";
    }
};

class TarCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 4)
        {
            std::cout << "Usage: tar [c|x] [tarfile] [files...]\n";
            return;
        }
        std::string mode = (args[1] == "c" ? "-cf " : "-xf ");
        std::string command = "tar " + mode + args[2] + " ";
        for (int i = 3; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Manages archives for backup and restoration. Usage: tar [c|x] [tarfile] [files...]";
    }
};

class LoginCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: login [username]\n";
            return;
        }
        std::string command = "login " + args[1];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Logs in as a specified user. Usage: login [username]";
    }
};

class ServiceCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: service [service_name] [start|stop|restart]\n";
            return;
        }
        std::string command = "service " + args[1] + " " + args[2];
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Manages system services. Usage: service [service_name] [start|stop|restart]";
    }
};

class DuCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string path = ".";
        if (args.size() > 1)
        {
            path = args[1];
        }
        std::string command = "du -sh " + path;
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Analyzes disk space usage. Usage: du [path]";
    }
};

class MysqlCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "mysql -u user -p -e '";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        command += "'";
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes MySQL commands. Usage: mysql [SQL commands]";
    }
};

class CronCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "crontab ";
        if (args.size() > 1)
        {
            command += args[1];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Manages cron jobs. Usage: cron [filename]";
    }
};

class InotifyCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "inotifywait -m ";
        if (args.size() > 1)
        {
            command += args[1];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Watches file system changes in real time. Usage: inotify [path]";
    }
};

class PlayCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: play [audio file]\n";
            return;
        }
        std::string command = "ffplay -autoexit " + args[1]; // Assumes ffplay is installed
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Plays audio files from the command line. Usage: play [audio file]";
    }
};

class ExecCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "";
        for (int i = 1; i < args.size(); i++)
        {
            command += args[i] + " ";
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Executes scripts or other programs. Usage: exec [command]";
    }
};

class WatchCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 3)
        {
            std::cout << "Usage: watch [interval] [command]\n";
            return;
        }
        int interval = std::stoi(args[1]);
        std::string command;
        for (size_t i = 2; i < args.size(); ++i)
        {
            command += args[i] + " ";
        }
        while (true)
        {
            system("clear");
            system(command.c_str());
            std::cout << "-----\nPress CTRL+C to stop...\n";
            sleep(interval);
        }
    }
    std::string helpText() override
    {
        return "Executes a command repeatedly, displaying the output. Usage: watch [interval] [command]";
    }
};

class ScreenCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "screen";
        for (size_t i = 1; i < args.size(); ++i)
        {
            command += " " + args[i];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Starts a screen session. Usage: screen [options]";
    }
};

class IPTablesCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        std::string command = "iptables";
        for (size_t i = 1; i < args.size(); ++i)
        {
            command += " " + args[i];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Administrate IP packet filter rules. Usage: iptables [options]";
    }
};

class SSHCommand : public Command
{
public:
    void execute(const std::vector<std::string> &args) override
    {
        if (args.size() < 2)
        {
            std::cout << "Usage: ssh [user@hostname]\n";
            return;
        }
        std::string command = "ssh " + args[1];
        for (size_t i = 2; i < args.size(); ++i)
        {
            command += " " + args[i];
        }
        system(command.c_str());
    }
    std::string helpText() override
    {
        return "Connects to a host via SSH. Usage: ssh [user@hostname] [options]";
    }
};

int main()
{
    CommandRegistry registry;
    registry.registerCommand("help", new HelpCommand());
    registry.registerCommand("setenv", new SetEnvCommand());
    registry.registerCommand("ls", new ListFilesCommand());
    registry.registerCommand("ll", new ListFilesDetailCommand());
    registry.registerCommand("cd", new ChangeDirectoryCommand());
    registry.registerCommand("pwd", new PrintWorkingDirectoryCommand());
    registry.registerCommand("cp", new CopyFileCommand());
    registry.registerCommand("rm", new DeleteFileCommand());
    registry.registerCommand("echo", new EchoCommand());
    registry.registerCommand("mkdir", new MakeDirectoryCommand());
    registry.registerCommand("mv", new MoveFileCommand());
    registry.registerCommand("sysinfo", new SystemInfoCommand());
    registry.registerCommand("cat", new CatCommand());
    registry.registerCommand("grep", new GrepCommand());
    registry.registerCommand("top", new TopCommand());
    registry.registerCommand("du", new DuCommand());
    registry.registerCommand("ifconfig", new IfconfigCommand());
    registry.registerCommand("find", new FindCommand());
    registry.registerCommand("wget", new WgetCommand());
    registry.registerCommand("hexdump", new HexDumpCommand());
    registry.registerCommand("ps", new PsCommand());
    registry.registerCommand("netstat", new NetstatCommand());
    registry.registerCommand("shutdown", new ShutdownCommand());
    registry.registerCommand("tail", new TailCommand());
    registry.registerCommand("tar", new TarCommand());
    registry.registerCommand("nano", new NanoCommand());
    registry.registerCommand("http", new HttpCommand());
    registry.registerCommand("chmod", new ChmodCommand());
    registry.registerCommand("chown", new ChownCommand());
    registry.registerCommand("sort", new SortCommand());
    registry.registerCommand("uniq", new UniqCommand());
    registry.registerCommand("wc", new WcCommand());
    registry.registerCommand("df", new DfCommand());
    registry.registerCommand("env", new EnvCommand());
    registry.registerCommand("ln", new LnCommand());
    registry.registerCommand("chgrp", new ChgrpCommand());
    registry.registerCommand("uptime", new UptimeCommand());
    registry.registerCommand("free", new FreeCommand());
    registry.registerCommand("who", new WhoCommand());
    registry.registerCommand("traceroute", new TracerouteCommand());
    registry.registerCommand("gzip", new GzipCommand());
    registry.registerCommand("kill", new KillCommand());
    registry.registerCommand("awk", new AwkCommand());
    registry.registerCommand("uname", new UnameCommand());
    registry.registerCommand("less", new LessCommand());
    registry.registerCommand("date", new DateCommand());
    registry.registerCommand("mount", new MountCommand());
    registry.registerCommand("umount", new UmountCommand());
    registry.registerCommand("init", new InitCommand());
    registry.registerCommand("last", new LastCommand());
    registry.registerCommand("nmap", new NmapCommand());
    registry.registerCommand("ps", new PsCommand());
    registry.registerCommand("psaux", new PsAuxCommand());
    registry.registerCommand("tcpdump", new TcpdumpCommand());
    registry.registerCommand("touch", new TouchCommand());
    registry.registerCommand("man", new ManCommand());
    registry.registerCommand("rsync", new RsyncCommand());
    registry.registerCommand("sql", new SqlCommand());
    registry.registerCommand("git", new GitCommand());
    registry.registerCommand("python", new PythonCommand());
    registry.registerCommand("envlist", new EnvListCommand());
    registry.registerCommand("g++", new GppCommand());
    registry.registerCommand("encrypt", new EncryptCommand());
    registry.registerCommand("diff", new DiffCommand());
    registry.registerCommand("ifstat", new IfstatCommand());
    registry.registerCommand("htop", new HtopCommand());
    registry.registerCommand("vim", new VimCommand());
    registry.registerCommand("sed", new SedCommand());
    registry.registerCommand("login", new LoginCommand());
    registry.registerCommand("service", new ServiceCommand());
    registry.registerCommand("mysql", new MysqlCommand());
    registry.registerCommand("cron", new CronCommand());
    registry.registerCommand("bash", new BashCommand());
    registry.registerCommand("ping", new PingCommand());
    registry.registerCommand("inotify", new InotifyCommand());
    registry.registerCommand("play", new PlayCommand());
    registry.registerCommand("exec", new ExecCommand());
    registry.registerCommand("watch", new WatchCommand());
    registry.registerCommand("screen", new ScreenCommand());
    registry.registerCommand("iptables", new IPTablesCommand());
    registry.registerCommand("ssh", new SSHCommand());

    const char* homeDir = getenv("HOME");
    if (homeDir != nullptr) {
        std::string dshrcPath = std::string(homeDir) + "/.dshrc";
        loadDshrc(dshrcPath, registry);
    }


    std::cout << "Welcome to DSH\n";
    char *input, shell_prompt[100];
    rl_bind_key('\t', rl_complete);

    while (true)
    {
        snprintf(shell_prompt, sizeof(shell_prompt), "%s: ", getcwd(NULL, 0));
        std::cout << std::flush;
        input = readline(shell_prompt);
        if (!input)
            break;

        if (input && *input)
            add_history(input);

        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token)
        {
            tokens.push_back(token);
        }
        free(input);

        if (tokens.empty())
            continue;
        if (tokens[0] == "exit")
            break;

        Command *cmd = registry.getCommand(tokens[0]);
        if (cmd)
        {
            cmd->execute(tokens);
        }
        else
        {
            std::cout << "Unknown command: " << tokens[0] << "\n";
        }
    }
    return 0;
}
