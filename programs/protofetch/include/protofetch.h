#ifndef PROTOFETCH_H
#define PROTOFETCH_H

#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_WHITE   "\x1b[37m"
#define ANSI_RESET   "\x1b[0m"

#define PROTOGEN_ASCII  "  /\\    /\\        \n" \
                        " /  \\__/  \\       \n" \
                        "/|\\ //  ~~~~~~+   \n" \
                        " | \\/          \\  \n" \
                        " \\( )  __     __\\ \n" \
                        "   \\\\     \\/\\/  / \n" \
                        "\\___\\\\_________/  \n" \

#define SWATCH_SYMBOL "O "

#define PROTOGEN_ASCII_WIDTH    17
#define PROTOGEN_ASCII_HEIGHT   7

#define PROTOFETCH_ROWS PROTOGEN_ASCII_HEIGHT

#endif // PROTOFETCH_H