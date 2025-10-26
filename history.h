#ifndef HISTORY_H
#define HISTORY_H
#include <stdio.h>

#define HIST_MAX 1000

void history_add(const char *line);
void history_print(int last_n);
char *history_expand_bang(const char *line); // returns malloc’d new line or NULL

// For the simple line editor:
const char *history_prev(void); // step back in history (for Up)
const char *history_next(void); // step forward (for Down)
#endif
