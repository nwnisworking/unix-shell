#include <stdio.h>
#include "utils.h"
#include "tokens.h"
#include "commands.h"

#define MAX_LINE_LENGTH 1024

int main(){
  char line[MAX_LINE_LENGTH];
  char* tokens[MAX_TOKENS] = {0};
  Command commands[MAX_NUM_COMMANDS] = {0};
  size_t len;

  while(1){
    // Clean up tokens and commands from previous iteration
    clearTokens(tokens);
    clearCommands(commands);

    printf("$ ");

    if(!fgets(line, MAX_LINE_LENGTH, stdin)) break;

    len = getInput(line);

    if(strcmp(line, "exit") == 0) break;

    int num_tokens = tokenise(line, tokens);
    int num_commands = separateCommands(tokens, commands);

    printf("Number of commands: %d\n", num_commands);
    for(int i = 0; i < num_commands; i++){
      printf(
        "Command %d: first=%d, last=%d, sep=%s\n stdin=%s\n stdout=%s\n",
        i,
        commands[i].first,
        commands[i].last,
        commands[i].sep,
        commands[i].stdin_file,
        commands[i].stdout_file
      );

      for(int j = 0; commands[i].argv != NULL && commands[i].argv[j] != NULL; j++){
        printf("  argv[%d]: %s\n", j, commands[i].argv[j]);
      }
    }
  }

  return 0;
}