#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdbool.h>

#include "debug/serial.h"
#include "string.h"
#include "globals.h"
#include "memory/heap.h"

#include "graphics/console.h"

int g_cursor_row                = 0;
int g_cursor_col                = 0;
color_t g_current_fg            = PROTO_WHITE;
color_t g_current_bg            = PROTO_BG;
int g_term_rows                 = PROTO_BG;
int g_term_cols                 = PROTO_BG;
bool g_term_graphics_init       = 0;
bool g_kbd_enable               = true;

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
    for (uint64_t i = 0; i < g_term_rows * g_term_cols; i++) {
        grid[i].ch = ' ';
        grid[i].fg = g_current_fg;
        grid[i].bg = g_current_bg;
    }
}

void terminal_init() {
    g_term_cols = g_framebuffer->width / (FONT_WIDTH + FONT_KERNING);
    g_term_rows = g_framebuffer->height / FONT_HEIGHT;

    grid = k_alloc(sizeof(cell_t) * g_term_rows * g_term_cols);
    if (grid == NULL) {
        return;
    }

    term_clear_buffer();

    g_cursor_row = 0;
    g_cursor_col = 0;
    g_term_graphics_init = true;
}

cell_t *cell_at(int row, int col) {
    return &grid[row * g_term_cols + col];
}

void render_char(int row, int col) {
    cell_t *cell = cell_at(row, col);

    int x = col * (FONT_WIDTH + FONT_KERNING);
    int y = row * FONT_HEIGHT;

    color_t fg = cell->fg;
    color_t bg = cell->bg;

    if (row == g_cursor_row && col == g_cursor_col) {
        color_t tmp = fg;
        fg = bg;
        bg = tmp;
    }

    uint32_t *fb_ptr = g_framebuffer->address;
    uint32_t fb_pitch = g_framebuffer->pitch / 4;

    for (int r = 0; r < FONT_HEIGHT; r++) {
        // Force flip the array reading layout vertically
        unsigned char row_data = font[cell->ch - 32][(FONT_HEIGHT - 1) - r];
        uint32_t *row_ptr = fb_ptr + (y + r) * fb_pitch;

        // Fill the kerning gap with background (skip for the first column where x < FONT_KERNING)
        if (col > 0) {
            for (int k = 0; k < FONT_KERNING; k++) {
                row_ptr[x - FONT_KERNING + k] = (uint32_t)cell->bg;
            }
        }

        for (int c = 0; c < FONT_WIDTH; c++) {
            // Read bits from left to right 
            row_ptr[x + c] = (row_data & (1 << ((FONT_WIDTH - 1) - c))) ? (uint32_t)fg : (uint32_t)bg;
        }
    }
}

void put_char(int row, int col, char c) {
    if (g_term_graphics_init == true) {
        cell_t *cell = cell_at(row, col);
        
        cell->ch = c;
        cell->fg = g_current_fg;
        cell->bg = g_current_bg;

        render_char(row, col);
    } 
    serial_write(c);
}

void _term_refresh() {
    for (int row = 0; row < g_term_rows; row++) {
        for (int col = 0; col < g_term_cols; col++) {
            render_char(row, col);
        }
    }
}

void scroll_terminal() {
    // shift cell data up by one row
    memmove(&grid[0], &grid[g_term_cols], sizeof(cell_t) * g_term_cols * (g_term_rows - 1));

    // clear the last row in the cell buffer
    uint64_t last_row = g_term_rows - 1;
    for (int col = 0; col < g_term_cols; col++) {
        cell_t *cell = cell_at(last_row, col);
        cell->ch = ' ';
        cell->fg = g_current_fg;
        cell->bg = g_current_bg;
    }

    // Scroll framebuffer pixels up by one character row using a single memmove
    uint32_t *fb_ptr = g_framebuffer->address;
    uint32_t fb_pitch = g_framebuffer->pitch / 4;
    uint32_t scroll_rows = (uint32_t)(g_term_rows - 1) * FONT_HEIGHT;
    memmove(fb_ptr,
            fb_ptr + FONT_HEIGHT * fb_pitch,
            sizeof(uint32_t) * fb_pitch * scroll_rows);

    // Only re-render the newly cleared last row
    for (int col = 0; col < g_term_cols; col++) {
        render_char(last_row, col);
    }
}

void set_cursor(int row, int col) {
    if (row >= g_term_rows) {
        scroll_terminal();
        row = g_term_rows - 1;
    }

    int old_row = g_cursor_row;
    int old_col = g_cursor_col;

    g_cursor_row = row;
    g_cursor_col = col;

    render_char(old_row, old_col);
    render_char(g_cursor_row, g_cursor_col);
}

void set_color(color_t fg, color_t bg) {
    g_current_fg = fg;
    g_current_bg = bg;
}

void print_char(char c) {
    if (g_term_graphics_init) {
        if (c == '\n') {
            put_char(g_cursor_row, g_cursor_col, ' ');
            serial_write('\n');
            set_cursor(g_cursor_row+1, 0);
        } else if (c == '\b') {
            serial_write('\b');
            put_char(g_cursor_row, g_cursor_col-1, ' ');
            serial_write('\b');
            set_cursor(g_cursor_row, g_cursor_col-1);
        } else {
            // Update the cell data first, before moving the cursor.
            // This way set_cursor's re-render of the old position draws the
            // character correctly (non-inverted) in a single pass, avoiding
            // the previous pattern of 3 render_char calls per typed character.
            cell_t *cell = cell_at(g_cursor_row, g_cursor_col);
            cell->ch = c;
            cell->fg = g_current_fg;
            cell->bg = g_current_bg;
            serial_write(c);

            int new_col = g_cursor_col + 1;
            int new_row = g_cursor_row;
            if (new_col >= g_term_cols) {
                new_col = 0;
                new_row++;
            }
            set_cursor(new_row, new_col);
        }
    } else {
        put_char(g_cursor_row, g_cursor_col, c); // we don't care about the row and col since we're outputting via serial
    }
}

void print_ansi() {
    const char *ansi_code = ANSI_RESET;

    if (g_current_fg == PROTO_RED)         { ansi_code = ANSI_RED; }
    else if (g_current_fg == PROTO_YELLOW) { ansi_code = ANSI_YELLOW; }
    else if (g_current_fg == PROTO_GREEN)  { ansi_code = ANSI_GREEN; }
    else if (g_current_fg == PROTO_CYAN)   { ansi_code = ANSI_CYAN; }
    else if (g_current_fg == PROTO_BLUE)   { ansi_code = ANSI_BLUE; }
    else if (g_current_fg == PROTO_MAGENTA){ ansi_code = ANSI_MAGENTA; }
    else if (g_current_fg == PROTO_WHITE)  { ansi_code = ANSI_WHITE; };

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
                        g_current_fg = ANSI_PALETTE[7]; 
                        g_current_bg = ANSI_PALETTE[0];
                    } else if (param >= 30 && param <= 37) {
                        // Foreground colors
                        g_current_fg = ANSI_PALETTE[param - 30];
                    } else if (param >= 40 && param <= 47) {
                        // Background colors
                        g_current_bg = ANSI_PALETTE[param - 40];
                    }

                    set_color(g_current_fg, g_current_bg);

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