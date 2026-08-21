#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>


// PASTEL ============================
/*
enum ProtoColors {
    PROTO_RED     = 0xFF6467,
    PROTO_GREEN   = 0x05DF72,
    PROTO_YELLOW  = 0xFDC700,
    PROTO_BLUE    = 0x51A2FF,
    PROTO_MAGENTA = 0xED6AFF,
    PROTO_CYAN    = 0x00D3F2,
    PROTO_WHITE   = 0xFFFFFF,
    PROTO_GREY    = 0x777777,
    PROTO_BLACK   = 0x000000,
    PROTO_BG      = 0x000000,
};
*/

// VIBRANT ============================
enum ProtoColors {
    PROTO_RED     = 0xef4444,
    PROTO_GREEN   = 0x22c55e,
    PROTO_YELLOW  = 0xeab308,
    PROTO_BLUE    = 0x3b82f6,
    PROTO_MAGENTA = 0xd946ef,
    PROTO_CYAN    = 0x06b6d4,
    PROTO_WHITE   = 0xFFFFFF,
    PROTO_GREY    = 0x6b7280,
    PROTO_BLACK   = 0x000000,
    PROTO_BG      = 0x000000,
};

// MOCHA ============================
/*
enum ProtoColors {
    PROTO_RED     = 0xf38ba8,
    PROTO_GREEN   = 0xa6e3a1,
    PROTO_YELLOW  = 0xf9e2af,
    PROTO_BLUE    = 0x89b4fa,
    PROTO_MAGENTA = 0xf5c2e7,
    PROTO_CYAN    = 0x94e2d5,
    PROTO_WHITE   = 0xcdd6f4,
    PROTO_GREY    = 0x7f849c,
    PROTO_BLACK   = 0x1e1e2e,
    PROTO_BG      = 0x1e1e2e,
};
*/

// OLDSKOOL ============================
/*
enum ProtoColors {
    PROTO_RED     = 0xFF0000,
    PROTO_GREEN   = 0x00FF00,
    PROTO_YELLOW  = 0xFFFF00,
    PROTO_BLUE    = 0x0000FF,
    PROTO_MAGENTA = 0xFF00FF,
    PROTO_CYAN    = 0x00FFFF,
    PROTO_WHITE   = 0xFFFFFF,
    PROTO_GREY    = 0x999999,
    PROTO_BLACK   = 0x000000,
    PROTO_BG      = 0x000000,
};
*/
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

extern int          g_cursor_row;
extern int          g_cursor_col;
extern uint32_t     g_current_fg;
extern uint32_t     g_current_bg;
extern int          g_term_rows;
extern int          g_term_cols;
extern bool         g_term_graphics_init;

extern bool         g_kbd_enable;

void term_clear_buffer();
void terminal_init(fb_info_t *fb, font_t *fnt);
void term_set_cursor(int row, int col);
void print_char(char c);
void print(const char *str);
void set_color(uint32_t fg, uint32_t bg);

#endif // CONSOLE_H
