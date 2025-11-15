#include <stdio.h>
#include <unistd.h>

#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <dirent.h>    // wildcard expansion
#include <fnmatch.h>
#include <limits.h>

#include "utils.h"
#include "tokens.h"
#include "commands.h"
#include "builtins.h"
#include "history.h"

#define MAX_LINE_LENGTH 16384

static int hasWild(const char *s){
  return s && strpbrk(s, "*?[") != NULL;
}
static char *joinPath(const char *dir, const char *name){
  if(!dir || strcmp(dir, ".") == 0) return strdup(name);
  
  size_t ld = strlen(dir), ln = strlen(name);
  char *p = malloc(ld + 1 + ln + 1);
  
  if(!p) return NULL;
  
  memcpy(p, dir, ld);
  p[ld] = '/';
  memcpy(p + ld + 1, name, ln + 1);
  
  return p;
}
static int expandOnePattern(const char *pattern, char ***outv, int *outc){
  const char *slash = strrchr(pattern, '/');
  char dirbuf[PATH_MAX];
  const char *dir = ".";
  const char *base = pattern;

  if(slash){
    size_t n = (size_t)(slash - pattern);
    
    if(n >= sizeof(dirbuf)) n = sizeof(dirbuf) - 1;
    
    memcpy(dirbuf, pattern, n);
    
    dirbuf[n] = '\0';
    dir = dirbuf;
    base = slash + 1;
  }

  DIR *d = opendir(dir);
  if(!d) return 0;

  int want_dots = (base[0] == '.');
  struct dirent *e;
  int added = 0;

  while((e = readdir(d))){
    const char *name = e->d_name;
    
    if(!want_dots && name[0] == '.') continue;
    if(fnmatch(base, name, 0) == 0){
      char *full = joinPath(dir, name);
      
      if(!full){ closedir(d); return added; }
      
      (*outv)[(*outc)++] = full;
      added++;
    }
  }

  closedir(d);
  return added;
}

static int expandWildcards(char ***argvp){
  char **argv = *argvp;
  int argc = 0; while(argv[argc]) argc++;

  char **nargv = malloc(sizeof(char*) * (argc + 4096));
  if(!nargv) return -1;

  int nargc = 0;

  for(int i = 0;i < argc; i++){
    const char *tok = argv[i];
    
    if(!tok || tok[0]=='-' || !hasWild(tok)){
      nargv[nargc++] = argv[i];
      continue;
    }
    
    int added = expandOnePattern(tok, &nargv, &nargc);
    
    if(added == 0){
      nargv[nargc++] = argv[i]; // no matches -> keep literal
    }
  }

  nargv[nargc] = NULL;
  *argvp = nargv;
  
  return 0;
}


// ---------- Pipelines ----------
static int runPipeline(Command *cmds, int start, int end, int background){
  int prev=-1;
  
  for(int i = start;i <= end; i++){
    int pipefd[2]={-1,-1};
    
    if(i < end && pipe(pipefd) < 0){ perror("pipe"); return -1; }
    
    pid_t pid = fork();
    
    if(pid < 0){ perror("fork"); return -1; }
    if(pid == 0){
      // child: restore default signals
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      if(prev != -1){ dup2(prev, STDIN_FILENO); }
      if(i < end){ dup2(pipefd[1], STDOUT_FILENO); }
      if(prev != -1) close(prev);
      if(i < end){ close(pipefd[0]); close(pipefd[1]); }

      if(applyRedirs(&cmds[i]) < 0) _exit(1);

     // wildcard expansion
    char **argv = cmds[i].argv;

    if(expandWildcards(&argv) < 0) _exit(1);
      execvp(argv[0], argv);
      perror(argv[0]); _exit(127);
    }

    if(prev != -1) close(prev);
    if(i < end){ close(pipefd[1]); prev = pipefd[0]; }
  }
  
  if(!background){
    for(int i = start; i <= end; i++) wait(NULL);
  }

  return 0;
}

// ---------- Built-ins in parent ----------
static int runBuiltinInParent(char prompt[], Command *c){
  if(!c->argv || !c->argv[0]) return 0;
  if(strcmp(c->argv[0],"exit") == 0){ exit(0); }
  if(strcmp(c->argv[0],"prompt") == 0){ builtinPrompt(prompt, c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"pwd") == 0){ builtinPwd(); return 1; }
  if(strcmp(c->argv[0],"cd") == 0 ){ builtinCD(c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"history") == 0 ){ historyPrint(1000); return 1; }
  
  return 0;
}

static void cleanup(){
  termiosMode(0);
  fflush(stdout);
}

int main(int argc, char* argv[]){
  char line[MAX_LINE_LENGTH];
  char *tokens[MAX_TOKENS]={0};
  Command commands[MAX_NUM_COMMANDS]={0};
  char prompt[64]="%";
  int skip_prompt = 0;
  int is_server = 0;

  if(argc > 1 && strcmp(argv[1], "-server") == 0){
    is_server = 1;
  }

  termiosMode(!is_server);
  installSignalHandlers();
  atexit(cleanup);

  while(1){
    // Reset everything for the next commands
    clearTokens(tokens); 
    clearCommands(commands);

    // Skip prompt ignores the prompt display once. 
    // This is made to prevent a double prompt from appearing. 
    // Whenever we navigate the history, we already have a prompt displayed.
    if(skip_prompt){
      skip_prompt = 0;
    }
    else{
      // Clear the line buffer for new input since it does not come from the history.
      clearLine(line);

      if(is_server) printf("[PROMPT]");
      else printf("%s ", prompt);

      fflush(stdout);
    }

    // Allows navigation through history using up and down arrows.
    int hist_nav;

    if(is_server){
      hist_nav = 0;
      if(!fgets(line, sizeof(line), stdin)){
        exit(0);
      }

      line[strcspn(line, "\n")] = 0; // Remove trailing newline
    }
    else{
      hist_nav = readLine(line);
    }

    if(hist_nav == -1){
      const char *prev = historyPrev();
      
      skip_prompt = 1;

      if(prev){
        // Make way for the previous command from history.
        clearLine(line);
        strncpy(line, prev, MAX_LINE_LENGTH - 1);

        // Clear the line before displaying the new command.
        printf(ANSI_CLEAR_LINE_CMD);
        fflush(stdout);

        // Display the previous command from history.
        printf("\r%s %s", prompt, line);
        fflush(stdout);
      }

      continue; 
    }
    else if(hist_nav == 1){
      const char *next = historyNext();
      
      skip_prompt = 1;

      if(next){
        // Make way for the next command from history.
        clearLine(line);
        strncpy(line, next, MAX_LINE_LENGTH-1);

        // Clear the line before displaying the new command.
        printf(ANSI_CLEAR_LINE_CMD);
        fflush(stdout);

        // Display the next command from history.
        printf("\r%s %s", prompt, line);
        fflush(stdout);
      }

      continue;
    }
    
    char *expanded = historyExpandBang(line);
    const char *use_line = expanded ? expanded : line;

    // store to history (ensure newline for nice printing)
    {
      size_t L = strlen(use_line);
      char stored[MAX_LINE_LENGTH+2];
      if(L > MAX_LINE_LENGTH) L = MAX_LINE_LENGTH;
      memcpy(stored, use_line, L);
      if(L == 0 || stored[L-1] != '\n') stored[L++] = '\n';
      stored[L] = '\0';
      historyAdd(stored);
    }

    
    if(tokenise((char*)use_line, tokens) < 0){ free(expanded); continue; }

    int ncmds = separateCommands(tokens, commands);
    if(ncmds <= 0){ free(expanded); continue; }

    int i=0;
    while(i < ncmds){
      if((i==ncmds-1 || commands[i].sep[0] != PIPE_SEP) &&
        runBuiltinInParent(prompt, &commands[i])
      ){
        i++;
        
        continue;
      }
      
      int j=i;
      
      while(j < ncmds-1 && commands[j].sep && commands[j].sep[0] == PIPE_SEP) j++;
      int background = (commands[j].sep && commands[j].sep[0] == CONCURRENT_SEP);

      // Termios might interfere if it tries to run the program. This disables it temporarily until the program ends.
      termiosMode(0);
      runPipeline(commands, i, j, background);
      termiosMode(!is_server);

      i = j + 1;
    }

    free(expanded);
  }

  return 0;
}
