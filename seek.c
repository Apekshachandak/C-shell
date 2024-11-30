#include "seek.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#define MAX_RESULTS 1024
#define maxpath 4096

#define COLOR_RESET "\033[0m"
#define green "\033[32m"  // Green for files
#define blue "\033[34m"   // Blue for directories

#define ff(i, a, b) for (int i = a; i < b; i++)

// Function to check if a filename matches the target name, ignoring the extension
int name_matches(const char *filename, const char *name)
{
    const char *dot = strrchr(filename, '.');
    int f = 0;
    size_t len;
    if (dot)
    {
        f = 1;
    }
    if (f == 1)
    {
        len = (size_t)(dot - filename);
    }
    else
    {
        len = strlen(filename);
    }

    return strncmp(filename, name, len) == 0 && name[len] == '\0';
}

// Function to check if the result is already present in the results array
int is_duplicate(const char results[][1024], int result_count, const char *path)
{
    ff(i, 0, result_count)
    {
        if (strcmp(results[i], path) == 0)
        {
            return 1;
        }
    }
    return 0;
}

// Function to recursively find entries
void find_entries(const char *directory, const char *name, int search_type, char results[][1024], int *result_count, const char *base_dir)
{
    struct dirent *entry;
    DIR *dp = opendir(directory);

    if (dp == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dp)))
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        {
            continue;
        }

        char path[maxpath];
        snprintf(path, sizeof(path), "%s/%s", directory, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == 0)
        {
            int is_match = name_matches(entry->d_name, name);
            int is_file = S_ISREG(statbuf.st_mode);
            int is_dir = S_ISDIR(statbuf.st_mode);

            if (is_match && ((search_type == 1 && is_file) || (search_type == 2 && is_dir) || (search_type == 0)))
            {
                if (!is_duplicate(results, *result_count, path + strlen(base_dir)))
                {
                    strcpy(results[*result_count], path + strlen(base_dir));
                    (*result_count)++;
                }
            }

            if (is_dir)
            {
                find_entries(path, name, search_type, results, result_count, base_dir);
            }
        }
    }

    closedir(dp);
}

void print_results(const char results[][1024], int result_count)
{
    if (result_count > 0)
    {
        ff(i, 0, result_count)
        {
            struct stat statbuf;
            char full_path[maxpath];
            snprintf(full_path, sizeof(full_path), ".%s", results[i]);

            if (stat(full_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    printf(green ".%s" COLOR_RESET "\n", results[i]);
                }
                else if (S_ISDIR(statbuf.st_mode))
                {
                    printf(blue ".%s" COLOR_RESET "\n", results[i]);
                }
            }
        }
    }
    else
    {
        printf("No match found!\n");
    }
}

// Function to read and print the content of a file
void print_file_content(const char *file_path)
{
    FILE *file = fopen(file_path, "r");
    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file) != NULL)
    {
        printf("%s", buffer);
    }

    fclose(file);
}

// Function to handle the seek command
void handle_seek_command(const char *input, const char *home_dir)
{
    char command[256];
    strcpy(command, input);
    char *token = strtok(command, " ");
    int d_flag = 0, f_flag = 0, e_flag = 0;
    char search_target[256] = "";
    char target_directory[256] = "";

    // Parse input command
    int token_count = 0;
    while (token != NULL)
    {
        if (token_count == 0)
        {
            // First token should be "seek", skip it
        }
        else if (strcmp(token, "-d") == 0)
        {
            d_flag = 1;
        }
        else if (strcmp(token, "-f") == 0)
        {
            f_flag = 1;
        }
        else if (strcmp(token, "-e") == 0)
        {
            e_flag = 1;
        }
        else if (search_target[0] == '\0')
        {
            strcpy(search_target, token);
        }
        else
        {
            // This is the target directory
            if (strcmp(token, "~") == 0)
            {
                strcpy(target_directory, home_dir);
            }
            else if (strcmp(token, ".") == 0)
            {
                getcwd(target_directory, sizeof(target_directory));
            }
            else
            {
                strcpy(target_directory, token);
            }
        }
        token = strtok(NULL, " ");
        token_count++;
    }

    // Handle special cases for target_directory
    if (target_directory[0] == '~')
    {
        char temp[256];
        snprintf(temp, sizeof(temp), "%s%s", home_dir, target_directory + 1);
        strncpy(target_directory, temp, sizeof(target_directory) - 1);
        target_directory[sizeof(target_directory) - 1] = '\0';
    }
    else if (target_directory[0] == '\0')
    {
        getcwd(target_directory, sizeof(target_directory));
    }

    // Check for invalid flag combination
    if (d_flag && f_flag)
    {
        printf("Invalid flags!\n");
        return;
    }

    // Handle the seek command
    char results[MAX_RESULTS][1024];
    int result_count = 0;

    if (d_flag)
    {
        find_entries(target_directory, search_target, 2, results, &result_count, target_directory);
    }
    else if (f_flag)
    {
        find_entries(target_directory, search_target, 1, results, &result_count, target_directory);
    }
    else // d=0 and f=0
    {
        find_entries(target_directory, search_target, 0, results, &result_count, target_directory);
    }

    print_results(results, result_count);
    int e=0;
    if (e_flag)
    {
        e=1;
        if (result_count == 1)
        {
            char full_path[maxpath];
            snprintf(full_path, sizeof(full_path), "%s%s", target_directory, results[0]);
            struct stat statbuf;
            if (stat(full_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode)) // checks if it is a file
                {
                    if (access(full_path, R_OK) == 0) // checks if it has read permissions
                    {
                        print_file_content(full_path);
                    }
                    else
                    {
                        printf("Missing permissions for task!\n");
                    }
                }
                else if (S_ISDIR(statbuf.st_mode)) // checks if it is a dir
                {
                    if (access(full_path, X_OK) == 0) // checks if it has execute permissions(ret 0 if executable)
                    {
                        if (chdir(full_path) != 0)
                        {
                            perror("chdir");
                        }
                    }
                    else
                    {
                        printf("Missing permissions for task!\n");
                    }
                }
            }
        }
    }
}
