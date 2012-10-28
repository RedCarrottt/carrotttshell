#ifndef _COMMANDS_H_
#define _COMMANDS_H_

#include <stdio.h>
#include "error.h"

error_code myls(char *);
error_code mycd(char *);
error_code mymkdir(char *);
error_code myrmdir(char *);
error_code myrm(char *);
error_code myfind(char *);
error_code mypwd(char *);
char * presentPath(error_code *);
error_code mysh(char *);
error_code myexecute(char *, char *);
error_code myexecute_grp(char *, char *, char *, char *);

#endif
