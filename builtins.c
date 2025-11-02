#include "builtins.h"

// Sets the shell prompt string.
// If no new prompt is provided or it's empty, defaults to "%". 
// This ensures the shell always has a visible prompt, even if the user doesn't customize it.
void builtinPrompt(char prompt[], const char* new_prompt) {
    if (new_prompt == NULL || strlen(new_prompt) == 0) {
        strncpy(prompt, "%", 2);  // Default fallback prompt
    } else {
        // Copies the new prompt string into the prompt buffer.
        // Uses strlen + 1 to include the null terminator.
        strncpy(prompt, new_prompt, strlen(new_prompt) + 1);
    }
}

// Prints the current working directory.
// Uses getcwd to retrieve the absolute path and prints it to stdout.
// This is a standard utility in shells to confirm the current location in the filesystem.
void builtinPwd() {
    char dir[1024];

    if (getcwd(dir, sizeof(dir)) != NULL) {
        printf("%s\n", dir);
    }
}

// Changes the current working directory to the specified path.
// If the path is invalid or inaccessible, prints an error message.
// This mirrors the behavior of the 'cd' command in Unix-like shells.
void builtinCD(const char* path) {
    if (path != NULL) {
        if (chdir(path) != 0) {
            printf("cd: no such file or directory\n");
        }
    }
}