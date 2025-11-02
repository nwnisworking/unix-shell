#ifndef UTILS_H
#define UTILS_H

/**
 * @file utils.h
 * @brief Terminal and signal utilities for shell interaction.
 *
 * Provides functions for enabling raw terminal mode, reading input with
 * cursor control, clearing buffers, and installing signal handlers.
 */

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
 *
 * When enabled, disables canonical mode and echoing to allow character-by-character input.
 * Restores original settings when disabled.
 *
 * @param enabled If non-zero, enable raw mode; otherwise, restore normal mode.
 */
void termiosMode(int enabled);

/**
 * @brief Read a line of input from the terminal with cursor support.
 *
 * Handles printable characters, backspace/delete, arrow keys, and escape sequences.
 * Supports in-line editing and returns direction for up/down arrow keys.
 *
 * @param line Pointer to the buffer where the input line will be stored.
 * @return -1 for up arrow, 1 for down arrow, 0 for normal input.
 */
int readLine(char* line);

/**
 * @brief Clear the contents of the line buffer.
 *
 * Sets all characters in the buffer to null terminators.
 *
 * @param line Pointer to the line buffer to be cleared.
 */
void clearLine(char* line);

/**
 * @brief Install signal handlers for the shell environment.
 *
 * Sets up handlers for child termination and ignores job control signals
 * to prevent shell interruption. Child processes restore default behavior.
 */
void installSignalHandlers();

#endif // UTILS_H