#include "utils.h"

static void childHandler(int sig);

void termiosMode(int enabled){
  // Create a static variable to hold the original terminal settings
  static struct termios original = {0};
  // This variable ensures that the original settings are saved once
  static int is_initialized = 0;

  if(!is_initialized){
    tcgetattr(STDIN_FILENO, &original);
    is_initialized = 1;
  }

  if(enabled){
    // Create a new termios structure based on the original settings
    struct termios new_termios = original;

    new_termios.c_lflag &= ~(ICANON | ECHO); 
    new_termios.c_cc[VMIN] = 1;
    new_termios.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
  }
  else{
    tcsetattr(STDIN_FILENO, TCSANOW, &original);
  }
}

int readLine(char* line){
  // Flag to indicate if newline is encountered
  int is_newline = 0;
  // The current index of the ANSI cursor position
  int index = strlen(line);
  // The total number of characters in the line
  int char_count = strlen(line);

  while(!is_newline){
    // Holds the input characters + special characters
    char ch[4] = {0};
    // Read the input from the terminal
    int bytes_read = read(STDIN_FILENO, ch, sizeof(ch));

    if(strcmp(ch, ANSI_NEWLINE_KEY) == 0){
      // Set the newline flag to true and break out of the loop
      is_newline = 1;

      printf("\n");
      fflush(stdout);
    }
    else if(strcmp(ch, ANSI_BACKSPACE_KEY) == 0){
      // This prevents the cursor from shifting left when at the start of the line.
      if(index > 0){
        index--;

        // Shift characters to the left to remove the character at the current index
        for(int i = index; i < char_count; i++)
          line[i] = line[i + 1];

        // Null-terminate the string and decrease the character count
        line[char_count] = '\0';
        char_count--;

        printf("%s%s", ANSI_LEFT_KEY, ANSI_DELETE_CMD);
        fflush(stdout);
      }
    }
    else if(strcmp(ch, ANSI_DELETE_KEY) == 0){
      // Prevent deletion if at the end of the line
      if(line[index] != '\0'){
        // Shift characters to the left to remove the character at the current index
        for(int i = index; i < char_count; i++)
          line[i] = line[i + 1];

        // Null-terminate the string and decrease the character count
        line[char_count] = '\0';
        char_count--;

        printf(ANSI_DELETE_CMD);
        fflush(stdout);
      }
    }
    else if(strcmp(ch, ANSI_LEFT_KEY) == 0){

      // Move cursor left if not at the start of the line
      if(index > 0){
        index--;

        printf(ANSI_LEFT_KEY);
        fflush(stdout);
      }
    }
    else if(strcmp(ch, ANSI_RIGHT_KEY) == 0){
      // Move cursor right if not at the end of the line
      if(line[index] != '\0'){
        index++;

        printf(ANSI_RIGHT_KEY);
        fflush(stdout);
      }
    }
    else if(strcmp(ch, ANSI_UP_KEY) == 0){
      return -1;
    }
    else if(strcmp(ch, ANSI_DOWN_KEY) == 0){
      return 1;
    }
    else{
      // Skip bytes that do not represent printable characters 
      if(bytes_read > 1) continue;

      char c = ch[0];

      // Shift characters to the right to make space for the new character
      for(int i = char_count; i >= index; i--)
        line[i + 1] = line[i];

      // Insert the new character at the current index and update counters
      line[index++] = c;
      char_count++;

      printf("%s%c", ANSI_INSERT_CMD, c);
      fflush(stdout);
    }
  }

  return 0;
}

void clearLine(char* line){
  for(int i = 0; line[i] != '\0'; i++)
    line[i] = '\0';
}

void installSignalHandlers(){
  struct sigaction sa = {0};
  sa.sa_handler = childHandler;

  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
  sigaction(SIGCHLD, &sa, NULL);

  // ignore job control signals in shell; children restore defaults
  sa.sa_handler = SIG_IGN;
  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGQUIT, &sa, NULL);
  sigaction(SIGTSTP, &sa, NULL);
}

static void childHandler(int sig){
  (void)sig;

  for(;;){
    int st; 
    pid_t p = waitpid(-1, &st, WNOHANG);
    
    if(p > 0) continue;
    if(p == 0) break;
    if(p < 0 && errno == EINTR) continue;
    break;
  }
}