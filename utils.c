#include "utils.h"

size_t getInput(char* buffer){
  size_t len = strlen(buffer);

  if(len > 0 && buffer[len - 1] == '\n'){
    buffer[len - 1] = '\0';
  }

  return len;
}