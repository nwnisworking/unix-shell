#include "commands.h"
#include "tokens.h"

int separateCommands(char* tokens[], Command commands[]){
  int i = 0, tokens_size = 0;

  while(tokens[i] != NULL) i++;
  tokens_size = i;

  if(tokens_size == 0) return 0;

  if(isSeparator(tokens[0])) return -3;

  if(!isSeparator(tokens[tokens_size - 1])){
    tokens[tokens_size] = strdup((char[]){SEQUENCE_SEP, '\0'});
    tokens_size++;
  }

  int first = 0, last, c = 0;
  char* sep;

  for(i = 0; i < tokens_size; i++){
    last = i;

    if(isSeparator(tokens[i])){
      sep = tokens[i];

      if(first == last) return -2;
      commands[c].first = first;
      commands[c].last = last - 1;
      commands[c].sep = sep;
      c++;

      first = i + 1;
    }
  }
  
  return c;
}

int isSeparator(char* token){
  if(token == NULL || strlen(token) > 1) return 0;

  int i = 0;

  char separators[] = {PIPE_SEP, CONCURRENT_SEP, SEQUENCE_SEP, '\0'};

  while(separators[i] != '\0'){
    if(separators[i] == token[0]){
      return 1;
    }

    i++;
  }

  return 0;
}