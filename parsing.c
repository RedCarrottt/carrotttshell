#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsing.h"
#include "shell.h"

int numChunk(char * params) {
	int n_chunks;
	int n_chars;
	char * p;
	int inLoop;
	
	p = params;
	inLoop = 1;
	n_chunks = 0;
	n_chars = 0;
	while(inLoop) {
		if(*p == '\0' || *p == ' ') {
			if(n_chars != 0) {
				n_chunks++;
			}
			if(*p == '\0') {
				inLoop = 0;
			}
			n_chars = 0;
		} else {
			n_chars++;
		}
		p++;
	}
	return n_chunks;
}
char * getChunk(char * params, int index) {
	int n_chars;
	int n_chunks;
	char * chunk;
	char * p;
	int inLoop;
	chunk = (char *)malloc(sizeof(char)*BUFFER_SIZE);
	chunk[0] = '\0';
	
	n_chars = 0;
	n_chunks = 0;
	inLoop = 1;
	p = params;
	while(inLoop) {
		if(*p == '\0' || *p == ' ') {
			if(n_chars != 0 && n_chunks == index) {
				strncpy(chunk, p-n_chars, n_chars);
				chunk[n_chars] = '\0';
				inLoop = 0;
			}
			if(*p == '\0') {
				inLoop = 0;
			}
			n_chunks++;
			n_chars = 0;
		} else {
			n_chars++;
		}
		p++;
	}
	if(chunk[0] == '\0')
		chunk = NULL;
	return chunk;
}

SYMBOL chunkToSymbol(char * chunk) {
	if(IS_EQUAL(chunk, "|")) {
		return SYM_PIPE;
	} else if(IS_EQUAL(chunk, "&")) {
		return SYM_BG;
	} else {
		return SYM_NORMAL;
	}
}
