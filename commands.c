#include "commands.h"
#include "shell.h"
#include "parsing.h"
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>

#define OPEN_MAX 10240

extern char path[BUFFER_SIZE];
char * find_target;

// ls
error_code myls(char * params) {
	DIR * dp;
	char * target;
	int inLoop;
	struct dirent * entry;
	int cnt;
	
	if(numChunk(params) > 1) {
		printf("ls : too many parameters\n");
		return ERROR_CODE(-2);
	}

	if(numChunk(params) == 0) {
		dp = opendir(".");
	} else {
		target = getChunk(params, 0);
		if(target == NULL) {
			printf("ls : wrong parameter\n");
			return ERROR_CODE(-1);
		}
		dp = opendir(target);
		free(target);
	}
	if(dp == NULL) {
		printf("ls : cannot open directory\n");
		return ERROR_CODE(-3);
	}

	cnt = 0;
	inLoop = 1;
	while(inLoop == 1) {
		entry = readdir(dp);
		if(entry == NULL) {
			inLoop = 0;
		} else {
			if(entry->d_ino != 0) {
				printf("%s\n", entry->d_name);
			}
			cnt++;
		}
	}
	printf("Total %d files and directories\n", cnt);
	
	return NO_ERROR;	
}

// cd
error_code mycd(char * params) {
	char * _target;
	char target[BUFFER_SIZE];
	char newPath[BUFFER_SIZE];

	if(numChunk(params) > 1) {
		printf("cd : too many parameters\n");
		return ERROR_CODE(-2);
	}

	if(numChunk(params) == 0) {
		strncpy(target, "~", strlen("~"));
	} else {
		_target = getChunk(params, 0);
		strncpy(target, _target, strlen(_target));
		target[strlen(_target)] = '\0';
		free(_target);
	}
	if(*target == '~')
		*target = '/';
	
	if(chdir(target) != 0) {
		printf("cd : error occured\n");
		return ERROR_CODE(-1);
	}
	return NO_ERROR;
}
	
// make directory
error_code mymkdir(char * params) {
	char * _target;
	char target[BUFFER_SIZE];
	char newPath[BUFFER_SIZE];
	int mode;
	int temp, oth, grp, usr;

	if(numChunk(params) > 2) {
		printf("mkdir : too many parameters\n");
		return ERROR_CODE(-2);
	} else if(numChunk(params) == 0) {
		printf("mkdir : few parameters\n");
		return ERROR_CODE(-1);
	}

	if(numChunk(params) == 2) {
		_target = getChunk(params, 0);
		strncpy(target, _target, strlen(_target));
		target[strlen(_target)] = '\0';
		free(_target);
		temp = atoi(target);
		
		_target = getChunk(params, 1);
		strncpy(target, _target, strlen(_target));
		target[strlen(_target)] = '\0';
		free(_target);
	} else {
		_target = getChunk(params, 0);
		strncpy(target, _target, strlen(_target));
		target[strlen(_target)] = '\0';
		free(_target);
		temp = 755;
	}
	oth = temp % 1000 / 100;
	grp = temp % 100 / 10;
	usr = temp % 10;	
	mode = oth | (grp << 3) | (usr << 6);
	
	if(mkdir(target, (mode_t)mode) != 0) {
		printf("mkdir : error occured\n");
		return ERROR_CODE(-3);
	}
	return NO_ERROR;
}

// remove directory
error_code myrmdir(char * params) {
	char * _target;
	char target[BUFFER_SIZE];
	char newPath[BUFFER_SIZE];

	if(numChunk(params) > 1) {
		printf("rmdir : too many parameters\n");
		return ERROR_CODE(-2);
	} else if(numChunk(params) == 0) {
		printf("rmdir : few parameters\n");
		return ERROR_CODE(-1);
	}

	_target = getChunk(params, 0);
	strncpy(target, _target, strlen(_target));
	target[strlen(_target)] = '\0';
	free(_target);

	if(rmdir(target) != 0) {
		printf("rmdir : error occured\n");
		return ERROR_CODE(-3);
	}
	return NO_ERROR;
}

// remove file
error_code myrm(char * params) {
	char * _target;
	char target[BUFFER_SIZE];
	char newPath[BUFFER_SIZE];

	if(numChunk(params) > 1) {
		printf("rm : too many parameters\n");
		return ERROR_CODE(-2);
	} else if(numChunk(params) == 0) {
		printf("rm : few parameters\n");
		return ERROR_CODE(-1);
	}

	_target = getChunk(params, 0);
	strncpy(target, _target, strlen(_target));
	target[strlen(_target)] = '\0';
	free(_target);

	if(unlink(target) != 0) {
		printf("rm : error occured\n");
		return ERROR_CODE(-3);
	}
	return NO_ERROR;
}

// find function for ftw
void findfn(const char * fpath, const struct stat *sb, int typeflag) {
	char buffer[BUFFER_SIZE];
	const char *filename;
	filename = fpath;
	filename = filename + (int)strlen(fpath);
	while(*filename != '/' && filename != fpath) {
		filename--;
	}
	if(*filename == '/') {
		filename++;
	}
	if((strncmp(filename, find_target, strlen(find_target)) == 0) && (strlen(filename) == strlen(find_target))) {
		printf("[FOUND] %s (@ %s)\n", fpath, realpath(fpath, buffer));
	}
}

// find a file
error_code myfind(char * params) {
	char * path;
	struct dirent * entry;
	DIR * dp;
	
	if(numChunk(params) > 2) {
		printf("find : too many parameters\n");
		return ERROR_CODE(-2);
	} else if(numChunk(params) < 2) {
		printf("find : few parameters\n");
		return ERROR_CODE(-1);
	}
	path = getChunk(params, 0);
	find_target = getChunk(params, 1);
	if(path == NULL) {
		printf("find : wrong parameter\n");
		return ERROR_CODE(-3);
	}

	if(ftw(path, findfn, OPEN_MAX) != 0) {
		printf("find : error occured\n");
		free(find_target);
		free(path);
		return ERROR_CODE(-4);
	}
	free(find_target);
	free(path);
	return NO_ERROR;	
}

// pwd
error_code mypwd(char * params) {
	error_code retError;
	char * path;
	if(numChunk(params) > 0) {
		printf("pwd : too many parameters\n");
		return ERROR_CODE(-2);
	}

	path = presentPath(&retError);

	if(retError != NO_ERROR) {
		printf("pwd : error occured\n");
	} else {
		printf("%s\n",path);
	}
	if(path != NULL)
		free(path);
	return retError;
}

// get present path
char * presentPath(error_code * error) {
	char * buf = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	char * cwd_error;
	cwd_error = getcwd(buf, BUFFER_SIZE);
	if(cwd_error == NULL)
		*error = ERROR_CODE(-1);
	else
		*error = NO_ERROR;
	return buf;
}

// run shell script
error_code mysh(char * params) {
	FILE * fp;
	struct stat file_stat;
	char * path;
	char filter[4];
	char buffer[BUFFER_SIZE];
	if(numChunk(params) > 1) {
		printf("script : too many parameters\n");
		return ERROR_CODE(-1);
	}
	if(numChunk(params) == 0) {
		printf("script : few parameters\n");
		return ERROR_CODE(-2);
	}
	path = getChunk(params, 0);
	if(lstat(path, &file_stat) != 0) {
		printf("script : error occured\n");
		free(path);
		return ERROR_CODE(-3);
	}
	if((file_stat.st_mode & S_IFMT) == S_IFREG) {
		fp = fopen(path, "r");
		if(fp != NULL) {
			fscanf(fp, "%c%c%c%c", &filter[0], &filter[1], &filter[2], &filter[3]);
			if(((filter[0] == 0x7f) && (filter[1] == 'E') && (filter[2] == 'O') && (filter[3] == 'F')) == 0) {
				rewind(fp);
				while(fgets(buffer, BUFFER_SIZE, fp) != NULL) {
					if(buffer[strlen(buffer) - 1] == '\n')
						buffer[strlen(buffer) - 1] = '\0';
					execute_command(buffer);
				}
			} else {
				free(path);
				printf("%s is for execution\n", path);
				return ERROR_CODE(-4);
			}
			fclose(fp);
		} else {
			free(path);
			printf("cannot open file\n", path);
			return ERROR_CODE(-5);
		}
	} else {
		free(path);
		printf("%s is not regular file\n", path);
		return ERROR_CODE(-6);
	}
	free(path);
	return NO_ERROR;
}
error_code myexecute(char * path, char * params) {
	pid_t pid;
	int chunks;
	FILE * fp;
	char magic[4];
	char * chunk;
	int pd[2];
	int status;
	int isBg;
	char * prmlist[BUFFER_SIZE];

	// get the number of chunks
	chunks = numChunk(params);

	if(chunks > 0) {
		int num_pipedParams = -1;
		int i;
		char * pipedParams = (char *)malloc(sizeof(char)*BUFFER_SIZE);
		int error;
		pipedParams[0] = '\0';
		for(i=0; i<chunks; i++) {
			chunk = getChunk(params, i);
			if(num_pipedParams >= 0) {
				num_pipedParams = num_pipedParams + 1;
				strncat(pipedParams, chunk, strlen(chunk));
				strncat(pipedParams, " ", strlen(" "));
			} else {
				if(IS_EQUAL(chunk, "|")) {
					num_pipedParams = 0;
				}
			}
			free(chunk);
		}
		if(num_pipedParams > 0) {
			char * path_piped, * params_piped;
			int i;
			int length = strlen(params)-1;
			for(i=0; i<length; i++) {
				if(params[i] == '|' && params[i+1] == ' ') {
					params[i] = '\0';
				}
			}
			path_piped = (char *)malloc(sizeof(char)*BUFFER_SIZE);
			params_piped = (char *)malloc(sizeof(char)*BUFFER_SIZE);
			path_piped[0] = '\0';
			params_piped[0] = '\0';
			chunk = getChunk(pipedParams, 0);
			strncat(path_piped, chunk, strlen(chunk));
			free(chunk);
			num_pipedParams = numChunk(pipedParams);
			for(i=1; i<num_pipedParams; i++) {
				chunk = getChunk(pipedParams, 1);
				strncat(params_piped, chunk, strlen(chunk));
				free(chunk);
			}
			error = myexecute_grp(path, params, path_piped, params_piped);
			free(pipedParams);
			free(path_piped);
			free(params_piped);
			return error;
		}
		free(pipedParams);
	}

	{
		int i;
		prmlist[0] = (char *)malloc(sizeof(char)*strlen(path)+1);
		if(prmlist[0] == NULL) {
			return ERROR_CODE(-5);
		}
		strncpy(prmlist[0], path, strlen(path));
		prmlist[0][strlen(path)] = '\0';
		for(i=0;i<chunks;i++) {
			chunk = getChunk(params, i);
			prmlist[i+1] = (char *)malloc(sizeof(char)*(strlen(chunk)+1));
			if(prmlist[i+1] == NULL) {
				return ERROR_CODE(-5);
			}
			strncpy(prmlist[i+1], chunk, strlen(chunk));
			prmlist[i+1][strlen(chunk)] = '\0';
		}
	}

	// determine if this command will be executed on background process
	isBg = 0;
	if(chunks > 0) {
		chunk = getChunk(params, chunks-1);
		if(IS_EQUAL(chunk,"&")) {
			if(strlen(params)-2 > 0) {
				params[strlen(params)-2] = '\0';
			} else {
				params[0] = '\0';
			}
			isBg = 1;
		} else {
			isBg = 0;
		}
	}

	// check if the user has the executing permission
	if(access(path, X_OK) != 0) {
		printf("%s : cannot execute the program\n", path);
		return ERROR_CODE(-1);
	}

	// check if the file is in ELF.
	// if this is not in ELF, try to run as text script
	fp = fopen(path, "r");
	fscanf(fp, "%c%c%c%c", &magic[0], &magic[1], &magic[2], &magic[3]);
	if(magic[0] != 0x7f || magic[1] != 'E' || magic[2] != 'L' || magic[3] != 'F') {
		return mysh(path);
	}
	fclose(fp);

	// make pipe
	if(pipe(pd) == -1) {
		printf("pipe creation failed\n");
	}
	// fork process
	pid = fork();
	if(pid < 0) {
		// fork error
		printf("Fork error\n");
		return ERROR_CODE(-2);
	} else if(pid == 0) {
		int result;
		// child process
		// if this is in ELF, try to execute this 'executable' file
		if(prmlist[0] == NULL) {
			result = execvp(path, NULL);
		} else {
			result = execvp(path, prmlist); 
		}
		if(result == -1) {
			printf("Command exec error\n");
		}
		exit(127);
	} else {
		// mother process
		if(isBg == 0) {
			waitpid(pid, &status, WUNTRACED);
		}
	}
	return NO_ERROR;
}
error_code myexecute_grp(char * path, char * params, char * path_piped, char *params_piped) {
	pid_t pid_first, pid_second;
	int chunks;
	FILE * fp;
	char magic[4];
	char * chunk;
	int pd[2];
	int status;
	char * prmlist[BUFFER_SIZE];
	char * prmlist_piped[BUFFER_SIZE];
	
	// get the number of chunks
	chunks = numChunk(params);

	// check if the user has the executing permission
	if(access(path, X_OK) != 0) {
		printf("%s : cannot execute the program\n", path);
		return ERROR_CODE(-1);
	}
	if(access(path_piped, X_OK) != 0) {
		printf("%s : cannot execute the program\n", path_piped);
		return ERROR_CODE(-1);
	}

	// check if the file is in ELF.
	// if this is not in ELF, error occured
	fp = fopen(path, "r");
	fscanf(fp, "%c%c%c%c", &magic[0], &magic[1], &magic[2], &magic[3]);
	if(magic[0] != 0x7f || magic[1] != 'E' || magic[2] != 'L' || magic[3] != 'F') {
		return mysh(params);
	}
	fclose(fp);
	fp = fopen(path_piped, "r");
	fscanf(fp, "%c%c%c%c", &magic[0], &magic[1], &magic[2], &magic[3]);
	if(magic[0] != 0x7f || magic[1] != 'E' || magic[2] != 'L' || magic[3] != 'F') {
		return mysh(params);
	}
	fclose(fp);

	{
		int i;
		prmlist[0] = (char *)malloc(sizeof(char)*strlen(path)+1);
		if(prmlist[0] == NULL) {
			return ERROR_CODE(-5);
		}
		strncpy(prmlist[0], path, strlen(path));
		prmlist[0][strlen(path)] = '\0';
		chunks = numChunk(params);
		for(i=0;i<chunks;i++) {
			chunk = getChunk(params, i);
			prmlist[i+1] = (char *)malloc(sizeof(char)*(strlen(chunk)+1));
			if(prmlist[i+1] == NULL) {
				return ERROR_CODE(-5);
			}
			strncpy(prmlist[i+1], chunk, strlen(chunk));
			prmlist[strlen(chunk)] = '\0';
		}
	}
	{
		int i;
		prmlist_piped[0] = (char *)malloc(sizeof(char)*strlen(path_piped)+1);
		if(prmlist_piped[0] == NULL) {
			return ERROR_CODE(-5);
		}
		strncpy(prmlist_piped[0], path_piped, strlen(path_piped));
		prmlist_piped[0][strlen(path_piped)] = '\0';
		chunks = numChunk(params_piped);
		for(i=0;i<chunks;i++) {
			chunk = getChunk(params_piped, i);
			prmlist_piped[i+1] = (char *)malloc(sizeof(char)*(strlen(chunk)+1));
			if(prmlist_piped[i+1] == NULL) {
				return ERROR_CODE(-5);
			}
			strncpy(prmlist_piped[i+1], chunk, strlen(chunk));
			prmlist_piped[strlen(chunk)] = '\0';
		}
	}

	// make pipe
	if(pipe(pd) == -1) {
		printf("pipe creation failed\n");
	}

	// fork first process
	pid_first = fork();
	if(pid_first < 0) {
		// fork error
		printf("Fork error\n");
		return ERROR_CODE(-2);
	} else if(pid_first == 0) {
		int result;
		// child process
		dup2(pd[1], 1);
		close(pd[0]);
		close(pd[1]);
		// if this is in ELF, try to execute this 'executable' file
		if(prmlist[0] == NULL) {
			result = execvp(path, NULL);
		} else {
			result = execvp(path, prmlist); 
		}
		if(result == -1) {
			printf("Command exec error\n");
		}
		exit(127);
	} else {
		// mother process
		// fork second process
		pid_second = fork();
		if(pid_second < 0) {
			// fork error
			printf("Fork error\n");
			return ERROR_CODE(-2);
		} else if(pid_second == 0) {
			int result;
			// child process
			dup2(pd[0], 0);
			close(pd[0]);
			close(pd[1]);
			// if this is in ELF, try to execute this 'executable' file
			if(prmlist_piped[0] == NULL) {
				result = execvp(path_piped, NULL);
			} else {
				result = execvp(path_piped, prmlist_piped); 
			}
			if(result == -1) {
				printf("Command exec error\n");
			}
			exit(127);
		} else {
			// mother process
			close(pd[0]);
			close(pd[1]);
			waitpid(pid_first, &status, WUNTRACED);
			waitpid(pid_second, &status, WUNTRACED);
		}
	}
}
