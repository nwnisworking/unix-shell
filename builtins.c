#include "builtins.h"

/**
 * @brief Sets the shell prompt to a new value or defaults to '%'.
 *
 * If the provided new_prompt is NULL or empty, sets the prompt to '%'.
 * Otherwise, copies the new_prompt into the prompt buffer.
 *
 * @param prompt Character array to store the prompt string.
 * @param new_prompt Optional new prompt string.
 */
void builtinPrompt(char prompt[], const char* new_prompt) {
    if (new_prompt == NULL || strlen(new_prompt) == 0) {
        strncpy(prompt, "%", 2);  // Default prompt
    } else {
        strncpy(prompt, new_prompt, strlen(new_prompt) + 1);  // Custom prompt
    }
}

/**
 * @brief Prints the current working directory to stdout.
 *
 * Uses getcwd to retrieve the current directory and prints it.
 */
void builtinPwd() {
    char dir[1024];

    if (getcwd(dir, sizeof(dir)) != NULL) {
        printf("%s\n", dir);
    }
}

/**
 * @brief Changes the current working directory.
 *
 * Attempts to change to the directory specified by path.
 * If the directory does not exist, prints an error message.
 *
 * @param path Target directory path.
 */
void builtinCD(const char* path) {
    if (path != NULL) {
        if (chdir(path) != 0) {
            printf("cd: no such file or directory\n");
        }
    }
}