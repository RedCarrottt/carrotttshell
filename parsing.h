#ifndef _PARSING_H_
#define _PARSING_H_

#define IS_EQUAL(a,b) (((strncmp(a,b,strlen(b))) == 0) && (strlen(a) == strlen(b)))
typedef enum { 
	SYM_PIPE,
	SYM_BG,
	SYM_NORMAL
} SYMBOL;

int numChunk(char *);
char * getChunk(char *, int);
SYMBOL chunkToSymbol(char *);

#endif
