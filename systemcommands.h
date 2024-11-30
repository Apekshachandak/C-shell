#ifndef SYSTEMCOMMANDS_H
#define SYSTEMCOMMANDS_H

#include <sys/types.h>

#define max_bg 4096

typedef struct
{
    int l; 
    char command_name[4096];
    pid_t pid;
} bginfo;

extern pid_t background_processes[max_bg];
extern bginfo fginfo[max_bg];
extern bginfo ctrlzbg[max_bg];
extern int num_ctrlzbg;
extern bginfo background_process_info[max_bg];
extern bginfo all[max_bg];

extern int num_bg;
extern int num_all;
extern int num_fg;

void handle_foreground_command(char *input);  
void handle_background_command(char *input); 
void check_and_print_completed_bg_processes(); 
void handle_system_command(char *input); 

#endif
