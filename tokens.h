#ifndef TOKENS_H
#define TOKENS_H

/**
 * @file tokens.h
 * @brief Tokenization utilities for shell input parsing.
 *
 * Defines constants and functions for splitting raw input into tokens,
 * handling quotes, escapes, separators, and redirection symbols.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/// Maximum number of tokens allowed per input line
#define MAX_TOKENS 4096   // roomy enough for ≥1000 args

// Character constants used during tokenization
#define SINGLE_QUOTE     '\''    ///< Single quote character
#define DOUBLE_QUOTE     '\"'    ///< Double quote character
#define ESCAPE_CHAR      '\\'    ///< Escape character
#define PIPE_SEP         '|'     ///< Pipe separator
#define CONCURRENT_SEP   '&'     ///< Concurrent execution separator
#define SEQUENCE_SEP     ';'     ///< Sequential execution separator
#define TAB_SEP          '\t'    ///< Tab character
#define SPACE_SEP        ' '     ///< Space character
#define NEWLINE_SEP      '\n'    ///< Newline character
#define REDIRECT_IN      '<'     ///< Input redirection
#define REDIRECT_OUT     '>'     ///< Output redirection
#define REDIRECT_ERR2    "2>"    ///< Stderr redirection token

/**
 * @brief Splits raw input into tokens.
 *
 * Parses the input string and populates the tokens array with pointers
 * to individual tokens. Handles quotes, escapes, and redirection symbols.
 *
 * @param input Raw input string to tokenize.
 * @param tokens Array to store token pointers.
 * @return Number of tokens parsed, or -1 on overflow.
 */
int tokenise(char* input, char* tokens[]);

/**
 * @brief Frees and clears token array.
 *
 * Releases memory for dynamically allocated tokens and resets all entries to NULL.
 *
 * @param tokens Array of token pointers to clear.
 */
void clearTokens(char* tokens[]);

#endif // TOKENS_H