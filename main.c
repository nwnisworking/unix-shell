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

  char prompt[MAX_LINE_LENGTH] = "%";

  while(1){
    // Clean up tokens and commands from previous iteration
    clearTokens(tokens);
    clearCommands(commands);

    printf("%s ", prompt);

    if(!fgets(line, MAX_LINE_LENGTH, stdin)) break;

    len = getInput(line);

    int num_tokens = tokenise(line, tokens);
    int num_commands = separateCommands(tokens, commands);

    // Debug
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

    // End of debug

    // For now, if the command parsing fails, just continue.
    if(num_commands <= 0) continue;

    for(int i = 0; i < num_commands; i++){
      Command cmd = commands[i];

      if(strcmp(cmd.argv[0], "exit") == 0){
        return 0;
      }
      else if(strcmp(cmd.argv[0], "prompt") == 0){
        if(cmd.argv[1] != NULL){
          strncpy(prompt, cmd.argv[1], strlen(cmd.argv[1]) + 1);
        }
        else{
          strncpy(prompt, "%", 2);
        }
      }
    }
  }

  return 0;
}