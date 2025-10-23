#ifndef BUILTINS_H
#define BUILTINS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Sets the shell prompt to a new value.
 * 
 * @param prompt The current prompt string to be updated.
 * @param new_prompt The new prompt string to set. If NULL or empty, sets to
 */
void builtinPrompt(char prompt[], const char* new_prompt);

/**
 * Prints the current working directory to standard output.
 */
void builtinPwd();

/**
 * Changes the current working directory to the specified path.
 * 
 * @param path The target directory path. If NULL, does nothing.
 */
void builtinCD(const char* path);

#endif