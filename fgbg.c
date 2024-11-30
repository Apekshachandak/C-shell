#include"fgbg.h"
#define ff(i, a, b) for (int i = a; i < b; i++)

int procpid(char *input)
{
    char *pidstr = strtok(input, " \t\n");
    pidstr = strtok(NULL, " \t\n");

    if (pidstr == NULL)
    {
        printf("No pid.\n");
        return -1;
    }
    if (strtok(NULL, " \t\n") != NULL)
    {
        printf("Too many arguments.\n");
        return -1;
    }

    return atoi(pidstr);
}

void bgfg(char *input)
{
    int pid = procpid(input);
    if (pid == -1)
        return;

    // Check if the process with the given PID exists in the background processes list
    int i;
    ff(i,0,num_all)
    {
        if (all[i].pid == pid)
            break;
    }
    if (i == num_all)
    {
        printf("Invalid command: No such process in background.\n");
        return;
    }

    // setting the signals to their default values
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    int pgid = getpgid(pid);
    int shellpgid = getpgid(0);

    // bringing the background process to the foreground
    if (tcsetpgrp(0, pgid) == -1)
        printf("Error moving process to foreground.\n");

    // telling the process to start executing again in the foreground
    if (kill(pid, SIGCONT) == -1)
        printf("Error executing process in foreground.\n");

    // parent waiting for process to execute
    int status;
    if (waitpid(pid, &status, WUNTRACED) == -1)
        perror("Kill");

    // remove process from background processes list
    
    ff(j,i,num_all-1)
    {
        all[j].pid = all[j + 1].pid;
        strcpy(all[j].command_name, all[j + 1].command_name);
    }
    num_all--;
    i--;
    // giving control of the foreground back to the terminal
    signal(SIGTTOU, SIG_IGN);
    if (tcsetpgrp(0, shellpgid) == -1)
        printf("Error passing control back to shell.\n");

    // setting the signals to their default values
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}

void bgrun(char *input)
{
    int pid = procpid(input);
    if (pid == -1)
        return;

    if (kill(pid, SIGCONT) != 0)
    {
        perror("Error");
        printf("Could not resume process %d\n", pid);
    }
}