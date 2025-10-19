#include <stdio.h>
#include "utils.h"

#define MAX_LINE_LENGTH 1024
#define MAX_NUM_TOKENS 100

int main(){
  char line[MAX_LINE_LENGTH];
  char* tokens[MAX_NUM_TOKENS];
  size_t len;

  while(1){
    printf("$ ");

    if(!fgets(line, MAX_LINE_LENGTH, stdin)) break;

    len = getInput(line);

    if(strcmp(line, "exit") == 0) break;

    int num_tokens = tokenise(line, tokens);

    // Debug token output
    for(int i = 0; i < num_tokens; i++){
      printf("Token %d: %s\n", i + 1, tokens[i]);
    }


  }

  return 0;
}