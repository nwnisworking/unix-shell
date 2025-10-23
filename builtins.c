#include "builtins.h"

void builtinPrompt(char prompt[], const char* new_prompt){
  if(new_prompt == NULL || strlen(new_prompt) == 0){
    strncpy(prompt, "%", 2);
  }
  else{
    strncpy(prompt, new_prompt, strlen(new_prompt) + 1);
  }
}

void builtinPwd(){
  char dir[1024];

  if(getcwd(dir, sizeof(dir)) != NULL){
    printf("%s\n", dir);
  }
}

void builtinCD(const char* path){
  if(path != NULL){
    if(chdir(path) != 0){
      printf("cd: no such file or directory\n");
    }
  }
}