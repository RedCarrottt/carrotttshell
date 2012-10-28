shell : shell.c commands.c parsing.c
	gcc -o shell shell.c commands.c parsing.c
clean : 
	rm shell
