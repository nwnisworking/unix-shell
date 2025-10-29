/// tokens.c
#include "tokens.h"
#include <string.h>
#include <stdlib.h>

#define REDIRECT_ERR2 "2>"    // ensure consistency with header

int tokenise(char* input, char* tokens[]){
  int count = 0;
  char* ptr = input;
  int is_token = 0;   // currently inside a word

  while (*ptr != '\0') {

    // two-char token "2>"
    if (*ptr == '2' && *(ptr + 1) == REDIRECT_OUT) {
      if (is_token) { *ptr = '\0'; is_token = 0; }
      if (count >= MAX_TOKENS) return -1;
      tokens[count++] = strdup(REDIRECT_ERR2);
      ptr += 2;
      continue;
    }

    switch (*ptr) {
      case TAB_SEP:
      case NEWLINE_SEP:
      case SPACE_SEP: {
        *ptr = '\0';
        is_token = 0;
        break;
      }

      case PIPE_SEP:
      case CONCURRENT_SEP:
      case SEQUENCE_SEP:
      case REDIRECT_IN:
      case REDIRECT_OUT: {
        char sep = *ptr;
        if (is_token) { *ptr = '\0'; is_token = 0; }
        if (count >= MAX_TOKENS) return -1;
        tokens[count++] = strdup((char[]){ sep, '\0' });
        break;
      }

      case ESCAPE_CHAR: {
        if (*(ptr + 1) != '\0') {
          // remove backslash, keep next char
          memmove(ptr, ptr + 1, strlen(ptr)); // includes trailing NUL
          continue; // reprocess the shifted char at same ptr
        }
        break;
      }

      case SINGLE_QUOTE:
      case DOUBLE_QUOTE: {
        char quote = *ptr;
        char* start = ptr + 1;

        if (is_token) { *ptr = '\0'; is_token = 0; }

        char *p = start;
        while (*p != '\0' && *p != quote) {
          if (*p == ESCAPE_CHAR && *(p + 1) == quote) {
            memmove(p, p + 1, strlen(p)); // drop backslash
          }
          p++;
        }

        if (*p == quote) {
          *p = '\0';
          if (count >= MAX_TOKENS) return -1;
          tokens[count++] = start;
          ptr = p; // loop increment moves past NUL
        }
        // if no closing quote, fall through and treat as normal chars
        break;
      }

      default: {
        if (!is_token) {
          if (count >= MAX_TOKENS) return -1;
          tokens[count++] = ptr;
          is_token = 1;
        }
        break;
      }
    }

    ptr++;
  }

  return count;
}

void clearTokens(char* tokens[]){
  for (int i = 0; i < MAX_TOKENS; i++) {
    char* token = tokens[i];
    if (token == NULL) break;

    if ((strlen(token) == 1 && (
          token[0] == PIPE_SEP ||
          token[0] == CONCURRENT_SEP ||
          token[0] == SEQUENCE_SEP ||
          token[0] == REDIRECT_IN ||
          token[0] == REDIRECT_OUT)) ||
        strcmp(token, REDIRECT_ERR2) == 0)
    {
      free(token);
    }
    tokens[i] = NULL;
  }
}
