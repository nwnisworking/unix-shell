#ifndef COMMANDS_H
#define COMMANDS_H

#include <string.h>
#include <stdio.h>

#include "tokens.h"

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

/**
 * Searches for input and output redirections in the command tokens.
 * 
 * @param tokens An array of token strings.
 * @param command A pointer to the Command structure to update with redirection info.
 */
void searchRedirections(char* tokens[], Command* command);

/**
 * Builds the argv array for a command from its tokens.
 * 
 * @param tokens An array of token strings.
 * @param command A pointer to the Command structure to build argv for.
 */
void buildArgvArray(char* tokens[], Command* command);

/**
 * Clears the command structures in the provided array.
 * 
 * @param commands An array of Command structures to clear.
 */
void clearCommands(Command commands[]);

#endif // COMMANDS_H