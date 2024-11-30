#ifndef HOP_H
#define HOP_H

#include <unistd.h>   
#include <stdio.h>    
#include <string.h>   
#include <stdlib.h>  

#define max_path 4096

void handle_hop_command(char *input, const char *home_dir, char *prev_dir);

#endif
