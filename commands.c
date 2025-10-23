#include "commands.h"

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
  
  if(tokens[last][0] == PIPE_SEP) return -4;

  for(i = 0; i < c; i++){
    searchRedirections(tokens, &commands[i]);
    buildArgvArray(tokens, &commands[i]);
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

void searchRedirections(char* tokens[], Command* command){
  for(int i = command->first; i <= command->last; i++){
    char* token = tokens[i];
    
    if(token[0] == REDIRECT_IN){
      if(tokens[i + 1] != NULL)
        command->stdin_file = tokens[i + 1];

      i++;
    }
    else if(token[0] == REDIRECT_OUT){
      if(tokens[i + 1] != NULL)
        command->stdout_file = tokens[i + 1];

      i++;
    }
  }
}

void buildArgvArray(char* tokens[], Command* command){
  int n = (command->last - command->first + 1) - 
  (command->stdin_file ? 2 : 0) - 
  (command->stdout_file ? 2 : 0) + 
  1;

  command->argv = (char**) realloc(command->argv, n * sizeof(char*));

  if(command->argv == NULL){
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  int k = 0;

  for(int i = command->first; i <= command->last; i++){
    char* token = tokens[i];

    if(token[0] == REDIRECT_IN || token[0] == REDIRECT_OUT){
      i++;
    }
    else{
      command->argv[k] = tokens[i];
      k++;
    }
  }

  command->argv[k] = NULL;
}

void clearCommands(Command commands[]){
  for(int i = 0; i < MAX_NUM_COMMANDS; i++){
    commands[i].first = 0;
    commands[i].last = 0;
    commands[i].sep = NULL;
    commands[i].stdin_file = NULL;
    commands[i].stdout_file = NULL;

    if(commands[i].argv != NULL){
      free(commands[i].argv);
      commands[i].argv = NULL;
    }
  }
}