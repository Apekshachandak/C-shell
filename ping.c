#include "ping.h"
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/wait.h>

#define ff(i, a, b) for (int i = a; i < b; i++)
int num_ctrlzbg = 0;
bginfo ctrlzbg[max_bg];
void handle_sigint(int sig)
{
    int f = 0;
    ff(i, 0, num_fg)
    {
        pid_t fg_pid = fginfo[i].pid;

        if (fg_pid != -1)
        {
            f = 1;
            kill(fg_pid, SIGINT);
            fginfo[i].pid = -1; // Reset after interrupting
        }
    }
    if (f == 0)
    {
        
    }
}
// Handle Ctrl-Z (SIGTSTP)
void handle_sigtstp(int sig)
{

    ff(i, 0, num_fg)

    {
        char command_name[4096];

        pid_t fg_pid = fginfo[i].pid;

        if (fg_pid != -1)
        {

            if (kill(fg_pid, SIGTSTP) == -1)
            {
                
            }
          
            strcpy(command_name, fginfo[i].command_name);
            pid_t newpid = fginfo[i].pid;
            if (num_ctrlzbg < max_bg)
            {

                strcpy(ctrlzbg[num_bg].command_name, command_name);
                ctrlzbg[num_bg].pid = newpid;
                num_ctrlzbg++;
            }
            else
            {
                fprintf(stderr, "Maximum number of background processes reached");
            }

            ff(j, i, num_fg - 1)
            {
                fginfo[j] = fginfo[j + 1];
            }
            num_fg--;
            i--;
            fginfo[i].pid = -1;
        }
    }
}

void execute_ping(int pid, int signal_number)
{
    if (pid <= 0)
    {
        fprintf(stderr, "Error: Invalid PID.\n");
        return;
    }

    signal_number %= 32;

    if (kill(pid, signal_number) == -1)
    {
        printf("No such process found\n");
    }
    else
    {
        printf("Sent signal %d to process with pid %d\n", signal_number, pid);
    }
}
