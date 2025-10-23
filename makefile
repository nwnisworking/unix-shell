tokens.o: tokens.c tokens.h
	gcc -c tokens.c -o tokens.o
utils.o: utils.c utils.h
	gcc -c utils.c -o utils.o
commands.o: commands.c commands.h tokens.h
	gcc -c commands.c -o commands.o
main.o: main.c utils.h commands.h tokens.h
	gcc -c main.c -o main.o
unix-shell: main.o utils.o commands.o tokens.o
	gcc main.o utils.o commands.o tokens.o -o unix-shell