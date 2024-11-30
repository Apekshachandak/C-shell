#include "neonate.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <time.h>
#include <dirent.h>
#include <string.h>

struct termios orig_termios;

void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); // Restore original terminal settings
}

void enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &orig_termios); // Get current terminal attributes
    atexit(disable_raw_mode);               // Ensure original settings are restored on exit

    struct termios raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);        // Disable canonical mode and echo
    raw.c_cc[VMIN] = 0;                     // Set minimum characters to read as 0 (non-blocking)
    raw.c_cc[VTIME] = 1;                    // Set timeout to 1 decisecond (0.1 seconds)

    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw); // Apply the modified settings
}

int get_most_recent_pid() {
    struct dirent *entry;
    int recent_pid = -1;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
        perror("opendir");
        return -1;
    }

    // Iterate through the entries in /proc
    while ((entry = readdir(proc_dir)) != NULL) {
        if (entry->d_type == DT_DIR && atoi(entry->d_name) > 0) { // Check if it's a valid PID directory
            int current_pid = atoi(entry->d_name);
            if (current_pid > recent_pid) {
                recent_pid = current_pid;
            }
        }
    }

    closedir(proc_dir);
    return recent_pid;
}

void neonate(int interval) {
   
    if (interval < 0) {
        fprintf(stderr, "Invalid time interval: %d\n", interval);
        return;
    }

    enable_raw_mode(); // Enable raw mode to detect 'x' key without waiting for Enter

    time_t start_time, current_time;
    int print_count = 0;
    char c;

    // Start timing
    time(&start_time);

    while (1) {
        time(&current_time);
        double elapsed = difftime(current_time, start_time);

        if (elapsed >= (print_count * interval)) {
            print_count++;

            int recent_pid = get_most_recent_pid();
            if (recent_pid != -1) {
                printf("%d\n", recent_pid); // Print the most recent PID
            } else {
                fprintf(stderr, "Failed to get recent PID.\n");
            }

            fflush(stdout);
        }

        // Check for user input (non-blocking due to raw mode)
        if (read(STDIN_FILENO, &c, 1) == 1 && (c == 'x')) {
            break; // Exit if 'x' is pressed
        }

        usleep(100000); // Sleep for 100ms to avoid high CPU usage
    }

    disable_raw_mode(); // Restore original terminal settings
}
