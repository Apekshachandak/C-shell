#include "prompt.h"
#include <unistd.h>
#include <limits.h>
#include <pwd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>

#ifndef maxname
#define maxname 255
#endif

#ifndef maxpath
#define maxpath 4096
#endif

const char *get_relative_path(const char *cwd, const char *home_dir) {
    static char relative_path[maxpath];
    const char *relative_cwd = cwd;
    
    if (strncmp(cwd, home_dir, strlen(home_dir)) == 0) {
        relative_cwd = cwd + strlen(home_dir);
        if (*relative_cwd == '\0') {
            return "~";
        } else {
            relative_path[0] = '~';
            strcpy(relative_path + 1, relative_cwd);
            return relative_path;
        }
    }
    return relative_cwd;
}

void prompt(const char *home_dir) {
    char cwd[maxpath];
    char hostname[maxname];
    struct passwd *pw = getpwuid(getuid());

    // Get the current working directory
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        perror("getcwd() error");
        return;
    }

    // Get the hostname
    if (gethostname(hostname, sizeof(hostname)) == -1) {
        perror("gethostname() error");
        return;
    }

    // Get the username
    const char *username = pw->pw_name;

    const char *relative_cwd = get_relative_path(cwd, home_dir);

    printf("<%s@%s:%s>", username, hostname, relative_cwd);
}
