#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

#include <dirent.h>     // wildcard expansion
#include <fnmatch.h>
#include <limits.h>

#include "utils.h"
#include "tokens.h"
#include "commands.h"
#include "builtins.h"
#include "history.h"

#define MAX_LINE_LENGTH 16384

// ----------------------
// SIGCHLD: reap zombies
// ----------------------
static void sigchld_handler(int sig){
  (void)sig;
  int saved = errno;
  while (1){
    int status;
    pid_t p = waitpid(-1, &status, WNOHANG);
    if (p <= 0) break;
  }
  errno = saved;
}

static void install_signal_handlers(void){
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = sigchld_handler;
  sa.sa_flags   = SA_RESTART | SA_NOCLDSTOP;
  sigemptyset(&sa.sa_mask);
  sigaction(SIGCHLD, &sa, NULL);

  // Don’t let Ctrl-C, Ctrl-\ or Ctrl-Z kill the shell
  signal(SIGINT,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
}

// ----------------------
// Redirections
// ----------------------
static int apply_redirs(const Command *c){
  if (c->stdin_file){
    int fd = open(c->stdin_file, O_RDONLY);
    if(fd < 0){ perror(c->stdin_file); return -1; }
    if(dup2(fd, STDIN_FILENO) < 0){ perror("dup2 in"); close(fd); return -1; }
    close(fd);
  }
  if (c->stdout_file){
    int fd = open(c->stdout_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd < 0){ perror(c->stdout_file); return -1; }
    if(dup2(fd, STDOUT_FILENO) < 0){ perror("dup2 out"); close(fd); return -1; }
    close(fd);
  }
  if (c->stderr_file){
    int fd = open(c->stderr_file, O_WRONLY|O_CREAT|O_TRUNC, 0666);
    if(fd < 0){ perror(c->stderr_file); return -1; }
    if(dup2(fd, STDERR_FILENO) < 0){ perror("dup2 err"); close(fd); return -1; }
    close(fd);
  }
  return 0;
}

// ----------------------
// Wildcards (fnmatch)
// ----------------------
static int has_wild(const char *s){
  return s && strpbrk(s, "*?[") != NULL;
}

static char *join_path(const char *dir, const char *name){
  if (!dir || strcmp(dir, ".")==0) return strdup(name);
  size_t ld=strlen(dir), ln=strlen(name);
  char *p = malloc(ld + 1 + ln + 1);
  if(!p) return NULL;
  memcpy(p, dir, ld);
  p[ld] = '/';
  memcpy(p+ld+1, name, ln+1);
  return p;
}

static int expand_one_pattern(const char *pattern, char ***outv, int *outc){
  const char *slash = strrchr(pattern, '/');
  char dirbuf[PATH_MAX];
  const char *dir = ".";
  const char *base = pattern;

  if (slash){
    size_t n = (size_t)(slash - pattern);
    if (n >= sizeof(dirbuf)) n = sizeof(dirbuf)-1;
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
    if(!want_dots && name[0]=='.') continue;
    if(fnmatch(base, name, 0) == 0){
      char *full = join_path(dir, name);
      if(!full){ closedir(d); return added; }
      (*outv)[(*outc)++] = full;
      added++;
    }
  }
  closedir(d);
  return added;
}

static int expand_wildcards(char ***argvp){
  char **argv = *argvp;
  int argc = 0; while(argv[argc]) argc++;

  // generous headroom to append matches
  char **nargv = malloc(sizeof(char*) * (argc + 4096));
  if(!nargv) return -1;

  int nargc = 0;
  for(int i=0;i<argc;i++){
    const char *tok = argv[i];
    if(!tok || tok[0]=='-' || !has_wild(tok)){
      nargv[nargc++] = argv[i];
      continue;
    }
    int added = expand_one_pattern(tok, &nargv, &nargc);
    if(added == 0){
      nargv[nargc++] = argv[i]; // no matches -> keep literal
    }
  }
  nargv[nargc] = NULL;
  *argvp = nargv;
  return 0;
}

// ----------------------
// Pipelines
// ----------------------
static int run_pipeline(Command *cmds, int start, int end, int background){
  int prev=-1;
  for(int i=start;i<=end;i++){
    int pipefd[2]={-1,-1};
    if(i<end && pipe(pipefd)<0){ perror("pipe"); return -1; }

    pid_t pid = fork();
    if(pid<0){ perror("fork"); return -1; }

    if(pid==0){
      // child: restore default signals
      signal(SIGINT,  SIG_DFL);
      signal(SIGQUIT, SIG_DFL);
      signal(SIGTSTP, SIG_DFL);

      if(prev!=-1){ dup2(prev, STDIN_FILENO); }
      if(i<end){ dup2(pipefd[1], STDOUT_FILENO); }
      if(prev!=-1) close(prev);
      if(i<end){ close(pipefd[0]); close(pipefd[1]); }

      if(apply_redirs(&cmds[i])<0) _exit(1);

      // wildcard expansion (argv is owned by parent tokens; safe to reshape)
      char **argv = cmds[i].argv;
      if(expand_wildcards(&argv) < 0) _exit(1);

      execvp(argv[0], argv);
      perror(argv[0]);
      _exit(127);
    }

    if(prev!=-1) close(prev);
    if(i<end){ close(pipefd[1]); prev = pipefd[0]; }
  }

  if(!background){
    // wait for all in this pipeline
    for(int i=start;i<=end;i++){
      int status;
      (void)wait(&status);
    }
  }
  return 0;
}

// ----------------------
// Built-ins in parent
// ----------------------
static int run_builtin_in_parent(char prompt[], Command *c){
  if(!c->argv || !c->argv[0]) return 0;

  if(strcmp(c->argv[0],"exit")==0){ exit(0); }
  if(strcmp(c->argv[0],"prompt")==0){ builtinPrompt(prompt, c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"pwd")==0){    builtinPwd(); return 1; }
  if(strcmp(c->argv[0],"cd")==0){     builtinCD(c->argv[1]); return 1; }
  if(strcmp(c->argv[0],"history")==0){ historyPrint(1000); return 1; }

  return 0;
}

// ----------------------
// Main loop
// ----------------------
int main(void){
  char line[MAX_LINE_LENGTH];
  char *tokens_arr[MAX_TOKENS]={0};
  Command commands[MAX_NUM_COMMANDS]={0};
  char prompt[64] = "%";

  install_signal_handlers();

  while(1){
    clearTokens(tokens_arr);
    clearCommands(commands);

    // prompt
    printf("%s ", prompt);
    fflush(stdout);

    // robust fgets that tolerates EINTR
    while(1){
      if(fgets(line, sizeof(line), stdin)) break;
      if(errno == EINTR) continue;  // try again if a signal interrupted
      // EOF (Ctrl-D) or fatal error
      putchar('\n');
      return 0;
    }

    // trim newline (inline; no utils.c dependency)
{
  size_t L = strlen(line);
  if (L > 0 && line[L-1] == '\n') line[L-1] = '\0';
}


    // history: expand bang, then store
    char *expanded = historyExpandBang(line);
    const char *use_line = expanded ? expanded : line;

    // store to history (store exactly once, no trailing blank line)
    historyAdd(use_line);

    // tokenise & split
    if (tokenise((char*)use_line, tokens_arr) < 0){ free(expanded); continue; }

    int ncmds = separateCommands(tokens_arr, commands);
    if(ncmds <= 0){ free(expanded); continue; }

    // execute commands: group pipelines, run built-ins in parent when not piped
    int i=0;
    while(i<ncmds){
      // attempt builtin if NOT part of a pipeline
      if( (i==ncmds-1 || (commands[i].sep && commands[i].sep[0] != PIPE_SEP)) &&
          run_builtin_in_parent(prompt, &commands[i]) ){
        i++;
        continue;
      }

      // find end of pipeline starting at i
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
