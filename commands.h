#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>
#include <stdio.h>

#define MAX_NUM_COMMANDS 1000

typedef struct Command{
  int first;
  int last;
  char* sep;
  char** argv;
  char* stdin_file;
  char* stdout_file;
} Command;

/**
 * Separates the tokens into commands based on command separators.
 * 
 * @param tokens An array of token strings.
 * @param commands An array to hold the separated commands.
 * @return The number of commands separated, or a negative value on error.
 */
int separateCommands(char* tokens[], Command commands[]);

/**
 * Checks if a token is a command separator.
 * 
 * @param token The token string to check.
 * @return 1 if the token is a separator, 0 otherwise.
 */
int isSeparator(char* token);

#endif // COMMANDS_H