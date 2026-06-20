#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#include "debug/serial.h"
#include "string.h"
#include "globals.h"
#include "memory/heap.h"

#include "graphics/console.h"

uint64_t cursor_row         = 0;
uint64_t cursor_col         = 0;
color_t current_fg          = PROTO_WHITE;
color_t current_bg          = PROTO_BG;
uint64_t term_rows          = PROTO_BG;
uint64_t term_cols          = PROTO_BG;
uint64_t term_graphics_init = 0;

cell_t *grid;

void terminal_init() {
    term_cols = g_vga_active_framebuffer->width / (FONT_WIDTH + FONT_KERNING);
    term_rows = g_vga_active_framebuffer->height / FONT_HEIGHT;

    grid = k_alloc(sizeof(cell_t) * term_rows * term_cols);
    for (uint64_t i = 0; i < term_rows * term_cols; i++) {
        grid[i].ch = ' ';
        grid[i].fg = current_fg;
        grid[i].bg = current_bg;
    }

    cursor_row = 0;
    cursor_col = 0;
    term_graphics_init = 1;
}

cell_t *cell_at(int row, int col) {
    return &grid[row * term_cols + col];
}

void render_char(int row, int col) {
    cell_t *cell = cell_at(row, col);

    int x = col * (FONT_WIDTH + FONT_KERNING);
    int y = row * FONT_HEIGHT;

    color_t fg = cell->fg;
    color_t bg = cell->bg;

    if (row == cursor_row && col == cursor_col) {
        color_t tmp = fg;
        fg = bg;
        bg = tmp;
    }

    for (int r = 0; r < FONT_HEIGHT; r++) {
        // Force flip the array reading layout vertically
        unsigned char row_data = font[cell->ch - 32][(FONT_HEIGHT - 1) - r];

        for (int c = 0; c < FONT_WIDTH; c++) {
            // Read bits from left to right 
            if (row_data & (1 << ((FONT_WIDTH - 1) - c))) {
                putpixel(x + c, y + r, fg);
            } else {
                putpixel(x + c, y + r, bg);
            }
        }
    }

    if (cell->bg != PROTO_BG) {
        draw_rect(x-FONT_KERNING, y, FONT_KERNING, FONT_HEIGHT, cell->bg); // fill in the gaps!
    }
}

void put_char(int row, int col, char c) {
    if (term_graphics_init) {
        cell_t *cell = cell_at(row, col);
        
        cell->ch = c;
        cell->fg = current_fg;
        cell->bg = current_bg;

        render_char(row, col);
    } 
    serial_write(c);
}

void set_cursor(int row, int col) {
    int old_row = cursor_row;
    int old_col = cursor_col;

    cursor_row = row;
    cursor_col = col;

    render_char(old_row, old_col);
    render_char(cursor_row, cursor_col);
}


void set_color(color_t fg, color_t bg) {
    current_fg = fg;
    current_bg = bg;
}

void print_char(char c) {
    if (term_graphics_init) {
        if (c == '\n') {
            set_cursor(cursor_row+1, 0);
        } else if (c == '\b') {
            put_char(cursor_row, cursor_col-1, ' ');
            set_cursor(cursor_row, cursor_col-1);
        } else {
            put_char(cursor_row, cursor_col, c);

            if (cursor_col >= term_cols) {
                set_cursor(cursor_row+1, 0);
            } else {
                set_cursor(cursor_row, cursor_col+1);
            }
        }
    } else {
        put_char(cursor_row, cursor_col, c); // we don't care about the row and col since we're outputting via serial
    }
}

void print_ansi() {
    const char *ansi_code = ANSI_RESET;

    if (current_fg == PROTO_RED)         { ansi_code = ANSI_RED; }
    else if (current_fg == PROTO_YELLOW) { ansi_code = ANSI_YELLOW; }
    else if (current_fg == PROTO_GREEN)  { ansi_code = ANSI_GREEN; }
    else if (current_fg == PROTO_CYAN)   { ansi_code = ANSI_CYAN; }
    else if (current_fg == PROTO_BLUE)   { ansi_code = ANSI_BLUE; }
    else if (current_fg == PROTO_MAGENTA){ ansi_code = ANSI_MAGENTA; }
    else if (current_fg == PROTO_WHITE)  { ansi_code = ANSI_WHITE; };

    // First print the color ansi code
    for (size_t c = 0; ansi_code[c] != '\0'; c++) {
        serial_write(ansi_code[c]);
    }
}

void print(const char *str) {
    int len = strlen(str);
    print_ansi();
    for (size_t c = 0; c < len; c++){
        print_char(str[c]);
    }
}

void print_f(const char *format, ...)
{
    va_list args;
    va_start(args, format);

    char buffer[1024];
    buffer[0] = '\0';
    int buffer_index = 0;

    for (size_t i = 0; format[i] != '\0'; i++)
    {
        if (format[i] == '%')
        {
            i++;

            char pad = ' ';
            int width = 0;

            if (format[i] == '0')
            {
                pad = '0';
                i++;
            }

            while (format[i] >= '0' && format[i] <= '9')
            {
                width = width * 10 + (format[i] - '0');
                i++;
            }

            if (format[i] == 'd')
            {
                int value = va_arg(args, int);
                char *str = int_to_string(value);

                int len = strlen(str);
                while (len < width)
                {
                    buffer[buffer_index++] = pad;
                    len++;
                }

                while (*str)
                {
                    buffer[buffer_index++] = *str++;
                }
                buffer[buffer_index] = '\0';
            }
            else if (format[i] == 'x')
            {
                uint64_t value = va_arg(args, uint64_t);
                char *str = hex_to_string(value);

                int len = strlen(str);
                while (len < width)
                {
                    buffer[buffer_index++] = pad;
                    len++;
                }

                while (*str)
                {
                    buffer[buffer_index++] = *str++;
                }
                buffer[buffer_index] = '\0';
            }
            else if (format[i] == 's')
            {
                char *str = va_arg(args, char *);
                if (str != NULL) {
                    int len = strlen(str);
                    while (len < width)
                    {
                        buffer[buffer_index++] = pad;
                        len++;
                    }

                    while (*str)
                    {
                        buffer[buffer_index++] = *str++;
                    }
                }
                buffer[buffer_index] = '\0';
            }
            else if (format[i] == '%')
            {
                buffer[buffer_index++] = '%';
                buffer[buffer_index] = '\0';
            }
        }
        else
        {
            buffer[buffer_index++] = format[i];
            buffer[buffer_index] = '\0';
        }
    }

    va_end(args);
    print(buffer);
}