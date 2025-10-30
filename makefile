CC=gcc
CFLAGS=-Wall -Wextra -std=c11 -O2 -g
CPPFLAGS=-D_POSIX_C_SOURCE=200809L
LDFLAGS=
LIBS=

OBJS=tokens.o utils.o commands.o builtins.o history.o main.o ansi.o

all: unix-shell

unix-shell: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)

tokens.o: tokens.c tokens.h
utils.o: utils.c utils.h history.h ansi.h
commands.o: commands.c commands.h tokens.h
builtins.o: builtins.c builtins.h
history.o: history.c history.h
ansi.o : ansi.h
main.o: main.c utils.h commands.h tokens.h builtins.h history.h ansi.h

clean:
	rm -f $(OBJS) unix-shell
