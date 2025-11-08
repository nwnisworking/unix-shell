#include "history.h"

static char *hist[HIST_MAX];
static int hcount = 0;
static int cursor = 0;

static char *dupTrimNl(const char *s){
    if(!s) return NULL;
    size_t L = strlen(s);
    if(L && s[L-1] == '\n') return strndup(s, L-1);
    
    return strdup(s);
}

void historyAdd(const char *line){
    if(!line || !*line) return;
    int idx = hcount % HIST_MAX;
    free(hist[idx]);
    hist[idx] = dupTrimNl(line);   // store WITHOUT trailing '\n'
    hcount++;
    cursor = hcount;                 // reset cursor to end
}

void historyPrint(int last_n){
    int start = (hcount > last_n ? hcount - last_n : 0);
    for(int i = start; i < hcount; i++){
        // entries are already newline-free; we add exactly one newline here
        fprintf(stdout, "%d  %s\n", i + 1, hist[i % HIST_MAX]);
    }
}

static const char *findPrefix(const char *pre){
    size_t L = strlen(pre);
    for(int i = hcount - 1; i >= 0; i--){
        const char *s = hist[i % HIST_MAX];
        if(s && strncmp(s, pre, L) == 0) return s; // match prefix
    }
    return NULL;
}

char *historyExpandBang(const char *line){
    if(!line || line[0] != '!') return NULL;

    if(strcmp(line, "!!") == 0){
        if(hcount == 0) return NULL;
        return strdup(hist[(hcount - 1) % HIST_MAX]); // already trimmed
    }
    if(isdigit((unsigned char)line[1])){
        int n = atoi(line + 1);
        if(n <= 0 || n > hcount) return NULL;
        return strdup(hist[(n - 1) % HIST_MAX]);      // already trimmed
    }
    const char *s = findPrefix(line + 1);
    return s ? strdup(s) : NULL;                      // already trimmed
}

const char *historyPrev(void){
    if(hcount == 0) return NULL;
    if(cursor <= 0) cursor = 0; else cursor--;
    return hist[cursor % HIST_MAX]; // newline-free
}

const char *historyNext(void){
    if(hcount == 0) return NULL;
    if(cursor < hcount) cursor++;
    if(cursor == hcount) return ""; // at end -> empty recall
    return hist[cursor % HIST_MAX]; // newline-free
}
