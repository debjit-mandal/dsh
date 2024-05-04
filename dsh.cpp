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
};

// List Files Command
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

// Change Directory Command
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

// Print Working Directory Command
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

// Help Command
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

// Copy File Command
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

// Delete File Command
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

// Echo Command
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

// Make Directory Command
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

// Move File Command
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

// System Info Command
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

// SetEnv Command
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

// View File Content Command
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

// Search File Command
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

// System Monitor Command
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

// Network Configuration Command
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

// Find Files Command
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

// Download File Command
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

// Hex Dump Command
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

// Process List Command
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

// Network Statistics Command
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

// Shutdown Command
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

// Tail File Command
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

// Edit File Command
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

// Simple HTTP Server Command
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

// Permission Change Command
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

// File Ownership Change Command
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

// Sort File Command
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

// Unique Lines Command
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

// Word Count Command
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

// Disk Free Command
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

// Ping Command
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

// Environment Variable Display Command
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

// Symbolic Link Creation Command
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

// Change Group Ownership Command
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

// System Uptime Command
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

// Memory Usage Command
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

// Who is Logged On Command
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

// Trace Route Command
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

// Script Execution Command
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

// Compression Command
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

// Kill Process Command
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

// Search Text Command
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

// Display System Information Command
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

// Interactive File Viewer Command
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

// Date and Time Command
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

// Mount Command
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

// Unmount Command
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

// Change System Runlevel Command
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

// Display Login History Command
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

// Network Scanning Command
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

// View Process Details Command
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

// Monitor Network Traffic Command
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

// Change File Timestamp Command
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

// View Manual Pages Command
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

// File Synchronization Command
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

// SQL Database Interaction Command
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

// Version Control with Git Command
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

// Python Execution Command
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

// Environment Variables List Command
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

// Compile C++ Code Command
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

// File Encryption Command
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

// File Difference Viewer Command
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

// Network Interface Monitor Command
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

// System Load Viewer Command
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

// Open File Viewer Command
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

// Stream Editor Command
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

// Backup and Restore Command
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

// User Login Command
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

// Service Management Command
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

// Disk Usage Analysis Command
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

// Database Management Command
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

// Batch Job Scheduler Command
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

// Monitor File System Changes Command
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

// Play Media Files Command
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

// Execute Scripts Command
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

    std::cout << "Welcome to dsh\n";
    char *input, shell_prompt[100];
    rl_bind_key('\t', rl_complete);

    while (true)
    {
        snprintf(shell_prompt, sizeof(shell_prompt), "%s: ", getcwd(NULL, 0)); // Use getcwd
        std::cout << std::flush;
        input = readline(shell_prompt);
        if (!input)
            break; // Check for EOF.

        if (input && *input)
            add_history(input); // Add non-empty input to history

        // Parse the input into tokens
        std::vector<std::string> tokens;
        std::istringstream iss(input);
        std::string token;
        while (iss >> token)
        {
            tokens.push_back(token);
        }
        free(input); // Free readline allocated input

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
