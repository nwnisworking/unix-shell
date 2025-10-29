#ifndef HISTORY_H
#define HISTORY_H

#define HIST_MAX 1000

void history_add(const char *line);
void history_print(int last_n);
char *history_expand_bang(const char *line); // malloc’d, caller frees or NULL

const char *history_prev(void);
const char *history_next(void);

#endif
