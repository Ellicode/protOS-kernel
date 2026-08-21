#ifndef WINDOW_H
#define WINDOW_H

#include <proto/graphics.h>

#define TITLEBAR_HEIGHT         16
#define TITLEBAR_BOTTOM_PADDING 2
#define WIN_PADDING             2
#define WIN_BACKGROUND          0xCCCCCC
#define WIN_FOREGROUND          0x000000
#define SHADOW_SIZE             6
typedef struct window_t {
    int id;
    int x;
    int y;
    int width;
    int height;
    int owidth;
    int oheight;

    char name[255];

    int frameless;
    int pid;

    fb_info_t *fb;

    struct window_t *next;
    struct window_t *prev;
} window_t;

extern window_t *g_window_stack;

void draw_window(window_t *win);
void _refresh_window(window_t *win);
window_t *_create_window(int x, int y, int w, int h, char *name, int frameless);
void draw_window_clipped(window_t *win, int cx, int cy, int cw, int ch); 
window_t *get_win_from_id(int id);

#endif // WINDOW_H
