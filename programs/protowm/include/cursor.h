#ifndef CURSOR_H
#define CURSOR_H

#define CURSOR_WIDTH    16
#define CURSOR_HEIGHT   16

extern int g_prev_x;
extern int g_prev_y;

void init_cursor();
void _draw_cursor_impl(int x, int y);
void draw_cursor(int x, int y);

#endif // CURSOR_H
