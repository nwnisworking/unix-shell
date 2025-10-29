#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>
#include <stdio.h>
#include "tokens.h"

#define MAX_NUM_COMMANDS 1000

typedef struct Command{
  int   first;
  int   last;
  char* sep;
  char** argv;
  char* stdin_file;
  char* stdout_file;
  char* stderr_file;   // support for "2>"
} Command;

int separateCommands(char* tokens[], Command commands[]);
int isSeparator(char* token);
void searchRedirections(char* tokens[], Command* command);
void buildArgvArray(char* tokens[], Command* command);
void clearCommands(Command commands[]);

#endif // COMMANDS_H
