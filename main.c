#include "log.h"
#include "proclore.h"
#include "reveal.h"
#include "seek.h"
#include "hop.h"
#include "prompt.h"
#include "activities.h"
#include "redirection.h"
#include "systemcommands.h"
#include "neonate.h"
#include "fgbg.h"
#include "iman.h"
#include "ping.h"
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#define ff(i, a, b) for (int i = a; i < b; i++)
#define max_length 100

#define MAX_TOKENS 4096
#define MAX_EXECUTED_COMMANDS 4096

void trimLeadingWhitespace(char *str)
{
    char *start = str;
    while (isspace((unsigned char)*start))
    {
        start++;
    }
    if (start != str)
    {
        memmove(str, start, strlen(start) + 1);
    }
}

void trimTrailingWhitespace(char *str)
{
    char *end = str + strlen(str) - 1;

    while (end > str && isspace((unsigned char)*end))
    {
        end--;
    }

    // Null-terminate the string after trimming
    *(end + 1) = '\0';
}

void trim_whitespace(char *str)
{
    trimLeadingWhitespace(str);
    trimTrailingWhitespace(str);
}

void removeExtraSpaces(char *str)
{
    int length = strlen(str);
    int idx = 0;
    int foundspace = 0;
    ff(i, 0, length)
    {
        if (isspace((unsigned char)str[i]))
        {
            if (!foundspace)
            {
                str[idx] = ' ';
                idx++;
                foundspace = 1;
            }
        }
        else
        {
            str[idx] = str[i];
            idx++;
            foundspace = 0;
        }
    }

    str[idx] = '\0';
}

void preprocess_command(char *str)
{
    trimLeadingWhitespace(str);
    trimTrailingWhitespace(str);
    removeExtraSpaces(str);
}

char executed_commands[MAX_EXECUTED_COMMANDS][100];

void execute_command(char *input, const char *home_dir, char *prev_dir)
{
    preprocess_command(input);
    char *semicolon_tokens[MAX_TOKENS];
    int num_semicolon_tokens = 0;
    int pid, signal_number;
    char flag[5]; 
    char time_str[10];
    char *semicolon_token = strtok(input, ";");
    while (semicolon_token != NULL)
    {
        semicolon_tokens[num_semicolon_tokens++] = strdup(semicolon_token);
        semicolon_token = strtok(NULL, ";");
    }
    ff(i, 0, num_semicolon_tokens)
    {
        char *semicolon_command = semicolon_tokens[i];
        preprocess_command(semicolon_command);
        if (strstr(semicolon_command, "&") != NULL)
        {
            char *ampersand_tokens[MAX_TOKENS];
            int num_ampersand_tokens = 0;

            char *ampersand_token = strtok(semicolon_command, "&");
            while (ampersand_token != NULL)
            {
                ampersand_tokens[num_ampersand_tokens++] = strdup(ampersand_token);
                ampersand_token = strtok(NULL, "&");
            }

            ff(k, 0, num_ampersand_tokens)
            {
                char *ampersand_command = ampersand_tokens[k];

                preprocess_command(ampersand_command);
                if (num_ampersand_tokens > 1)
                {
                    if (k < num_ampersand_tokens - 1)
                    {
                        if (strncmp(ampersand_command, "hop", 3) != 0 && strncmp(ampersand_command, "log", 3) != 0 && strncmp(ampersand_command, "proclore", 8) != 0 && strncmp(ampersand_command, "reveal", 6) != 0 && strncmp(ampersand_command, "seek", 4) != 0)
                        {
                            handle_background_command(ampersand_command);
                        }
                        else
                        {
                            fprintf(stderr, "Background execution not supported for non-system commands: %s\n", ampersand_command);
                        }
                    }
                    else
                    {
                        if (strstr(semicolon_command, ">") != NULL || strstr(semicolon_command, ">>") != NULL || strstr(semicolon_command, "<") != NULL || strstr(semicolon_command, "|") != NULL)
                        {
                            pipe_parse(semicolon_command);
                        }
                        else
                        {
                            if (strncmp(ampersand_command, "hop", 3) == 0)
                            {
                                handle_hop_command(ampersand_command, home_dir, prev_dir);
                            }
                            else if (strncmp(ampersand_command, "log", 3) == 0)
                            {
                                if (strcmp(ampersand_command, "log") == 0)
                                {
                                    show_log();
                                }
                                else if (strcmp(ampersand_command, "log purge") == 0)
                                {
                                    clear_log();
                                }
                            }
                            else if (strncmp(ampersand_command, "proclore", 8) == 0)
                            {
                                handle_proclore_command(ampersand_command);
                            }
                            else if (strncmp(ampersand_command, "reveal", 6) == 0)
                            {
                                handle_reveal_command(ampersand_command + 6, home_dir);
                            }
                            else if (strncmp(ampersand_command, "seek", 4) == 0)
                            {
                                handle_seek_command(ampersand_command, home_dir);
                            }
                            else if (strncmp(ampersand_command, "activities", 10) == 0)
                            {
                                activities();
                            }
                            else if (sscanf(ampersand_command, "ping %d %d", &pid, &signal_number) == 2)
                            {
                                execute_ping(pid, signal_number);
                            }
                            else if (sscanf(ampersand_command, "fg %d", &pid) == 1)
                            {
                                bgfg(ampersand_command);
                            }
                            else if (sscanf(ampersand_command, "bg %d", &pid) == 1)
                            {
                                bgrun(ampersand_command);
                            }
                            else if (strncmp(ampersand_command, "neonate", 7) == 0)
                            {
                                sscanf(ampersand_command + 7, "%s %s", flag, time_str);

                                // Check if the flag is "-n"
                                if (strcmp(flag, "-n") == 0)
                                {
                                    // Convert time_str to an integer (time_arg)
                                    int time_arg = atoi(time_str);

                                    // Validate time_arg
                                    if (time_arg >= 0)
                                    {
                                        // Call neonate functionality with time_arg
                                        neonate(time_arg); // Call your neonate function here
                                    }
                                    else
                                    {
                                        fprintf(stderr, "Invalid time argument: %s\n", time_str);
                                    }
                                }
                            }
                            else if (strncmp(ampersand_command, "iMan", 4) == 0)
                            {
                                iMan(ampersand_command + 5);
                            }
                            else
                            {
                                handle_system_command(ampersand_command);
                            }
                        }
                    }
                }
                else if (num_ampersand_tokens == 1)
                {
                    if (strncmp(ampersand_command, "hop", 3) != 0 && strncmp(ampersand_command, "log", 3) != 0 && strncmp(ampersand_command, "proclore", 8) != 0 && strncmp(ampersand_command, "reveal", 6) != 0 && strncmp(ampersand_command, "seek", 4) != 0)
                    {
                        handle_background_command(ampersand_command);
                    }
                    else
                    {
                        fprintf(stderr, "Background execution not supported for non-system commands: %s\n", ampersand_command);
                    }
                }

                free(ampersand_command);
            }
        }
        else // only semicolon wale
        {
            if (strstr(semicolon_command, ">") != NULL || strstr(semicolon_command, ">>") != NULL || strstr(semicolon_command, "<") != NULL || strstr(semicolon_command, "|") != NULL)
            {
                pipe_parse(semicolon_command);
            }

            else
            {
                if (strncmp(semicolon_command, "hop", 3) == 0)
                {
                    handle_hop_command(semicolon_command, home_dir, prev_dir);
                }
                else if (strncmp(semicolon_command, "log", 3) == 0)
                {
                    if (strcmp(semicolon_command, "log") == 0)
                    {
                        show_log();
                    }
                    else if (strcmp(semicolon_command, "log purge") == 0)
                    {
                        clear_log();
                    }
                }
                else if (strncmp(semicolon_command, "proclore", 8) == 0)
                {
                    handle_proclore_command(semicolon_command);
                }
                else if (strncmp(semicolon_command, "reveal", 6) == 0)
                {
                    handle_reveal_command(semicolon_command + 6, home_dir);
                }
                else if (strncmp(semicolon_command, "seek", 4) == 0)
                {
                    handle_seek_command(semicolon_command, home_dir);
                }
                else if (strncmp(semicolon_command, "activities", 10) == 0)
                {
                    activities();
                }
                else if (sscanf(semicolon_command, "ping %d %d", &pid, &signal_number) == 2)
                {
                    execute_ping(pid, signal_number);
                }
                else if (sscanf(semicolon_command, "fg %d", &pid) == 1)
                {
                    bgfg(semicolon_command);
                }
                else if (sscanf(semicolon_command, "bg %d", &pid) == 1)
                {
                    bgrun(semicolon_command);
                }
                else if (strncmp(semicolon_command, "neonate", 7) == 0)
                {
                    // Convert time_str to an integer (time_arg)
                    sscanf(semicolon_command + 7, "%s %s", flag, time_str);

                    // Check if the flag is "-n"
                    if (strcmp(flag, "-n") == 0)
                    {
                        // Convert time_str to an integer (time_arg)
                        int time_arg = atoi(time_str);

                        // Validate time_arg
                        if (time_arg >= 0)
                        {
                            // Call neonate functionality with time_arg
                            neonate(time_arg); // Call your neonate function here
                        }
                        else
                        {
                            fprintf(stderr, "Invalid time argument: %s\n", time_str);
                        }
                    }
                }
                else if (strncmp(semicolon_command, "iMan", 4) == 0)
                {
                    iMan(semicolon_command + 5);
                }

                else
                {
                    handle_system_command(semicolon_command);
                }
            }
        }
        free(semicolon_command);
    }
}

int main()
{
    
    struct sigaction sa_tstp;
 
    signal(SIGINT, handle_sigint); // Handle Ctrl-C
    // Set up SIGTSTP handler
    sa_tstp.sa_handler = handle_sigtstp;
    sigemptyset(&sa_tstp.sa_mask);
    sa_tstp.sa_flags = 0; // Or use SA_RESTART if needed
    if (sigaction(SIGTSTP, &sa_tstp, NULL) == -1)
    {
        perror("sigaction for SIGTSTP");
        exit(EXIT_FAILURE);
    }

    char home_dir[4096];

    if (getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        perror("getcwd() error");
        return 0;
    }

    char prev_dir[max_path] = "";

    load_log();

    // Keep accepting commands
    while (1)
    {

        // Print appropriate prompt with username, systemname and directory before accepting input
        prompt(home_dir);
        char input[4096];

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            break;
        }

        // Remove newline character from the end of the input and replace with null terminator
        input[strcspn(input, "\n")] = '\0';
        check_and_print_completed_bg_processes();
        if (strcmp(input, "exit") == 0)
        {
            save_log();
            add_pastevent(input);
            break;
        }
        preprocess_command(input);
        char modified[4096] = "";
        char command[100];
        char temp[2]; // Temporary string for strncat
        int index;
        int i = 0, j = 0;

        // Initialize modified string
        modified[0] = '\0';
        int f = 0;
        while (input[i] != '\0')
        {
            j = 0;
            // Reset command
            memset(command, 0, sizeof(command));

            // Extract a single command
            while (input[i] != ';' && input[i] != '&' && input[i] != '|' && input[i] != '>' && input[i] != '<' && input[i] != '\0')
            {
                if (f == 0)
                {
                    temp[0] = input[i];
                    temp[1] = '\0';
                    strncat(command, temp, 1);
                    i++;
                }
            }

            // Check if the command is "log execute <index>"
            if (sscanf(command, "log execute %d", &index) == 1 || sscanf(command, " log execute %d", &index) == 1)
            {
                if (index > 0 && index <= num_log)
                {
                    strcat(modified, history[num_log - index]);
                }
            }
            else
            {
                strcat(modified, command);
            }

            // Add the delimiter back to the modified string
            if (input[i] != '\0')
            {
                temp[0] = input[i];
                temp[1] = '\0';
                strcat(modified, temp);
            }

            // Move to the next character if not end of input
            if (input[i] != '\0')
            {
                i++;
            }
        }
        add_pastevent(modified);

        // else if (strstr(modified, "|") != NULL)
        // {
        //     piping(modified);
        // }
        // else
        {
            execute_command(modified, home_dir, prev_dir);
        }
    }
    save_log();

    return 0;
}
