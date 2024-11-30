#include "activities.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>   
#include <limits.h>  
#include <errno.h>    
#include <fcntl.h>    

#define ff(i, a, b) for (int i = a; i < b; i++)

static int compare_bginfo(const void *a, const void *b)
{
    const bginfo *infoA = (const bginfo *)a;
    const bginfo *infoB = (const bginfo *)b;
    return strcmp(infoA->command_name, infoB->command_name);
}

char get_process_state(pid_t pid)
{
    char proc_stat_path[4096];
    snprintf(proc_stat_path, sizeof(proc_stat_path), "/proc/%d/stat", pid);

    FILE *stat_file = fopen(proc_stat_path, "r");
    if (stat_file == NULL)
    {
        // If the file cannot be opened, return '?' indicating process is not available
        return '?';
    }

    char state;
    fscanf(stat_file, "%*d %*s %c", &state); // Skip pid and process name, read state
    fclose(stat_file);
    return state;
}

void activities()
{
    
    ff(i,num_all,num_all+num_ctrlzbg)
    {
        all[i]=ctrlzbg[i-num_all];
    }
    num_all+=num_ctrlzbg;
    
    ff(i, 0, num_all)
    {
        pid_t all_pid = all[i].pid;
        char proc_path[4096];
        snprintf(proc_path, sizeof(proc_path), "/proc/%d", all_pid);

        // Check if the process exists in the /proc directory
        if (access(proc_path, F_OK) != 0) // Process no longer exists
        {
            // Remove the terminated process from the all array
            ff(j, i, num_all - 1)
            {
                all[j].pid = all[j + 1].pid;
                all[j] = all[j + 1];
            }
            num_all--;
            i--;
        }
    }
    int f=0;

    if (num_all == 0) {
        f=1;
        printf("No processes to display.\n");
        return;
    }

    qsort(all, num_all, sizeof(bginfo), compare_bginfo);
    ff(i,0,num_all)
    {
        char state = get_process_state(all[i].pid);

        if (state == 'R' || state == 'S') // Running or Sleeping
        {
            printf("[%d] : %s - Running\n", all[i].pid, all[i].command_name);
        }
        else if (state == 'T') // Stopped
        {
            printf("[%d] : %s - Stopped\n", all[i].pid, all[i].command_name);
        }
        
    }
}
