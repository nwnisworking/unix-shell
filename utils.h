#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stddef.h>

// Old helper (kept)
size_t getInput(char* buffer);

// Raw mode line editor: supports ← → cursor, ↑/↓ history, Backspace
int enableRawMode(void);
void disableRawMode(void);
int readLine(char *out, size_t cap, const char *prompt);

#endif // UTILS_H
