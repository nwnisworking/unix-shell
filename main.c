#include <stdio.h>
#include "utils.h"
#include "tokens.h"
#include "commands.h"

#define MAX_LINE_LENGTH 1024

int main(){
  char line[MAX_LINE_LENGTH];
  char* tokens[MAX_TOKENS] = {0};
  Command commands[MAX_NUM_COMMANDS];
  size_t len;

  while(1){
    // Clean up tokens from previous iteration
    clearTokens(tokens);

    printf("$ ");

    if(!fgets(line, MAX_LINE_LENGTH, stdin)) break;

    len = getInput(line);

    if(strcmp(line, "exit") == 0) break;

    int num_tokens = tokenise(line, tokens);
    int num_commands = separateCommands(tokens, commands);

    printf("Number of commands: %d\n", num_commands);
    for(int i = 0; i < num_commands; i++){
      printf("Command %d: first=%d, last=%d, sep=%s\n", i, commands[i].first, commands[i].last, commands[i].sep);
    }
  }

  clearTokens(tokens);

  return 0;
}