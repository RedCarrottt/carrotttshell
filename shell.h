#ifndef _SHELL_H_
#define _SHELL_H_
#include "error.h"

#define BUFFER_SIZE 1024
#define MAX_FORK_DEPTH 100

//error_code recieve_line(char *);
char * type_line();
error_code execute_command(char *);
error_code call_command_handler(char *, char *);
void showException(int);

#endif
