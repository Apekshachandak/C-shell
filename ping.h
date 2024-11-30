#ifndef PING_H
#define PING_H

#include <stdio.h>
#include <stdlib.h>
#include"systemcommands.h"

void execute_ping(int pid, int signal_number);
void handle_sigint(int sig);
void handle_sigtstp(int sig);
#endif
