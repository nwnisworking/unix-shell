#include "utils.h"
#include "history.h"
#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>   // isprint

size_t getInput(char* buffer){
  size_t len = strlen(buffer);
  if(len > 0 && buffer[len - 1] == '\n'){
    buffer[len - 1] = '\0';
  }
  return len;
}

static struct termios orig;

int enableRawMode(void){
  if(!isatty(STDIN_FILENO)) return 0;
  if(tcgetattr(STDIN_FILENO, &orig) == -1) return -1;
  struct termios raw = orig;
  raw.c_lflag &= ~(ICANON | ECHO);
  raw.c_cc[VMIN]=1; raw.c_cc[VTIME]=0;
  return tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}
void disableRawMode(void){
  if(isatty(STDIN_FILENO)) tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig);
}

static void redraw(const char *prompt, const char *buf, size_t cur){
  printf("\r%s %s\033[K", prompt, buf);
  printf("\r\033[%zuC", strlen(prompt)+1+cur);
  fflush(stdout);
}

int readLine(char *out, size_t cap, const char *prompt){
  if (!isatty(STDIN_FILENO)) {
    fputs(prompt, stdout); fflush(stdout);
    return fgets(out, cap, stdin)!=NULL;
  }

  if (enableRawMode()<0) return 0;
  size_t len=0; size_t cur=0;
  out[0]='\0';

  fputs(prompt, stdout); fputc(' ', stdout); fflush(stdout);

  for(;;){
    unsigned char c;
    if(read(STDIN_FILENO, &c, 1)!=1){ disableRawMode(); return 0; }

    if(c=='\r' || c=='\n'){
      putchar('\n'); out[len]='\0';
      disableRawMode(); return 1;
    }
    else if(c==127 || c==8){ // backspace
      if(cur>0){
        memmove(out+cur-1, out+cur, len-cur);
        cur--; len--;
        redraw(prompt, out, cur);
      }
    }
    else if(c==27){ // escape seq
      unsigned char seq[2];
      if(read(STDIN_FILENO, seq, 2)!=2) continue;
      if(seq[0]=='['){
        if(seq[1]=='D'){ // left
          if(cur>0){ cur--; printf("\033[1D"); fflush(stdout); }
        } else if(seq[1]=='C'){ // right
          if(cur<len){ cur++; printf("\033[1C"); fflush(stdout); }
        } else if(seq[1]=='A'){ // up
          const char *h = history_prev();
          if(h){
            len = cur = strnlen(h, cap-1);
            strncpy(out, h, cap-1); out[len]='\0';
            redraw(prompt, out, cur);
          }
        } else if(seq[1]=='B'){ // down
          const char *h = history_next();
          if(h){
            len = cur = strnlen(h, cap-1);
            strncpy(out, h, cap-1); out[len]='\0';
            redraw(prompt, out, cur);
          }
        }
      }
    }
    else if(isprint(c)){
      if(len+1 < cap){
        memmove(out+cur+1, out+cur, len-cur);
        out[cur]=c; len++; cur++;
        redraw(prompt, out, cur);
      }
    }
  }
}
