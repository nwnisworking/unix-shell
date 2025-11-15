#ifndef ANSI_H
#define ANSI_H

#define ANSI_ESCAPE_KEY "\x1b"
#define ANSI_LEFT_KEY "\x1b[D"
#define ANSI_RIGHT_KEY "\x1b[C"
#define ANSI_UP_KEY "\x1b[A"
#define ANSI_DOWN_KEY "\x1b[B"
#define ANSI_DELETE_KEY "\x1b[3~"
#define ANSI_NEWLINE_KEY "\n"
#define ANSI_BACKSPACE_KEY "\x7f"

#define ANSI_DELETE_CMD "\x1b[P"
#define ANSI_INSERT_CMD "\x1b[@"
#define ANSI_CLEAR_LINE_CMD "\x1b[2K\r"

#endif // ANSI_H