#include "tokens.h"

int tokenise(char* input, char* tokens[]){
  int count = 0;
  char* ptr = input;
  int is_token = 0;

  while(*ptr != '\0'){
    switch(*ptr){
      case '\t' : 
      case '\n' :
      case ' ' : 
        *ptr = '\0';
        is_token = 0;
      break;
      default : 
        if(!is_token){
          if(count >= MAX_TOKENS) return -1;
          tokens[count++] = ptr;
          is_token = 1;
        }
      break;
    }

    ptr++;
  }

  return count;
}