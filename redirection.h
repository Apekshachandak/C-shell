#ifndef REDIRECTION_H
#define REDIRECTION_H

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#define INPUT_SIZE 4096

void tokenizer(char *result[], char *command, const char *delimiter, int *count);
void piping(char *command);
void handle_command(char *input);
void pipe_parse(char *input);
void io_parse(char *input);
void no_ops_execute(char **args);
void input_redir(char **cmd, char *input);
void output_redir(char **cmd, char *output, int append_flag);
void pipe_handler(char **cmds, int pipe_count);
void io_redir(char **cmd, char *input, char *output, int append_flag);
int count_pipes(char *args);
char check_op_order(char *input);
void print_args(char **args);

#endif 
