#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 100

/**
 * Tokenise the input string into an array of tokens.
 * 
 * @param input The input string to be tokenised.
 * @param tokens The array to store the pointers to the tokens.
 * @return The number of tokens extracted. -1 if the number of tokens exceeds MAX_TOKENS.
 */
int tokenise(char* input, char* tokens[]);

#endif // TOKENS_H