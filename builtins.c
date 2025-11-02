#include "builtins.h"

void builtinPrompt(char prompt[], const char* new_prompt){
    // Set default prompt if none provided
    if(new_prompt == NULL || strlen(new_prompt) == 0){
        strncpy(prompt, "%", 2);
    }else{
        // Copy custom prompt into buffer
        strncpy(prompt, new_prompt, strlen(new_prompt) + 1);
    }
}

void builtinPwd(){
    char dir[1024];

    // Print current working directory
    if(getcwd(dir, sizeof(dir)) != NULL){
        printf("%s\n", dir);
    }
}

void builtinCD(const char* path){
    // Attempt to change directory
    if(path != NULL){
        if(chdir(path) != 0){
            // Print error if path is invalid
            printf("cd: no such file or directory\n");
        }
    }
}