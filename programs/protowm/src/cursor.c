#include <proto/core.h>

#include "init.h"
#include "cursor.h"
#include "globals.h"
#include "window.h"

int g_prev_x = 0;
int g_prev_y = 0;
uint32_t *cursor_prev = NULL;

void init_cursor() {
    cursor_prev = malloc((size_t)CURSOR_WIDTH * CURSOR_HEIGHT * sizeof(uint32_t));
    capture_rect(g_fb, cursor_prev, 0, 0, CURSOR_WIDTH, CURSOR_HEIGHT);
}

void _draw_cursor_impl(int x, int y) {
    capture_rect(g_fb, cursor_prev, x, y, CURSOR_WIDTH, CURSOR_HEIGHT);
    bmp_draw(g_fb, g_cursor_bmp, x, y);

    g_prev_x = x;
    g_prev_y = y;
}

void draw_cursor(int x, int y) {
    draw_img(g_fb, cursor_prev, g_prev_x, g_prev_y, CURSOR_WIDTH, CURSOR_HEIGHT);
    _draw_cursor_impl(x, y);
}
