#include "tokens.h"

int tokenise(char* input, char* tokens[]){
  int count = 0;
  char* ptr = input;
  int is_token = 0;

  while(*ptr != '\0'){
    switch(*ptr){
      case TAB_SEP : 
      case NEWLINE_SEP :
      case SPACE_SEP : 
        *ptr = '\0';
        is_token = 0;
      break;
      case PIPE_SEP :
      case CONCURRENT_SEP :
      case SEQUENCE_SEP :
      case REDIRECT_IN :
      case REDIRECT_OUT :
        char sep = *ptr;

        if(is_token){
          *ptr = '\0';
          is_token = 0;
        }

        tokens[count++] = strdup((char[]){sep, '\0'});
        
        if(count >= MAX_TOKENS) return -1;
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

void clearTokens(char* tokens[]){
  for(int i = 0; i < MAX_TOKENS; i++){
    char* token = tokens[i];

    if(token == NULL) break;

    if(strlen(token) == 1 && (
        token[0] == PIPE_SEP || 
        token[0] == CONCURRENT_SEP ||
        token[0] == SEQUENCE_SEP
      )
    ){
      // Free strdup'ed separator tokens
      free(token);
      tokens[i] = NULL;
    }
    else{
      tokens[i] = NULL;
    }
  }
}