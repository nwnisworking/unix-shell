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

    for(int i = 0; i < len; i++){
      printf("[%d]: %c (%d)\n", i + 1, line[i], (int) line[i]);
    }

    if(strcmp(line, "exit") == 0) break;
  }

  return 0;
}