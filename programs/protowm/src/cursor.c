#include <proto/core.h>

#include "init.h"
#include "cursor.h"
#include "globals.h"

int prev_x = 0;
int prev_y = 0;
uint32_t *cursor_prev = NULL;

void init_cursor() {
    cursor_prev = malloc((size_t)CURSOR_WIDTH * CURSOR_HEIGHT * sizeof(uint32_t));
    capture_rect(g_fb, cursor_prev, 0, 0, CURSOR_WIDTH, CURSOR_HEIGHT);
}

void draw_cursor(int x, int y) {
    draw_img(g_fb, cursor_prev, prev_x, prev_y, CURSOR_WIDTH, CURSOR_HEIGHT);
    capture_rect(g_fb, cursor_prev, x, y, CURSOR_WIDTH, CURSOR_HEIGHT);
    bmp_draw(g_fb, g_cursor_bmp, x, y);

    prev_x = x;
    prev_y = y;
}