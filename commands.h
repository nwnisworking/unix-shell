#ifndef COMMANDS_H
#define COMMANDS_H

/**
 * @file commands.h
 * @brief Command parsing and redirection utilities.
 *
 * Provides functionality for splitting tokenized input into structured commands,
 * identifying separators, handling I/O redirections, and building argv arrays.
 * Each command is represented by a Command struct, which stores metadata and
 * redirection targets for execution.
 */

#include <string.h>
#include <stdio.h>
#include "tokens.h"

/// Maximum number of commands supported in a single input line
#define MAX_NUM_COMMANDS 1000

/**
 * @struct Command
 * @brief Represents a parsed command with metadata and redirection info.
 *
 * Stores the token range, separator, argument vector, and optional redirection
 * targets for stdin, stdout, and stderr.
 */
typedef struct Command{
    int   first;         ///< Index of first token in the command
    int   last;          ///< Index of last token in the command
    char* sep;           ///< Separator token (e.g., ; | &)
    char** argv;         ///< Argument vector for exec
    char* stdin_file;    ///< Input redirection file
    char* stdout_file;   ///< Output redirection file
    char* stderr_file;   ///< Error redirection file ("2>")
} Command;

/**
 * @brief Splits token array into individual Command structs.
 *
 * Parses the token array and populates the commands array with metadata
 * and redirection info. Handles edge cases like empty commands and trailing pipes.
 *
 * @param tokens Array of token strings.
 * @param commands Array of Command structs to populate.
 * @return Number of commands parsed, or negative error code.
 */
int separateCommands(char* tokens[], Command commands[]);

/**
 * @brief Checks if a token is a command separator.
 *
 * Valid separators include '|', '&', and ';'.
 *
 * @param token Token string to check.
 * @return 1 if separator, 0 otherwise.
 */
int isSeparator(char* token);

/**
 * @brief Extracts redirection targets from a command's token range.
 *
 * Scans the token range for '<', '>', and '2>' and sets the corresponding
 * file pointers in the Command struct.
 *
 * @param tokens Array of token strings.
 * @param command Pointer to the Command struct to update.
 */
void searchRedirections(char* tokens[], Command* command);

/**
 * @brief Builds the argv array for a command.
 *
 * Allocates and populates the argv array, excluding redirection tokens
 * and their associated filenames.
 *
 * @param tokens Array of token strings.
 * @param command Pointer to the Command struct to update.
 */
void buildArgvArray(char* tokens[], Command* command);

/**
 * @brief Frees memory and resets all fields in the commands array.
 *
 * Clears all dynamically allocated argv arrays and resets metadata.
 *
 * @param commands Array of Command structs to clear.
 */
void clearCommands(Command commands[]);

#endif // COMMANDS_H