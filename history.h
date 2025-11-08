#ifndef HISTORY_H
#define HISTORY_H

/**
 * @file history.h
 * @brief Command history management for interactive shell.
 *
 * Provides functions to store, retrieve, and expand command history.
 * Supports bang expansion (!), navigation through previous/next entries,
 * and printing recent history.
 */

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>

#define HIST_MAX 1000  ///< Maximum number of history entries stored

/**
 * @brief Adds a new line to the history buffer.
 *
 * Stores the given command line in the circular history buffer.
 * Overwrites oldest entries if the buffer is full.
 *
 * @param line Command line string to store.
 */
void historyAdd(const char *line);

/**
 * @brief Prints the last N history entries.
 *
 * Displays up to last_n entries from the history buffer.
 *
 * @param last_n Number of recent entries to print.
 */
void historyPrint(int last_n);

/**
 * @brief Expands a bang expression (!).
 *
 * Supports !!, !N, and !prefix formats. Returns a copy of the matching
 * history line or NULL if no match is found.
 *
 * @param line Bang expression to expand.
 * @return malloc’d string with expanded line, or NULL if not found.
 */
char *historyExpandBang(const char *line);

/**
 * @brief Retrieves the previous history entry.
 *
 * Moves the internal cursor backward and returns the corresponding entry.
 *
 * @return Pointer to previous history string, or NULL if none.
 */
const char *historyPrev(void);

/**
 * @brief Retrieves the next history entry.
 *
 * Moves the internal cursor forward and returns the corresponding entry.
 *
 * @return Pointer to next history string, or empty string if at end.
 */
const char *historyNext(void);

#endif // HISTORY_H