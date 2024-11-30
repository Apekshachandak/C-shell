#include "log.h"
#include <stdio.h>
#include <string.h>

#define ff(i,a,b) for(int i=a;i<b;i++)

char history[max][4096];
int num_log = 0;

void load_log()
{
    FILE *file;
    if ((file = fopen(log_FILE, "r")) == NULL)
    {
        return;
    }

    char line[4096];
    while (fgets(line, sizeof(line), file))
    {
        if (num_log < max)
        {
            char *token = strtok(line, "\n");//remove newline
            if (token != NULL)
            {
                strcpy(line, token);
            }

            strcpy(history[num_log++], line);
        }
    }

    fclose(file);
}

void save_log()
{
    FILE *file;
    if ((file = fopen(log_FILE, "w")) == NULL)
    {
        perror("fopen() error");
        return;
    }
    ff(i,0,num_log)
    {
        fprintf(file, "%s\n", history[i]);
    }

    fclose(file);
}

void add_pastevent(char *command)
{
    // Skip storing log or log purge commands
   if (strstr(command, "log") != NULL)
    {
        return;
    }

    // Skip storing duplicate consecutive commands
    if (num_log > 0 && strcmp(history[num_log - 1], command) == 0)
    {
        return;
    }

    // Add the command to log
    if (num_log < max)
    {
        strcpy(history[num_log], command);
        num_log++;

    }
    else
    {
        // Shift commands to make space for the new one
        ff(i,1,15)
        {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[14], command);
    }
}

void show_log()
{
    ff(i,0,num_log)
    {
        printf("%d %s\n", i + 1, history[i]);
    }
}

void clear_string(char *str) {
    str[0] = '\0';
}

void clear_log() {
    ff(i,0,num_log)
    {
        clear_string(history[i]);
    }
    num_log = 0;
}

