#ifndef UTILS_H
#define UTILS_H

#include <string.h>

/**
 * Get input from the user and remove the trailing newline character.
 * 
 * @param buffer The input buffer containing the user input.
 * @return The length of the input string
 */
size_t getInput(char* buffer);

#endif // UTILS_H