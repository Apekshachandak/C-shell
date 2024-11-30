#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "redirection.h"
#include "log.h"
#include "proclore.h"
#include "reveal.h"
#include "seek.h"
#include "hop.h"
#include "systemcommands.h"


int count_pipes(char *args);
void pipe_handler(char **cmds, int pipe_count);
void io_parse(char *input);
void no_ops_execute(char **args);
void input_redir(char **cmd, char *input);
void output_redir(char **cmd, char *output, int append_flag);
void io_redir(char **cmd, char *input, char *output, int append_flag);
char check_op_order(char *input);
void print_args(char **args);

#define INPUT_SIZE 4096

#define MAX_PIPES 10
int 
count_pipes(char *args) {
    int count = 0; 
    for (int i=0; i < strlen(args); i++) {
        count += (args[i] == '|');
    }
    return count; 
}

void pipe_handler(char **cmds, int pipe_count) {
    int exit_value;  
    int infd; 
    int pipefd[2]; 

    for (int i = 0; i <= pipe_count; i++) {
        //create new pipe for cmd i 
        if (pipe(pipefd) == -1) { 
            perror("pipe"); 
            exit(EXIT_FAILURE); 
        }
        //fork child to handle cmd 
        pid_t pid; 
        pid = fork(); 
        if (pid == -1) {
            perror("fork"); 
            return; 
        } else if(pid == 0) {
            //for all but first cmd, connect stdin with pipefd[0]
            if(i != 0) { 
                dup2(infd, 0); 
            }
            //for all but last cmd, connect stdout with pipefd[1] 
            if (i != pipe_count) { 
                dup2(pipefd[1], 1); 
            }
            io_parse(cmds[i]); 
  
            exit(1); 
        } else {
            //wait and store pipefd[0] for next iteration 
            wait(&exit_value); 
            infd = pipefd[0]; 
            close(pipefd[1]); 
        }
    }
}

void pipe_parse(char *input) {
    char *arg; 
    char **args = malloc(INPUT_SIZE);  
    int count; 

    if(strchr(input, '|')) { 
        int pipe_number = count_pipes(input); 
        count = 0;  
        while((arg = strtok_r(input, "|", &input))) {
            args[count] = arg; 
            count ++; 
        }
       
        pipe_handler(args, pipe_number); 
        free(args); 
    } else {
        io_parse(input); 
    }
    return; 
}

void io_parse(char *input)
{
    char *arg;
    char **args = malloc(INPUT_SIZE);
    int io_flag;
    int append_flag = 0;
    int count = 0;
    int io_order_flag = 0;

    if ((strchr(input, '<')) || (strstr(input, ">")))
    {
        if (strstr(input, ">>"))
        {
            append_flag = 1;
        }

        // Handle both input and output redirection
        if ((strchr(input, '<')) && (strstr(input, ">")))
        {
            if (check_op_order(input) == '>')
            {
                io_order_flag = 1;
            }
            else
            {
                io_order_flag = 0;
            }
            while ((arg = strtok_r(input, "<>", &input)))
            {
                args[count] = arg;
                count++;
            }
            io_flag = 2;
        }
        else if (strchr(input, '<'))
        {
            while ((arg = strtok_r(input, "<", &input)))
            {
                args[count] = arg;
                count++;
            }
            io_flag = 1;
        }
        else if (strstr(input, ">"))
        {
            while ((arg = strtok_r(input, ">", &input)))
            {
                args[count] = arg;
                count++;
            }
            io_flag = 0;
        }

        // Parse out io files and cmds
        char *cmd_arg = args[0];
        char **cmd_args = malloc(INPUT_SIZE);
        cmd_args[0] = cmd_arg;
        char *io_file = strtok(args[1], " ");

        if (io_flag == 2)
        {
            char *io_file2 = strtok(args[2], " ");
            if (io_order_flag == 0)
            {
                io_redir(cmd_args, io_file, io_file2, append_flag);
            }
            else
            {
                io_redir(cmd_args, io_file2, io_file, append_flag);
            }
        }
        else if (io_flag == 1)
        {
            input_redir(cmd_args, io_file);
        }
        else
        {
            output_redir(cmd_args, io_file, append_flag);
        }
    }
    else
    {
        args[0] = input; 
    args[1] = NULL;  //null-terminate


    no_ops_execute(args);
    }
    free(args);
}


void no_ops_execute(char **args)
{
    int exit_value;
    pid_t pid;
    char home_dir[4096];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    char prev_dir[4096] = "";
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        if (strncmp(args[0], "hop", 3) == 0)
        {
            handle_hop_command(args[0], home_dir, prev_dir);
        }
        else if (strncmp(args[0], "reveal", 6) == 0)
        {
            handle_reveal_command(args[0] + 6, home_dir);
        }
        else if (strncmp(args[0], "seek", 4) == 0)
        {
            handle_seek_command(args[0], home_dir);
        }
        else if (strncmp(args[0], "log", 3) == 0)
        {
            if (strcmp(args[0], "log") == 0)
            {
                show_log();
            }
            else if (strcmp(args[0], "log purge") == 0)
            {
                clear_log();
            }
        }
        else if (strncmp(args[0], "proclore", 8) == 0)
        {
            handle_proclore_command(args[0]);
        }

        else
        {
            handle_system_command(args[0]);
        }
    }
    wait(&exit_value);
    return;
}

void input_redir(char **cmd, char *input)
{
    int exit_value;
    int stdin = dup(0);
    int file_fd = open(input, O_RDONLY);
    if (file_fd == -1)
    {
        perror("open");
        return;
    }
    dup2(file_fd, 0);

    // fork child process
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        execvp(cmd[0], cmd);
        perror("execvp");
        exit(1);
    }
    wait(&exit_value);
    dup2(stdin, 0);
    close(stdin);
    close(file_fd);
    return;
}

void output_redir(char **cmd, char *output, int append_flag)
{
    int exit_value;
    int flags;
    int stdout = dup(1);
    if (append_flag == 1)
    {
        flags = (O_RDWR | O_CREAT | O_APPEND);
    }
    else
    {
        flags = (O_RDWR | O_CREAT);
    }
    int file_fd = open(output, flags, 0644);
    if (file_fd == -1)
    {
        perror("open");
        return;
    }
    dup2(file_fd, 1);
    char home_dir[4096];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    char prev_dir[4096] = "";
    // fork child process
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        if (strncmp(cmd[0], "hop", 3) == 0)
        {
            handle_hop_command(cmd[0], home_dir, prev_dir);
        }
        else if (strncmp(cmd[0], "reveal", 6) == 0)
        {
            handle_reveal_command(cmd[0] + 6, home_dir);
        }
        else if (strncmp(cmd[0], "seek", 4) == 0)
        {
            handle_seek_command(cmd[0], home_dir);
        }
        else if (strncmp(cmd[0], "log", 3) == 0)
        {
            if (strcmp(cmd[0], "log") == 0)
            {
                show_log();
            }
            else if (strcmp(cmd[0], "log purge") == 0)
            {
                clear_log();
            }
        }
        else if (strncmp(cmd[0], "proclore", 8) == 0)
        {
            handle_proclore_command(cmd[0]);
        }

        else
        {
            handle_system_command(cmd[0]);
        }
    }
    wait(&exit_value);
    dup2(stdout, 1);
    close(stdout);
    close(file_fd);
    return;
}

void io_redir(char **cmd, char *input, char *output, int append_flag)
{
    int exit_value;
    int flags;

    // set flags
    if (append_flag == 1)
    {
        flags = (O_RDWR | O_CREAT | O_APPEND);
    }
    else
    {
        flags = (O_RDWR | O_CREAT);
    }

    // manipulate fds
    int stdout = dup(1);
    int stdin = dup(0);
    int outfile_fd = open(output, flags, 0644);
    if (outfile_fd == -1)
    {
        perror("open out");
        return;
    }
    int infile_fd = open(input, O_RDONLY);
    if (infile_fd == -1)
    {
        perror("open in");
        return;
    }
    dup2(infile_fd, 0);
    dup2(outfile_fd, 1);
    char home_dir[4096];
    if (getcwd(home_dir, sizeof(home_dir)) == NULL)
    {
        perror("getcwd() error");
        return;
    }
    char prev_dir[4096] = "";
    // fork and execute cmd
    pid_t pid;
    pid = fork();
    if (pid == -1)
    {
        perror("fork");
        return;
    }
    else if (pid == 0)
    {
        if (strncmp(cmd[0], "hop", 3) == 0)
        {
            handle_hop_command(cmd[0], home_dir, prev_dir);
        }
        else if (strncmp(cmd[0], "reveal", 6) == 0)
        {
            handle_reveal_command(cmd[0] + 6, home_dir);
        }
        else if (strncmp(cmd[0], "seek", 4) == 0)
        {
            handle_seek_command(cmd[0], home_dir);
        }
        else if (strncmp(cmd[0], "log", 3) == 0)
        {
            if (strcmp(cmd[0], "log") == 0)
            {
                show_log();
            }
            else if (strcmp(cmd[0], "log purge") == 0)
            {
                clear_log();
            }
        }
        else if (strncmp(cmd[0], "proclore", 8) == 0)
        {
            handle_proclore_command(cmd[0]);
        }

        else
        {
            handle_system_command(cmd[0]);
        }
    }
    wait(&exit_value);

    // clean up
    dup2(stdin, 0);
    dup2(stdout, 1);
    close(stdin);
    close(stdout);
    close(infile_fd);
    close(outfile_fd);
    return;
}

char check_op_order(char *input)
{
    int in, out;
    in = out = 0;
    while ((input[in] != '<') && (input[in] != '\0'))
    {
        in++;
    }
    while ((input[out] != '>') && (input[out] != '\0'))
    {
        out++;
    }
    if (in < out)
    {
        return ('<');
    }
    else
    {
        return ('>');
    }
}

void print_args(char **args)
{
    while (*args)
    {
        printf("%s\n", *args++);
    }
    return;
}
