#include "commands.h"
#include <stdlib.h>

#define REDIRECT_ERR2 "2>"

static int is_redir_token(const char *t){
  if (!t) return 0;
  if ((strlen(t) == 1) && (t[0] == REDIRECT_IN || t[0] == REDIRECT_OUT)) return 1;
  if (strcmp(t, REDIRECT_ERR2) == 0) return 1;
  return 0;
}

int separateCommands(char* tokens[], Command commands[]){
  int i = 0, tokens_size = 0;
  while (tokens[i] != NULL) i++;
  tokens_size = i;

  if (tokens_size == 0) return 0;
  if (isSeparator(tokens[0])) return -3;

  // Append ';' sentinel if last token isn't a separator
  if (!isSeparator(tokens[tokens_size - 1])) {
    tokens[tokens_size] = strdup((char[]){ SEQUENCE_SEP, '\0' });
    tokens_size++;
  }

  int first = 0, last = 0, c = 0;

  for (i = 0; i < tokens_size; i++) {
    last = i;
    if (isSeparator(tokens[i])) {
      if (first == last) return -2; // empty command between seps

      commands[c].first = first;
      commands[c].last  = last - 1;
      commands[c].sep   = tokens[i];
      commands[c].argv  = NULL;
      commands[c].stdin_file  = NULL;
      commands[c].stdout_file = NULL;
      commands[c].stderr_file = NULL;
      c++;

      first = i + 1;
    }
  }

  if (tokens[last][0] == PIPE_SEP) return -4; // ends with pipe

  for (i = 0; i < c; i++) {
    searchRedirections(tokens, &commands[i]);
    buildArgvArray(tokens, &commands[i]);
  }

  return c;
}

int isSeparator(char* token){
  if (token == NULL) return 0;
  if (strlen(token) == 1) {
    char separators[] = { PIPE_SEP, CONCURRENT_SEP, SEQUENCE_SEP, '\0' };
    for (int i = 0; separators[i] != '\0'; i++) {
      if (separators[i] == token[0]) return 1;
    }
  }
  return 0;
}

void searchRedirections(char* tokens[], Command* command){
  for (int i = command->first; i <= command->last; i++) {
    char* token = tokens[i];
    if (!token) continue;

    if (token[0] == REDIRECT_IN && token[1] == '\0') {
      if (i + 1 <= command->last && tokens[i + 1] != NULL) {
        command->stdin_file = tokens[i + 1];
        i++;
      }
    }
    else if (token[0] == REDIRECT_OUT && token[1] == '\0') {
      if (i + 1 <= command->last && tokens[i + 1] != NULL) {
        command->stdout_file = tokens[i + 1];
        i++;
      }
    }
    else if (strcmp(token, REDIRECT_ERR2) == 0) {
      if (i + 1 <= command->last && tokens[i + 1] != NULL) {
        command->stderr_file = tokens[i + 1];
        i++;
      }
    }
  }
}

void buildArgvArray(char* tokens[], Command* command){
  int pairs =
    (command->stdin_file  ? 1 : 0) +
    (command->stdout_file ? 1 : 0) +
    (command->stderr_file ? 1 : 0);

  int span = (command->last - command->first + 1);
  int n = span - (pairs * 2) + 1; // +1 for NULL
  if (n < 1) n = 1;

  command->argv = (char**) realloc(command->argv, n * sizeof(char*));
  if (command->argv == NULL) {
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  int k = 0;
  for (int i = command->first; i <= command->last; i++) {
    char* token = tokens[i];
    if (is_redir_token(token)) { i++; continue; } // skip file after redir
    command->argv[k++] = tokens[i];
  }
  command->argv[k] = NULL;
}

void clearCommands(Command commands[]){
  for (int i = 0; i < MAX_NUM_COMMANDS; i++) {
    commands[i].first = 0;
    commands[i].last = 0;
    commands[i].sep = NULL;
    commands[i].stdin_file = NULL;
    commands[i].stdout_file = NULL;
    commands[i].stderr_file = NULL;

    if (commands[i].argv != NULL) {
      free(commands[i].argv);
      commands[i].argv = NULL;
    }
  }
}
