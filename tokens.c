/// tokens.c
#include "tokens.h"

#define REDIRECT_ERR2 "2>"    // two-character stderr redirection token

// Tokenize input into tokens[] in-place by inserting NULs and strdup'ing
// single-char separators (& ; | < >) and the two-char token "2>".
// Supports quotes (' "..."), backslash escapes, and leaves quoted content
// as a single token without the surrounding quotes.
int tokenise(char* input, char* tokens[]){
  int count = 0;
  char* ptr = input;
  int is_token = 0;   // currently inside a word

  while (*ptr != '\0') {

    // --- handle the two-character token "2>" BEFORE single-char checks
    if (*ptr == '2' && *(ptr + 1) == REDIRECT_OUT) {
      if (is_token) {
        *ptr = '\0';
        is_token = 0;
      }
      if (count >= MAX_TOKENS) return -1;
      tokens[count++] = strdup(REDIRECT_ERR2);
      ptr += 2;
      continue;
    }

    switch (*ptr) {
      // whitespace → terminate current token if any
      case TAB_SEP:
      case NEWLINE_SEP:
      case SPACE_SEP: {
        *ptr = '\0';
        is_token = 0;
        break;
      }

      // single-char separators: | & ; < >
      case PIPE_SEP:
      case CONCURRENT_SEP:
      case SEQUENCE_SEP:
      case REDIRECT_IN:
      case REDIRECT_OUT: {
        char sep = *ptr;

        if (is_token) {
          *ptr = '\0';
          is_token = 0;
        }

        if (count >= MAX_TOKENS) return -1;
        tokens[count++] = strdup((char[]){ sep, '\0' });
        break;
      }

      // backslash escape: include next char literally (if any)
      case ESCAPE_CHAR: {
        if (*(ptr + 1) != '\0') {
          // remove the backslash by shifting left
          memmove(ptr, ptr + 1, strlen(ptr)); // copies including trailing NUL
          // do not advance ptr here; we want to process the now-shifted char
          continue;
        }
        break;
      }

      // quoted strings: keep content as a single token
      case SINGLE_QUOTE:
      case DOUBLE_QUOTE: {
        char quote = *ptr;
        char* quote_start = ptr + 1;

        // terminate any token before the quote
        if (is_token) {
          *ptr = '\0';
          is_token = 0;
        }

        // scan forward to matching quote, honouring backslash-escaped quote
        char *p = quote_start;
        while (*p != '\0' && *p != quote) {
          if (*p == ESCAPE_CHAR && *(p + 1) == quote) {
            // remove the backslash, keep the quote
            memmove(p, p + 1, strlen(p));
          }
          p++;
        }

        if (*p == quote) {
          // close quote found; turn it into NUL to isolate the token
          *p = '\0';
          if (count >= MAX_TOKENS) return -1;
          tokens[count++] = quote_start;
          // move ptr to end of token; loop increment will advance past NUL
          ptr = p;
        } else {
          // no closing quote — treat the opening quote as a normal char
          // fall back by converting the opening quote to a normal char
          // and let default handler pick it up as part of a token
        }
        break;
      }

      default: {
        // start of a new token?
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

// Free dynamically allocated separator tokens and clear the array.
void clearTokens(char* tokens[]){
  for (int i = 0; i < MAX_TOKENS; i++) {
    char* token = tokens[i];
    if (token == NULL) break;

    // free strdup'ed separators ("|", "&", ";", "<", ">", "2>")
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
