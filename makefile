CFLAGS := -Wall -Wextra -std=c11 -O2 -g -D_POSIX_C_SOURCE=200809L
OBJS   := tokens.o utils.o commands.o builtins.o history.o main.o

all: unix-shell

unix-shell: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) unix-shell
