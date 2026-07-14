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

const color_t ANSI_PALETTE[16] = {
    PROTO_BLACK, // 0: Black
    PROTO_RED, // 1: Red
    PROTO_GREEN, // 2: Green
    PROTO_YELLOW, // 3: Yellow
    PROTO_BLUE, // 4: Blue
    PROTO_MAGENTA, // 5: Magenta
    PROTO_CYAN, // 6: Cyan
    PROTO_WHITE,  // 7: White / Default

    PROTO_GREY, // 0: Black
    PROTO_RED, // 1: Red
    PROTO_GREEN, // 2: Green
    PROTO_YELLOW, // 3: Yellow
    PROTO_BLUE, // 4: Blue
    PROTO_MAGENTA, // 5: Magenta
    PROTO_CYAN, // 6: Cyan
    PROTO_WHITE  // 7: White / Default
};

cell_t *grid;

void term_clear_buffer() {
    for (uint64_t i = 0; i < term_rows * term_cols; i++) {
        grid[i].ch = ' ';
        grid[i].fg = current_fg;
        grid[i].bg = current_bg;
    }
}

void terminal_init() {
    term_cols = g_vga_active_framebuffer->width / (FONT_WIDTH + FONT_KERNING);
    term_rows = g_vga_active_framebuffer->height / FONT_HEIGHT;

    grid = k_alloc(sizeof(cell_t) * term_rows * term_cols);
    if (grid == NULL) {
        return;
    }

    term_clear_buffer();

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
    draw_rect(x-FONT_KERNING, y, FONT_KERNING, FONT_HEIGHT, cell->bg); // fill in the gaps!
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

void _term_refresh() {
    for (uint64_t row = 0; row < term_rows; row++) {
        for (uint64_t col = 0; col < term_cols; col++) {
            render_char(row, col);
        }
    }
}

void scroll_terminal() {
    // shift every row up by one
    for (uint64_t row = 1; row < term_rows; row++) {
        for (uint64_t col = 0; col < term_cols; col++) {
            cell_t *src = cell_at(row, col);
            cell_t *dst = cell_at(row - 1, col);
            *dst = *src;
        }
    }

    // clear the last row
    uint64_t last_row = term_rows - 1;
    for (uint64_t col = 0; col < term_cols; col++) {
        cell_t *cell = cell_at(last_row, col);
        cell->ch = ' ';
        cell->fg = current_fg;
        cell->bg = current_bg;
    }

    _term_refresh();
}

void set_cursor(int row, int col) {
    if (row >= term_rows) {
        scroll_terminal();
        row = term_rows - 1;
    }

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
            put_char(cursor_row, cursor_col, ' ');
            serial_write('\n');
            set_cursor(cursor_row+1, 0);
        } else if (c == '\b') {
            serial_write('\b');
            put_char(cursor_row, cursor_col-1, ' ');
            serial_write('\b');
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
    size_t len = strlen(str);
    size_t c = 0;
    
    print_ansi();

    while (c < len) {
        if (str[c] == '\x1b' && (c + 1 < len) && str[c+1] == '[') {
            c += 2; // Skip '\x1b' and '['
            
            int param = 0;
            bool param_empty = true;

            while (c < len) {
                if (str[c] >= '0' && str[c] <= '9') {
                    param = param * 10 + (str[c] - '0');
                    param_empty = false;
                } else if (str[c] == ';' || str[c] == 'm') {
                    if (param_empty) { param = 0; }

                    if (param == 0) {
                        // Reset to defaults
                        current_fg = ANSI_PALETTE[7]; 
                        current_bg = ANSI_PALETTE[0];
                    } else if (param >= 30 && param <= 37) {
                        // Foreground colors
                        current_fg = ANSI_PALETTE[param - 30];
                    } else if (param >= 40 && param <= 47) {
                        // Background colors
                        current_bg = ANSI_PALETTE[param - 40];
                    }

                    set_color(current_fg, current_bg);

                    print_ansi();

                    if (str[c] == 'm') {
                        c++;
                        break; 
                    }
                    
                    param = 0;
                    param_empty = true;
                } else {
                    break; 
                }
                c++;
            }
        } else {
            print_char(str[c]);
            c++;
        }
    }
}

void print_f(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    print(buffer);
}