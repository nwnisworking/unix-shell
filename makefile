tokens.o: tokens.c tokens.h
	gcc -c tokens.c -o tokens.o
utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o
commands.o: commands.c commands.h tokens.h
	gcc -c commands.c -o commands.o
main.o: main.c utils.h commands.h tokens.h builtins.h
	gcc -c main.c -o main.o
builtins.o: builtins.c builtins.h
	gcc -c builtins.c -o builtins.o

unix-shell: main.o utils.o commands.o tokens.o builtins.o
	gcc main.o utils.o commands.o tokens.o builtins.o -o unix-shell