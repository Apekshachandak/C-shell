#ifndef NEONATE_H
#define NEONATE_H

#include <termios.h>

void enable_raw_mode();

void disable_raw_mode();

// Function to handle errors and exit
void die(const char *s);

int get_most_recent_pid();

void neonate(int timestr);

#endif 
