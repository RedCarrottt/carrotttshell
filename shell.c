#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include "parsing.h"
#include "shell.h"
#include "commands.h"
typedef void Sigfunc(int);

// the clean up function when executed as foreground
void cleanup(int signo) {
	return;
}

int main() {
	error_code errorCode = 1;	
	Sigfunc * intFunc, * quitFunc, * tstpFunc;

	// start to ignore signals
	if((intFunc = signal(SIGINT, SIG_IGN)) != SIG_IGN) {
		signal(SIGINT, cleanup);
	}
	if((quitFunc = signal(SIGQUIT, SIG_IGN)) != SIG_IGN) {
		signal(SIGQUIT, cleanup);
	}
	if((tstpFunc = signal(SIGTSTP, SIG_IGN)) != SIG_IGN) {
		signal(SIGTSTP, cleanup);
	}

	// main loop until exit command input
	while(errorCode != ERROR_EXIT) {
		char * typed_str;
		typed_str = type_line();
		errorCode = execute_command(typed_str);
		free(typed_str);
	}

	// restore signal handlers
	signal(SIGINT, intFunc);
	signal(SIGQUIT, quitFunc);
	signal(SIGTSTP, tstpFunc);

	kill(0, SIGKILL);

	return 0;
}

/*
// a command line -> commands
// recieve a command line as a character array
error_code recieve_line(char * typed_str) {
	int chunks, i;
	char * chunk;
	char * buffer;
	SYMBOL symbol;
	error_code error;

	if(typed_str == NULL)
		return;
	// execute command by command bounded by pipe symbol
	chunks = numChunk(typed_str);
	buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	buffer[0] = '\0';
	for(i=0; i<chunks; i++) {
		chunk = getChunk(typed_str, i);
		symbol = chunkToSymbol(chunk);
		switch(symbol) {
			case SYM_PIPE:
				// execute command
				error = execute_command(buffer);
				if(error != NO_ERROR) {
					free(chunk);
					free(buffer);
					return error;
				}
				buffer[0] = '\0';	// clear buffer
				break;
			case SYM_BG:
			case SYM_NORMAL:
				// push a command chunk into command buffer
				if(buffer[0] != '\0')
					strncat(buffer, " ", strlen(" "));
				strncat(buffer, chunk, strlen(chunk));
				break;
		}
		free(chunk);
	}
	// if there is command in buffer, execute it.
	if(buffer[0] != '\0') {
		error = execute_command(buffer);
		if(error != NO_ERROR) {
			free(buffer);
			return error;
		}
	}
	free(buffer);
	return NO_ERROR;
}
*/

// keyboard -> a command line
// type a command line by keyboard
char * type_line() {
	char * typed_str = (char *)malloc(sizeof(char) * BUFFER_SIZE);
	char * buf;
	char path[BUFFER_SIZE];
	error_code error;

	// get present path
	buf = presentPath(&error);
	if(error != NO_ERROR) {
		printf("cannot read present path\n");
		return NULL;
	}
	strncpy(path, buf, strlen(buf));
	path[strlen(buf)] = '\0';
	free(buf);

	// display present path and get a command line by keyboard
	printf("%s $$ ", path);
	fgets(typed_str, BUFFER_SIZE, stdin);
	typed_str[strlen(typed_str) - 1] = '\0';
	return typed_str;
}

// command -> command handler
// execute command
error_code execute_command(char * fullCmd) {
	pid_t pid;	
	error_code error;	
	char * cmd;
	char * prm;
	int i, chunks;
	char * chunk;
	
	// split full command to command head and parameters
	chunks = numChunk(fullCmd);
	if(fullCmd == NULL)
		return ERROR_CODE(-10);
	cmd = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	prm = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	cmd[0] = '\0';
	prm[0] = '\0';
	if(chunks > 0) {
		chunk = getChunk(fullCmd, 0);
		strncat(cmd, chunk, strlen(chunk));
		free(chunk);
	}
	for(i=1; i<chunks; i++) {
		chunk = getChunk(fullCmd, i);
		if(prm[0] != '\0')
			strncat(prm, " ", strlen(" "));
		strncat(prm, chunk, strlen(chunk));
		free(chunk);
	}

	error = call_command_handler(cmd, prm);
	free(cmd);
	free(prm);
	return error;
}

// call command handler
error_code call_command_handler(char * cmd, char * prm) {
	// exception
	if(cmd == NULL)
		return ERROR_EXIT;
	
	// call command handler corresponding to each command head
	if(IS_EQUAL(cmd, "exit")) {
		return ERROR_EXIT;
	} else if(IS_EQUAL(cmd, "ls")) {
		return myls(prm);
	} else if(IS_EQUAL(cmd, "cd")) {
		return mycd(prm);
	} else if(IS_EQUAL(cmd, "mkdir")) {
		return mymkdir(prm);
	} else if(IS_EQUAL(cmd, "rmdir")) {
		return myrmdir(prm);
	} else if(IS_EQUAL(cmd, "rm")) {
		return myrm(prm);
	} else if(IS_EQUAL(cmd, "find")) {
		return myfind(prm);
	} else if(IS_EQUAL(cmd, "pwd")) {
		return mypwd(prm);
	} else if(IS_EQUAL(cmd, "sh")) {
		return mysh(prm);
	} else {
		return myexecute(cmd, prm);
	}
}

// display exception(error code)
void showException(error_code code) {
	printf("Error Occured! (Code : %d)\n", code);
}
