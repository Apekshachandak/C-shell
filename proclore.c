#include "proclore.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#ifndef max_path
#define max_path 4096
#endif
void proclore(pid_t pid)
{
    
    char proc_path[256];
    char buffer[4096];
    char status;
    char *virmem;
    char executable[max_path];

    char pid_str[10];
    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    strcpy(proc_path, "/proc/");
    strcat(proc_path, pid_str);
    strcat(proc_path, "/stat");

    FILE *fileptr = fopen(proc_path, "r");//open stat file
    if (!fileptr)
    {
        perror("fopen() error");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fileptr) == NULL)
    {
        perror("fgets() error");
        fclose(fileptr);
        return;
    }
    fclose(fileptr);
    //stat file holds info abt a specific process(info abt 23 things)
    // Extract the status and group
    // 1. stores PID
    // 2. stores process name
    // 3. stores process status
    // 4. stores parent PID
    // 5. stores process group
    //and much more in stat file..
    char *token = strtok(buffer, " ");
    int f = 1;
    int pgrp;
    while (token != NULL)
    {
        if (f == 3)
        {
            status = token[0];
        }
        else if (f == 5)
        {
            pgrp = atoi(token);
            break;
        }
        token = strtok(NULL, " ");
        f++;
    }

    snprintf(pid_str, sizeof(pid_str), "%d", pid);
    strcpy(proc_path, "/proc/");
    strcat(proc_path, pid_str);
    strcat(proc_path, "/statm");
    //statm file contains info abt mem usage(info abt 7 things, 1st is total mem(virmem))
    fileptr = fopen(proc_path, "r");
    if (!fileptr)
    {
        perror("fopen() error");
        return;
    }

    if (fgets(buffer, sizeof(buffer), fileptr) == NULL)
    {
        perror("fgets() error");
        fclose(fileptr);
        return;
    }
    fclose(fileptr);

    virmem = strtok(buffer, " ");

    snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", pid);
    ssize_t len = readlink(proc_path, executable, sizeof(executable) - 1);
    executable[len] = '\0';

    printf("pid : %d\n", pid);
    printf("process status : %c%s\n", status, (pid == pgrp) ? "+" : "");
    printf("process group : %d\n", pgrp);
    printf("virtual memory : %s\n", virmem);
    printf("executable path : %s\n", executable);
}

void handle_proclore_command(char *input)
{
   
    char *arguments = input + 8; // Skip the 'proclore' part
    pid_t pid = getpid();

    if (strlen(arguments) > 0)
    {
        pid = atoi(arguments);//atoi converts string to int
    }

    proclore(pid);
}
