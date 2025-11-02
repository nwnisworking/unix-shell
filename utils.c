#include "utils.h"

static void childHandler(int sig);

void termiosMode(int enabled){
    static struct termios original = {0}; // save original settings
    static int is_initialized = 0;

    if(!is_initialized){
        tcgetattr(STDIN_FILENO, &original);
        is_initialized = 1;
    }

    if(enabled){
        struct termios new_termios = original;

        new_termios.c_lflag &= ~(ICANON | ECHO);
        new_termios.c_cc[VMIN] = 1;
        new_termios.c_cc[VTIME] = 0;

        tcsetattr(STDIN_FILENO, TCSANOW, &new_termios);
    }else{
        tcsetattr(STDIN_FILENO, TCSANOW, &original);
    }
}

int readLine(char* line){
    int is_newline = 0;
    int index = strlen(line);
    int char_count = strlen(line);

    while(!is_newline){
        char ch[4] = {0};
        int bytes_read = read(STDIN_FILENO, ch, sizeof(ch));

        if(strcmp(ch, ANSI_NEWLINE_KEY) == 0){
            is_newline = 1;
            printf("\n");
            fflush(stdout);
        }
        else if(strcmp(ch, ANSI_BACKSPACE_KEY) == 0){
            if(index > 0){
                index--;
                for(int i = index; i < char_count; i++)
                    line[i] = line[i + 1];
                line[char_count] = '\0';
                char_count--;
                printf("%s%s", ANSI_LEFT_KEY, ANSI_DELETE_CMD);
                fflush(stdout);
            }
        }
        else if(strcmp(ch, ANSI_DELETE_KEY) == 0){
            if(line[index] != '\0'){
                for(int i = index; i < char_count; i++)
                    line[i] = line[i + 1];
                line[char_count] = '\0';
                char_count--;
                printf(ANSI_DELETE_CMD);
                fflush(stdout);
            }
        }
        else if(strcmp(ch, ANSI_LEFT_KEY) == 0){
            if(index > 0){
                index--;
                printf(ANSI_LEFT_KEY);
                fflush(stdout);
            }
        }
        else if(strcmp(ch, ANSI_RIGHT_KEY) == 0){
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
            if(bytes_read > 1) continue;

            char c = ch[0];
            for(int i = char_count; i >= index; i--)
                line[i + 1] = line[i];
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

    // ignore job control signals in shell
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