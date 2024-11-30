#include "systemcommands.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

#define ff(i, a, b) for (int i = a; i < b; i++)
#define max_bg 4096

pid_t background_processes[max_bg];

int num_bg = 0;
int num_fg=0;
int num_all=0;
bginfo background_process_info[max_bg];
bginfo fginfo[max_bg];
bginfo all[max_bg];
void initialize_all_array()
{
    ff(i, 0, max_bg)
    {
        all[i].pid = 0; // Mark the entries as unused
    }
}
void handle_foreground_command(char *input)
{
    char command_name[4096];
    strcpy(command_name, input);

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork() error");
        return;
    }
    else if (pid == 0)
    {
        char *arguments[4096];
        int i = 0;
        char *semicolon_token = strtok(input, " ");
        while (semicolon_token != NULL)
        {
            arguments[i++] = semicolon_token;
            semicolon_token = strtok(NULL, " ");
        }
        arguments[i] = NULL;

        execvp(arguments[0], arguments);

        perror("execvp() error");
        exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        pid_t wpid = waitpid(pid, &status, 0);
        strcpy(fginfo[num_fg].command_name, command_name);
            fginfo[num_fg].pid = pid;
            num_fg++;
    }
}

void handle_background_command(char *input)
{
    char command_name[4096];
    strcpy(command_name, input);

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("fork() error");
        return;
    }
    else if (pid == 0)
    {
        char *arguments[4096];
        int i = 0;
        char *semicolon_token = strtok(input, " ");
        while (semicolon_token != NULL)
        {
            arguments[i++] = semicolon_token;
            semicolon_token = strtok(NULL, " ");
        }
        arguments[i] = NULL;

        execvp(arguments[0], arguments);

        perror("execvp() error");
        exit(EXIT_FAILURE);
    }
    else
    {
        printf("[%d]\n", pid);
        if (num_bg < max_bg)
        {
            background_processes[num_bg] = pid;
            strcpy(background_process_info[num_bg].command_name, command_name);
            background_process_info[num_bg].pid = pid;
            num_bg++;
            if (num_all < max_bg)
            {
                strcpy(all[num_all].command_name, command_name);
                all[num_all].pid = pid;
                num_all++;
            }
            else
            {
                fprintf(stderr, "Maximum number of processes in 'all' array reached");
            }
        }
        else
        {
            fprintf(stderr, "Maximum number of background processes reached");
        }
    }
}

void check_and_print_completed_bg_processes()
{
    ff(i, 0, num_bg)
    {
        pid_t bg_pid = background_processes[i];
        int status;
        if (waitpid(bg_pid, &status, WNOHANG) > 0)
        {
            if (WIFEXITED(status))
            {
                printf("%s exited normally (%d)\n", background_process_info[i].command_name, bg_pid);
            }
            else 
            {
                printf("%s exited abnormally (%d)\n", background_process_info[i].command_name, bg_pid);
            }
           
            ff(j, i, num_bg - 1)
            {
                background_processes[j] = background_processes[j + 1];
                background_process_info[j] = background_process_info[j + 1];
            }
            num_bg--;
            i--;
        }
    }
}

void handle_system_command(char *input)
{
    if (input[strlen(input) - 1] == '&')
    {
        handle_background_command(input);
    }
    else
    {
        handle_foreground_command(input);
    }
}
