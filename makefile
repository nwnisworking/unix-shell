CFLAGS=-Wall -Wextra -std=c11 -O2 -g
LDFLAGS=
LIBS=

tokens.o: tokens.c tokens.h
	gcc $(CFLAGS) -c tokens.c -o tokens.o
utils.o: utils.c utils.h
	gcc $(CFLAGS) -c utils.c -o utils.o
commands.o: commands.c commands.h tokens.h
	gcc $(CFLAGS) -c commands.c -o commands.o
history.o: history.c history.h
	gcc $(CFLAGS) -c history.c -o history.o
builtins.o: builtins.c builtins.h
	gcc $(CFLAGS) -c builtins.c -o builtins.o
main.o: main.c utils.h commands.h tokens.h builtins.h history.h
	gcc $(CFLAGS) -c main.c -o main.o

unix-shell: main.o utils.o commands.o tokens.o builtins.o history.o
	gcc $(CFLAGS) main.o utils.o commands.o tokens.o builtins.o history.o -o unix-shell $(LDFLAGS) $(LIBS)
