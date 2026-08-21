#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <proto/core.h>
#include <proto/graphics.h>

#include "console.h"

int g_cursor_row                = 0;
int g_cursor_col                = 0;
uint32_t g_current_fg           = PROTO_WHITE;
uint32_t g_current_bg           = PROTO_BG;
int g_term_rows                 = PROTO_BG;
int g_term_cols                 = PROTO_BG;
bool g_term_graphics_init       = 0;
bool g_kbd_enable               = true;
font_t *g_font                  = NULL;
fb_info_t *g_fb                 = NULL;

const uint32_t ANSI_PALETTE[16] = {
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
    for (int i = 0; i < g_term_rows * g_term_cols; i++) {
        grid[i].ch = ' ';
        grid[i].fg = g_current_fg;
        grid[i].bg = g_current_bg;
    }
}

void terminal_init(fb_info_t *fb, font_t *fnt) {
    g_term_cols = fb->width / fnt->width;
    g_term_rows = fb->height / fnt->height;

    grid = malloc(sizeof(cell_t) * g_term_rows * g_term_cols);
    if (grid == NULL) {
        return;
    }

    term_clear_buffer();

    g_cursor_row = 0;
    g_cursor_col = 0;
    g_term_graphics_init = true;
    g_fb = fb;
    g_font = fnt;
}

cell_t *cell_at(int row, int col) {
    return &grid[row * g_term_cols + col];
}


void put_char(int row, int col, char c) {
    if (g_term_graphics_init == true) {
        cell_t *cell = cell_at(row, col);
        
        cell->ch = c;
        cell->fg = g_current_fg;
        cell->bg = g_current_bg;

        (row, col);
    } 
}

void render_char(int row, int col) {
    cell_t *cell = cell_at(row, col);

    int x = col * g_font->width;
    int y = row * g_font->height;

    uint32_t fg = cell->fg;
    uint32_t bg = cell->bg;

    if (row == g_cursor_row && col == g_cursor_col) {
        uint32_t tmp = fg;
        fg = bg;
        bg = tmp;
    }

    draw_rect(g_fb, x, y, g_font->width, g_font->height, bg);
    draw_char(g_fb, g_font, cell->ch, x, y, fg);
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

    uint32_t *fb_ptr = (uint32_t *)g_fb->address;
    uint32_t fb_pitch = g_fb->pitch / 4;
    uint32_t scroll_rows = (uint32_t)(g_term_rows - 1) * g_font->height;
    memmove(fb_ptr,
            fb_ptr + g_font->height * fb_pitch,
            sizeof(uint32_t) * fb_pitch * scroll_rows);

    if (g_cursor_row > 0) {
        render_char(g_cursor_row - 1, g_cursor_col);
    }

    for (int col = 0; col < g_term_cols; col++) {
        render_char(last_row, col);
    }
}

void term_set_cursor(int row, int col) {
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

void set_color(uint32_t fg, uint32_t bg) {
    g_current_fg = fg;
    g_current_bg = bg;
}

void print_char(char c) {
    if (g_term_graphics_init) {
        if (c == '\n') {
            put_char(g_cursor_row, g_cursor_col, ' ');
            term_set_cursor(g_cursor_row+1, 0);
        } else if (c == '\b') {
            put_char(g_cursor_row, g_cursor_col-1, ' ');
            term_set_cursor(g_cursor_row, g_cursor_col-1);
        } else {
            cell_t *cell = cell_at(g_cursor_row, g_cursor_col);
            cell->ch = c;
            cell->fg = g_current_fg;
            cell->bg = g_current_bg;

            int new_col = g_cursor_col + 1;
            int new_row = g_cursor_row;
            if (new_col >= g_term_cols) {
                new_col = 0;
                new_row++;
            }
            term_set_cursor(new_row, new_col);
        }
    } else {
        put_char(g_cursor_row, g_cursor_col, c); // we don't care about the row and col since we're outputting via serial
    }
}

void print(const char *str) {
    size_t len = strlen(str);
    size_t c = 0;
    
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
