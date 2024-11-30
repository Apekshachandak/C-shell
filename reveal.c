#include "reveal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#define ff(i, a, b) for (int i = a; i < b; i++)

void print_file_info(const char *dir_path, struct dirent *entry, int l_flag)
{
    struct stat file_stat;
    char finalpath[1024];

    strcpy(finalpath, dir_path);
    int k=0;
    // Add a slash if the directory path does not end with one
    if (finalpath[strlen(finalpath) - 1] != '/')
    {
        strcat(finalpath, "/");
    }

    // Append the file name
    strcat(finalpath, entry->d_name);
    if (stat(finalpath, &file_stat) == -1)
    {
        perror("stat");
        return;
    }
    int f = 0;
    if (l_flag == 1)
    {
        f = 1;
    }

    if (f == 1)
    {
        char permissions[11];
        snprintf(permissions, sizeof(permissions), "%c%c%c%c%c%c%c%c%c%c",
                 (S_ISDIR(file_stat.st_mode)) ? 'd' : '-',
                 (file_stat.st_mode & S_IRUSR) ? 'r' : '-',
                 (file_stat.st_mode & S_IWUSR) ? 'w' : '-',
                 (file_stat.st_mode & S_IXUSR) ? 'x' : '-',
                 (file_stat.st_mode & S_IRGRP) ? 'r' : '-',
                 (file_stat.st_mode & S_IWGRP) ? 'w' : '-',
                 (file_stat.st_mode & S_IXGRP) ? 'x' : '-',
                 (file_stat.st_mode & S_IROTH) ? 'r' : '-',
                 (file_stat.st_mode & S_IWOTH) ? 'w' : '-',
                 (file_stat.st_mode & S_IXOTH) ? 'x' : '-');

        struct passwd *pw = getpwuid(file_stat.st_uid);
        struct group *gr = getgrgid(file_stat.st_gid);
        char timebuf[80];
        time_t mod_time = file_stat.st_mtime;                        // modification time
        struct tm *timeinfo = localtime(&mod_time);                  // Convert to local time
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", timeinfo); // Format the time

        printf("%s %ld %s %s %ld %s %s\n", permissions, file_stat.st_nlink, pw->pw_name, gr->gr_name, file_stat.st_size, timebuf, entry->d_name);
    }
    else
    {
        if (S_ISDIR(file_stat.st_mode))
        {
            printf("\033[1;34m%s\033[0m\n", entry->d_name); // Blue for directories
        }
        else if (file_stat.st_mode & S_IXUSR)
        {
            printf("\033[1;32m%s\033[0m\n", entry->d_name); // Green for executables
        }
        else
        {
            printf("%s\n", entry->d_name); // Default color for regular files
        }
    }
}

void handle_reveal_command(const char *input, const char *home_dir)
{
    int a_flag = 0;
    int l_flag = 0;
    char path[1024] = ".";
    char *token = NULL;
    char *command = strdup(input);

    // Parse flags and path
    token = strtok(command, " ");
    while (token != NULL)
    {
        if (strcmp(token, "-") == 0)  // token is "-"
        {
            const char *oldpwd = getenv("OLDPWD");
            if (oldpwd != NULL)  
            {
                strncpy(path, oldpwd, sizeof(path) - 1);
                path[sizeof(path) - 1] = '\0';
            }
        }
        if (token[0] == '-')
        {
            if (strlen(token) > 1)
            {
                ff(i,1,strlen(token))
                {
                    if (token[i] == 'a')
                    {
                        a_flag = 1;
                    }
                    else if (token[i] == 'l')
                    {
                        l_flag = 1;
                    }
                }
            }
        }
        else if (token[0] != '-')
        {
            strncpy(path, token, sizeof(path) - 1);
            path[sizeof(path) - 1] = '\0';
        }
        token = strtok(NULL, " ");
    }
    free(command);

    // Handle special paths
    if (strcmp(path, "~") == 0)
    {
        strncpy(path, home_dir, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }
    
    if (path[0] == '~')
    {
        char temp[256];
        snprintf(temp, sizeof(temp), "%s%s", home_dir, path + 1);
        strncpy(path, temp, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    // Open directory
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        fprintf(stderr, "opendir() error: %s\n", strerror(errno));
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (!a_flag && entry->d_name[0] == '.')
        {
            continue;
        }
        print_file_info(path, entry, l_flag);
    }

    closedir(dir);
}
