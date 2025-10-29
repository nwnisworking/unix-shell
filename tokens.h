#ifndef TOKENS_H
#define TOKENS_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_TOKENS 4096   // roomy enough for ≥1000 args

#define SINGLE_QUOTE '\''
#define DOUBLE_QUOTE '\"'
#define ESCAPE_CHAR  '\\'
#define PIPE_SEP     '|'
#define CONCURRENT_SEP '&'
#define SEQUENCE_SEP ';'
#define TAB_SEP      '\t'
#define SPACE_SEP    ' '
#define NEWLINE_SEP  '\n'
#define REDIRECT_IN  '<'
#define REDIRECT_OUT '>'
#define REDIRECT_ERR2 "2>"   // stderr redirection token

int tokenise(char* input, char* tokens[]);
void clearTokens(char* tokens[]);

#endif // TOKENS_H
