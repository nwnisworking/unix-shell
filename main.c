#include <stdio.h>
#include <unistd.h>
#include <glob.h>
#include "utils.h"
#include "tokens.h"
#include "commands.h"
#include "builtins.h"

#define MAX_LINE_LENGTH 1024

int main(){
  char line[MAX_LINE_LENGTH]; // Buffer to hold input
  char* tokens[MAX_TOKENS] = {0}; // Array to hold tokenised input
  Command commands[MAX_NUM_COMMANDS] = {0}; // Array to hold separated commands
  size_t len; // Length of the current input line

  char prompt[MAX_LINE_LENGTH] = "%"; // Default shell prompt

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
      // pid_t pid;

      // if(cmd.sep == CONCURRENT_SEP){
      //   pid = fork();
      // }

      if(strcmp(cmd.argv[0], "exit") == 0){
        return 0;
      }
      else if(strcmp(cmd.argv[0], "prompt") == 0){
        builtinPrompt(prompt, cmd.argv[1]);
      }
      else if(strcmp(cmd.argv[0], "pwd") == 0){
        builtinPwd();
      }
      else if(strcmp(cmd.argv[0], "cd") == 0){
        builtinCD(cmd.argv[1]);
      }
      else if(strcmp(cmd.argv[0], "ls") == 0){
        // glob_t results;
        // char *pattern = cmd.argv[1] != NULL ? cmd.argv[1] : "*";

        // if(glob(pattern, 0, NULL, &results) == 0){
        //   int count = results.gl_pathc;
        //   char** files = malloc((count + 2) * sizeof(char*));

        //   files[0] = strdup("ls");
        //   files[count + 1] = NULL;

        //   for(int j = 0; j < count; j++){
        //     files[j + 1] = strdup(results.gl_pathv[j]);
        //   }

        //   globfree(&results);

        //   pid_t pid = fork();

        //   if(pid < 0){
        //     printf("ls: fork failed\n");
        //     continue;
        //   }
        //   else if(pid > 0){
        //     // Parent process
        //     int status;
        //     waitpid(pid, &status, 0);
            
        //     for(int j = 0; j < count + 1; j++){
        //       free(files[j]);
        //     }
            
        //     free(files);
        //     continue;
        //   }
        //   else{
        //     if(execvp("ls", files) == -1){
        //       _exit(1);
        //     }
        //   }

        // }
      }
    }
  }

  return 0;
}