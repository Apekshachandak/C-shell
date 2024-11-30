#include "hop.h"

void handle_hop_command(char *input, const char *home_dir, char *prev_dir) {
    char *arguments = input + 3;  // Skip the 'hop' part

    // Save the current directory
    char current_dir[max_path];
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd() error");
        return;
    }

    // If no argument is given, hop to home directory
    if (strlen(arguments) == 0) {
        if (chdir(home_dir) != 0) {
            perror("chdir() error");
        } else {
            printf("%s\n", home_dir);
            strcpy(prev_dir, current_dir);  // Update prev dir
            strcpy(current_dir, home_dir);  // Update the current
        }
        return;
    }

    // Split the arguments by spaces
    char *semicolon_token = strtok(arguments, " ");
    while (semicolon_token != NULL) {
        char path[max_path];
        if (strcmp(semicolon_token, "~") == 0) {
            strcpy(path, home_dir);
        } else if (strcmp(semicolon_token, "-") == 0) {
            if (strlen(prev_dir) == 0) {
                fprintf(stderr, "No previous directory.\n");
                return;
            }
            strcpy(path, prev_dir);
        } else if (strcmp(semicolon_token, ".") == 0) {
            strcpy(path, current_dir);
        } else if (strcmp(semicolon_token, "..") == 0) {
            if (chdir("..") != 0) {
                perror("chdir() error");
                return;
            }
            if (getcwd(path, sizeof(path)) == NULL) {
                perror("getcwd() error");
                return;
            }
        } else {
            strcpy(path, semicolon_token);
        }

        // Change the directory
        if (chdir(path) != 0) {
            perror("chdir() error");
        } else {
            // Print the new working directory
            if (getcwd(path, sizeof(path)) == NULL) {
                perror("getcwd() error");
            } else {
                printf("%s\n", path);
                strcpy(prev_dir, current_dir);  // Update prev dir
                strcpy(current_dir, path);      // Update the current directory
            }
        }

        semicolon_token = strtok(NULL, " ");  // Move to the next semicolon_token
    }
}
