# Description
Compilation

    Use the following command to compile the shell:
    'make'
Assuming that input will contain max 4096 characters.
1. Display Requirement
   shows username,systemname followed by current directory.
2. supports a ‘;’ or ‘&’ separated list of commands. 
   ';' separated commands execute sequentially. works for all user and system commands.
   '&'operator runs the command preceding it in the background after printing the process id of the newly created process. background execution works only for system commands.
3. hop
   changes the directory that the shell is currently in. It also prints the full path of working directory after changing. supports “.”, “..”, “~”, and “-” flags. If more than one argument is present, executes hop sequentially with all of them being the argument one by one (from left to right). If no argument is present, then hop into the home directory.
4. reveal
   lists all the files and directories in the specified directories.
    -l : displays extra information
    -a : displays all files, including hidden files
    supports “.”, “..”, “~” symbols.
    In my implemenation, output is  not printed in lexicographical order. Also doesnt work for files.
    Input format: reveal <flags> <path/name>
5. log
   similar to the actual history command in bash. stores 15 commands at max. 
   log purge
   Clears all the log currently stored. 
   log execute <index>
   Executes the command at position in log and stores the commmand at that index in log.
6. system commands
   execute the other system commands present in Bash.
   "Foreground Process"
   Executing a command in foreground means the shell will wait for that process to complete and regain control afterwards. Control of terminal is handed over to this process for the time being while it is running.
   "Background Process"
   Any command invoked with “&” is treated as a background command. This implies that your shell will spawn that process but doesn’t hand the control of terminal to it. Shell will keep taking other user commands.
7. Proclore
   obtains information regarding a process. If an argument is missing, prints the information of your shell.
   Information printed:
   pid
   Process Status (R/R+/S/S+/Z)
   Process group
   Virtual Memory
   Executable path of process
8. seek
   looks for a file/directory in the specified target directory (or current if no directory is specified). It returns a list of relative paths (from target directory) of all matching files/directories (files in green and directories in blue) separated with a newline character.
   Input format: seek <flags> <search> <target_directory>
10. i/o redirection
    supports I/O redirection with >, >>, and <, allowing input to be read from files and output to be written or appended to files. 
11. piping
    supports pipes, passing output from one command as input to the next, handling multiple pipes sequentially.
12. redirection and piping
    my implementation doesnt work correctly for & containing commands.
13. activities
    lists all processes spawned by it, sorted in lexicographic order, displaying the command name, PID, and whether each process is running or stopped, in the format [pid] : [command name] - [State].
14. signals
    allows sending signals to processes using the ping command, taking the signal number modulo 32, and handles direct keyboard inputs (Ctrl-C, Ctrl-D, and Ctrl-Z) to control foreground processes.
    Ctrl-C: Sends SIGINT to interrupt the currently running foreground process.
    Ctrl-D: Logs out of the shell after terminating all processes.
    Ctrl-Z: Sends SIGTSTP to stop the foreground process and push it to the background. In my implementation process doesnt get stored as stopped process. Also if no fg process is present, then it terminates my shell on Ctrl-Z.
15. fgbg
    supports the fg <pid> command to bring a running or stopped background process to the foreground and the bg <pid> command to    resume a stopped background process
16. neonate
    implements the neonate command, printing the PID of the most recently created process every specified number of seconds, stopping when the user presses the 'x' key.
17. iMan
    implements the iMan command, fetching man pages from http://man.he.net/ using sockets and printing the response, including HTML tags, while excluding the header from the GET request.
    Input format: iMan <command_name>