#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKENS 100

#define PIPE_SEP '|'
#define CONCURRENT_SEP '&'
#define SEQUENCE_SEP ';'
#define TAB_SEP '\t'
#define SPACE_SEP ' '
#define NEWLINE_SEP '\n'
#define REDIRECT_IN '<'
#define REDIRECT_OUT '>'

/**
 * Tokenise the input string into an array of tokens.
 * 
 * @param input The input string to be tokenised.
 * @param tokens The array to store the pointers to the tokens.
 * @return The number of tokens extracted. -1 if the number of tokens exceeds MAX_TOKENS.
 */
int tokenise(char* input, char* tokens[]);

/**
 * Clear the tokens array, freeing any dynamically allocated memory.
 * 
 * @param tokens The array of tokens to be cleared.
 */
void clearTokens(char* tokens[]);

#endif // TOKENS_H