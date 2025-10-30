#ifndef UTILS_H
#define UTILS_H

#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include "ansi.h"

/**
 * @brief Enable or disable raw mode for the terminal.
 * @param enabled If non-zero, enable raw mode; otherwise, disable it.
 */
void termiosMode(int enabled);

/**
 * @brief Read a line of input from the terminal.
 * @param line Pointer to the buffer where the input line will be stored.
 * @return The direction of arrow keys if pressed, otherwise 0.
 */
int readLine(char* line);

/**
 * @brief Clear the contents of the line buffer.
 * @param line Pointer to the line buffer to be cleared.
 */
void clearLine(char* line);

/**
 * @brief Install signal handlers for the shell.
 */
void installSignalHandlers();

#endif // UTILS_H
