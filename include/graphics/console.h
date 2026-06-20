#include "../limine/limine.h"
#include "graphics/vga.h"

#ifndef CONSOLE_H
#define CONSOLE_H

#include "fonts/niji-pixel-bold-16.h"

enum ProtoColors {
    PROTO_RED     = 0xFF6467,
    PROTO_GREEN   = 0x05DF72,
    PROTO_YELLOW  = 0xFDC700,
    PROTO_BLUE    = 0x51A2FF,
    PROTO_MAGENTA = 0xED6AFF,
    PROTO_CYAN    = 0x00D3F2,
    PROTO_WHITE   = 0xFFFFFF,
    PROTO_GREY    = 0x777777,
    PROTO_BG      = 0x000000,
};

typedef struct {
    char     ch;
    uint32_t fg;
    uint32_t bg;
} cell_t;

#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_WHITE   "\x1b[37m"
#define ANSI_RESET   "\x1b[0m"

extern uint64_t cursor_row;
extern uint64_t cursor_col;
extern color_t current_fg;
extern color_t current_bg;
extern uint64_t term_rows;
extern uint64_t term_cols;
extern uint64_t term_graphics_init;

void terminal_init();
void set_cursor(int row, int col);
void print(const char *str);
void print_f(const char *format, ...);
void set_color(color_t fg, color_t bg);

#endif // CONSOLE_H