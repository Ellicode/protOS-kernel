#include "../limine/limine.h"

#include "vga.h"

#ifndef CONSOLE_H
#define CONSOLE_H

#define FONT_WIDTH 8
#define FONT_HEIGHT 16
#define FONT_KERNING 2

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

#define ANSI_RED     "\x1b[31m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_WHITE   "\x1b[37m"
#define ANSI_RESET   "\x1b[0m"

void cursor_set(uint32_t x, uint32_t y);
void cursor_newline(void);
void draw_char(uint32_t x, uint32_t y, char c, color_t color);
void draw_text(uint32_t x, uint32_t y, const char *text, color_t color);
void print(const char *text);
void print_f(const char *format, ...);
void set_color(color_t color);

#endif // CONSOLE_H