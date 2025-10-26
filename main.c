#include <stdio.h>
#include <unistd.h>
#include <glob.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <string.h>

#include "utils.h"
#include "tokens.h"
#include "commands.h"
#include "builtins.h"
#include "history.h"

#define MAX_LINE_LENGTH 16384

// --- SIGCHLD: reap zombies
static void sigchld_handler(int sig){
  (void)sig;
  for(;;){
    int st; pid_t p = waitpid(-1, &st, WNOHANG);
    if (p > 0) continue;
    if (p == 0) break;
    if (p < 0 && errno == EINTR) continue;
    break;
  }
}
static void install_signals(void){
  struct sigaction sa = {0};
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);
  // shell ignores these; children will restore defaults
  signal(SIGINT,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
}

// --- redirections
static int apply_redirs(const Command *c){
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

// --- glob expansion (flags untouched)
static int has_glob(const char *s){
  return strpbrk(s, "*?[") != NULL;
}
static int expand_glob(char ***argvp, int *argcp){
  char **argv=*argvp; int argc=*argcp;
  char **nargv = malloc(sizeof(char*)*(argc+4096));
  if(!nargv) return -1; int nargc=0;
  int gflags = GLOB_TILDE | GLOB_BRACE | GLOB_NOCHECK;
  for(int i=0;i<argc;i++){
    if(argv[i][0]=='-' || !has_glob(argv[i])){
      nargv[nargc++]=argv[i];
      continue;
    }
    glob_t gr={0};
    int rc = glob(argv[i], gflags, NULL, &gr);
    if(rc==GLOB_NOSPACE || rc==GLOB_ABORTED){ globfree(&gr); free(nargv); return -1; }
    for(size_t k=0;k<gr.gl_pathc;k++){
      char *dup = strdup(gr.gl_pathv[k]);
      if(!dup){ globfree(&gr); free(nargv); return -1; }
      nargv[nargc++]=dup;
    }
    globfree(&gr);
  }
  nargv[nargc]=NULL;
  *argvp=nargv; *argcp=nargc;
  return 0;
}

// --- run a pipeline (commands[i]..commands[j] where sep '|' until non-pipe)
static int run_pipeline(Command *cmds, int start, int end, int background){
  int prev=-1;
  for(int i=start;i<=end;i++){
    int pipefd[2]={-1,-1};
    if(i<end && pipe(pipefd)<0){ perror("pipe"); return -1; }
    pid_t pid = fork();
    if(pid<0){ perror("fork"); return -1; }
    if(pid==0){
      // child: restore default signals
      signal(SIGINT, SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);
      if(prev!=-1){ dup2(prev, STDIN_FILENO); }
      if(i<end){ dup2(pipefd[1], STDOUT_FILENO); }
      if(prev!=-1) close(prev);
      if(i<end){ close(pipefd[0]); close(pipefd[1]); }
      if(apply_redirs(&cmds[i])<0) _exit(1);
      // expand globs
      char **argv = cmds[i].argv; int argc=0; while(argv[argc]) argc++;
      expand_glob(&argv, &argc);
      execvp(argv[0], argv);
      perror(argv[0]); _exit(127);
    }
    if(prev!=-1) close(prev);
    if(i<end){ close(pipefd[1]); prev = pipefd[0]; }
  }
  if(!background){
    // wait for all children in the pipeline
    for(int i=start;i<=end;i++) wait(NULL);
  }
  return 0;
}

// --- detect & run builtins in parent
static int run_builtin_in_parent(char prompt[], Command *c){
  if(!c->argv || !c->argv[0]) return 0;
  if(strcmp(c->argv[0],"exit")==0){ exit(0); }
  if(strcmp(c->argv[0],"prompt")==0){ builtinPrompt(prompt, c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"pwd")==0){    builtinPwd(); return 1; }
  if(strcmp(c->argv[0],"cd")==0){     builtinCD(c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"history")==0){
      history_print(1000); return 1;
  }
  return 0;
}

int main(){
  char line[MAX_LINE_LENGTH];
  char *tokens[MAX_TOKENS]={0};
  Command commands[MAX_NUM_COMMANDS]={0};
  char prompt[64]="%";

  install_signals();

  for(;;){
    // read line (with arrows + history)
    if(!readLine(line, sizeof(line), prompt)) break;

    // support bang expansion
    char *expanded = history_expand_bang(line);
    const char *use_line = expanded ? expanded : line;

    // add to history (store original or expanded; bash stores expanded)
    history_add((char*)use_line[ strlen(use_line)-1 ]=='\n'? use_line : (char*)use_line);

    // tokenise → commands
    clearTokens(tokens); clearCommands(commands);
    if (tokenise((char*)use_line, tokens)<0) continue;

    int ncmds = separateCommands(tokens, commands);
    if(ncmds <= 0) { free(expanded); continue; }

    // walk command list, build pipelines separated by non-'|' seps
    int i=0;
    while(i<ncmds){
      // if builtin and not in a pipeline, run in parent and skip forking
      if( (i==ncmds-1 || commands[i].sep[0] != PIPE_SEP) && run_builtin_in_parent(prompt, &commands[i]) ){
        i++; continue;
      }
      // find end of pipeline [i..j]
      int j=i;
      while(j<ncmds-1 && commands[j].sep && commands[j].sep[0]==PIPE_SEP) j++;
      int background = (commands[j].sep && commands[j].sep[0]==CONCURRENT_SEP);
      run_pipeline(commands, i, j, background);
      i = j+1;
    }

    free(expanded);
  }
  return 0;
}
