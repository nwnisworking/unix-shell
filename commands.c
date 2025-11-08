#include "commands.h"

static void unescapeInPlace(char *s){
  if(!s) return;
  size_t r = 0, w = 0;
    
  while(s[r]){
    if(s[r] == '\\' && s[r + 1]){
      r++; // skip backslash
      s[w++] = s[r++]; // copy next char
    }
    else{
      s[w++] = s[r++]; // copy char
    }
  }

  s[w] = '\0';
}

static void stripSurroundingQuotes(char *s){
  if(!s) return;
  size_t n = strlen(s);

  if(n >= 2){
    char a = s[0], b = s[n-1];

    if((a == '\'' && b == '\'') || (a == '"' && b == '"')){
      // remove the first and last quote, in place
      memmove(s, s+1, n-2);
      s[n-2] = '\0';
    }
  }
}

static void normaliseToken(char *s){
  // Order matters: strip outer quotes first, then unescape inside
  stripSurroundingQuotes(s);
  unescapeInPlace(s);
}

static int isRedirToken(const char *t){
    if(!t) return 0;
    if((strlen(t) == 1) && (t[0] == REDIRECT_IN || t[0] == REDIRECT_OUT)) return 1;
    if(strcmp(t, REDIRECT_ERR2) == 0) return 1;
    return 0;
}

// -----------------------
// Public API
// -----------------------
int separateCommands(char* tokens[], Command commands[]){
  int i = 0, tokens_size = 0;
  while(tokens[i] != NULL) i++;
  tokens_size = i;

  if(tokens_size == 0) return 0;
  if(isSeparator(tokens[0])) return -3;

  // Add trailing separator if missing
  if(!isSeparator(tokens[tokens_size - 1])){
    tokens[tokens_size] = strdup((char[]){ SEQUENCE_SEP, '\0' });
    tokens_size++;
  }

  int first = 0, last = 0, c = 0;

  for(i = 0; i < tokens_size; i++){
    last = i;

    if(isSeparator(tokens[i])){
      if(first == last) return -2; // empty command

      // Populate command metadata
      commands[c].first = first;
      commands[c].last  = last - 1;
      commands[c].sep   = tokens[i];
      commands[c].argv  = NULL;
      commands[c].stdin_file  = NULL;
      commands[c].stdout_file = NULL;
      commands[c].stderr_file = NULL;
      c++;

      first = i + 1;
    }
  }

  if(tokens[last][0] == PIPE_SEP) return -4; // ends with pipe

  // Finalise each command
  for(i = 0; i < c; i++){
    searchRedirections(tokens, &commands[i]);
    buildArgvArray(tokens, &commands[i]);
  }

  return c;
}

int isSeparator(char* token){
  if(token == NULL) return 0;
  if(strlen(token) == 1){
    char separators[] = { PIPE_SEP, CONCURRENT_SEP, SEQUENCE_SEP, '\0' };
      
    for(int i = 0; separators[i] != '\0'; i++){
      if(separators[i] == token[0]) return 1;
    }
  }

  return 0;
}

void searchRedirections(char* tokens[], Command* command){
  for(int i = command->first; i <= command->last; i++){
    char* token = tokens[i];
    if(!token) continue;

    // Handle input redirection
    if(token[0] == REDIRECT_IN && token[1] == '\0'){
      if(i + 1 <= command->last && tokens[i + 1] != NULL){
        normaliseToken(tokens[i + 1]);          // NEW: normalise filename
        command->stdin_file = tokens[i + 1];
        i++;
      }
    }
    // Handle output redirection
    else if(token[0] == REDIRECT_OUT && token[1] == '\0'){
      if(i + 1 <= command->last && tokens[i + 1] != NULL){
        normaliseToken(tokens[i + 1]);          // NEW: normalise filename
        command->stdout_file = tokens[i + 1];
        i++;
      }
    }
    // Handle stderr redirection (2>)
    else if(strcmp(token, REDIRECT_ERR2) == 0){
      if(i + 1 <= command->last && tokens[i + 1] != NULL){
        normaliseToken(tokens[i + 1]);          // NEW: normalise filename
        command->stderr_file = tokens[i + 1];
        i++;
      }
    }
  }
}

void buildArgvArray(char* tokens[], Command* command){
  int pairs =
    (command->stdin_file  ? 1 : 0) +
    (command->stdout_file ? 1 : 0) +
    (command->stderr_file ? 1 : 0);

  int span = (command->last - command->first + 1);
  int n = span - (pairs * 2) + 1; // +1 for NULL

  if(n < 1) n = 1;

  // Allocate argv array
  command->argv = (char**) realloc(command->argv, n * sizeof(char*));
  if(command->argv == NULL){
    fprintf(stderr, "Memory allocation failed\n");
    exit(1);
  }

  int k = 0;
  for(int i = command->first; i <= command->last; i++){
    char* token = tokens[i];
    if(!token) continue;

    // skip redirection operator + filename
    if(isRedirToken(token)){
        i++; // skip its target filename
        continue;
    }

    command->argv[k] = tokens[i];

    // NEW: normalise each argv token (handles echo "\" and friends)
    normaliseToken(command->argv[k]);

    k++;
  }

  command->argv[k] = NULL;
}

void clearCommands(Command commands[]){
  for(int i = 0; i < MAX_NUM_COMMANDS; i++){
    commands[i].first = 0;
    commands[i].last = 0;
    commands[i].sep = NULL;
    commands[i].stdin_file = NULL;
    commands[i].stdout_file = NULL;
    commands[i].stderr_file = NULL;

    // Free argv if allocated
    if(commands[i].argv != NULL){
      free(commands[i].argv);
      commands[i].argv = NULL;
    }
  }
}

int applyRedirs(const Command *c){
  int fd = -1;

  if(c->stdin_file){
    fd = open(c->stdin_file, O_RDONLY);

    if(fd < 0){ perror(c->stdin_file); return -1; }
    if(dup2(fd, STDIN_FILENO) < 0){ perror("dup2 in"); close(fd); return -1; }
  }

  if(c->stdout_file){
    fd = open(c->stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    if(fd < 0){ perror(c->stdout_file); return -1; }
    if(dup2(fd, STDOUT_FILENO) < 0){ perror("dup2 out"); close(fd); return -1; }
  }

  if(c->stderr_file){
    fd = open(c->stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    
    if(fd < 0){ perror(c->stderr_file); return -1; }
    if(dup2(fd, STDERR_FILENO) < 0){ perror("dup2 err"); close(fd); return -1; }
  }

  // Make sure to close the file descriptor if opened
  if(fd >= 0)
    close(fd);

  return 0;
}

