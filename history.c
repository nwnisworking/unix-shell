#include "history.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static char *hist[HIST_MAX];
static int hcount=0;
static int cursor=0; // for arrow browsing

void history_add(const char *line){
    if(!line || !*line) return;
    int idx = hcount % HIST_MAX;
    free(hist[idx]); hist[idx]=strdup(line);
    hcount++;
    cursor = hcount; // reset browsing to "after last"
}

void history_print(int last_n){
    int start = (hcount>last_n? hcount-last_n : 0);
    for(int i=start;i<hcount;i++){
        fprintf(stdout, "%d  %s", i+1, hist[i%HIST_MAX]); // assume line has '\n'
    }
}

static const char *find_prefix(const char *pre){
    size_t L=strlen(pre);
    for(int i=hcount-1;i>=0;i--){
        const char *s=hist[i%HIST_MAX];
        if(s && strncmp(s, pre, L)==0) return s;
    }
    return NULL;
}

char *history_expand_bang(const char *line){
    // Only expand if line starts with '!'
    if(!line || line[0] != '!') return NULL;

    // "!!"
    if(strcmp(line,"!!")==0){
        if(hcount==0) return NULL;
        return strdup(hist[(hcount-1)%HIST_MAX]);
    }

    // "!n"
    if(isdigit((unsigned char)line[1])){
        int n = atoi(line+1);
        if(n<=0 || n>hcount) return NULL;
        return strdup(hist[(n-1)%HIST_MAX]);
    }

    // "!prefix"
    const char *s = find_prefix(line+1);
    return s ? strdup(s) : NULL;
}

// Arrow browsing
const char *history_prev(void){
    if(hcount==0) return NULL;
    if(cursor<=0) cursor=0; else cursor--;
    return hist[cursor%HIST_MAX];
}
const char *history_next(void){
    if(hcount==0) return NULL;
    if(cursor < hcount) cursor++; // move towards "after last"
    if(cursor==hcount) return ""; // empty current
    return hist[cursor%HIST_MAX];
}
