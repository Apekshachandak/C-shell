#ifndef LOG_H
#define LOG_H

#define log_FILE "log.txt"
#define max 15

extern char history[max][4096];
extern int num_log;

void load_log();
void save_log();
void add_pastevent(char *command);
void show_log();
void clear_string(char *str);
void clear_log();
void execute_pastevent(int index);

#endif
