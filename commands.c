#include "commands.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>


#define REDIRECT_ERR2 "2>"

// -----------------------
// Quote/escape normalisers
// -----------------------
static void unescape_in_place(char *s){
    if(!s) return;
    size_t r = 0, w = 0;
    while(s[r]){
        if(s[r] == '\\'){
            if(s[r+1] == '\\'){          // double backslash -> one
                s[w++] = '\\'; r += 2;
            } else if(s[r+1] != '\0'){   // \X -> X (remove only the backslash)
                s[w++] = s[r+1]; r += 2;
            } else {                     // lone backslash at end
                s[w++] = '\\'; r++;
            }
        } else {
            s[w++] = s[r++];
        }
    }
    s[w] = '\0';
}


static void strip_surrounding_quotes(char *s){
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

static void normalise_token(char *s){
    // Order matters: strip outer quotes first, then unescape inside
    strip_surrounding_quotes(s);
    unescape_in_place(s);
}

// -----------------------
// Helpers
// -----------------------
static int is_redir_token(const char *t){
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
                normalise_token(tokens[i + 1]);          // NEW: normalise filename
                command->stdin_file = tokens[i + 1];
                i++;
            }
        }
        // Handle output redirection
        else if(token[0] == REDIRECT_OUT && token[1] == '\0'){
            if(i + 1 <= command->last && tokens[i + 1] != NULL){
                normalise_token(tokens[i + 1]);          // NEW: normalise filename
                command->stdout_file = tokens[i + 1];
                i++;
            }
        }
        // Handle stderr redirection (2>)
        else if(strcmp(token, REDIRECT_ERR2) == 0){
            if(i + 1 <= command->last && tokens[i + 1] != NULL){
                normalise_token(tokens[i + 1]);          // NEW: normalise filename
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
        if(is_redir_token(token)){
            i++; // skip its target filename
            continue;
        }

        command->argv[k] = tokens[i];

        // NEW: normalise each argv token (handles echo "\" and friends)
        normalise_token(command->argv[k]);

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

// Redirect
int apply_redirs(const Command *c){
  if (c->stdin_file){
    int fd = open(c->stdin_file, O_RDONLY);
    if(fd<0){ perror(c->stdin_file); return -1; }
    if(dup2(fd, STDIN_FILENO)<0){ perror("dup2 in"); close(fd); return -1; }
    close(fd);
  }
  if (c->stdout_file){
    int fd = open(c->stdout_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd<0){ perror(c->stdout_file); return -1; }
    if(dup2(fd, STDOUT_FILENO)<0){ perror("dup2 out"); close(fd); return -1; }
    close(fd);
  }
  if (c->stderr_file){
    int fd = open(c->stderr_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd<0){ perror(c->stderr_file); return -1; }
    if(dup2(fd, STDERR_FILENO)<0){ perror("dup2 err"); close(fd); return -1; }
    close(fd);
  }
  return 0;
}

