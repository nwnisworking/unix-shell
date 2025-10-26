#ifndef UTILS_H
#define UTILS_H

#include <string.h>
#include <stddef.h>

/**
 * Get input from the user and remove the trailing newline character.
 * 
 * @param buffer The input buffer containing the user input.
 * @return The length of the input string
 */
size_t getInput(char* buffer);

// Enable/disable raw terminal mode
int enableRawMode(void);
void disableRawMode(void);

// Read an edited line (supports L/R arrows, Up/Down history, backspace)
// Returns 1 if a line was read into out (null-terminated), 0 on EOF.
int readLine(char *out, size_t cap, const char *prompt);

#endif // UTILS_H
