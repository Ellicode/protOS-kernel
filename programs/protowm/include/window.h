#ifndef WINDOW_H
#define WINDOW_H

#define TITLEBAR_HEIGHT 20
#define WIN_PADDING     2
#define WIN_BACKGROUND  0xBCC0CC
#define WIN_FOREGROUND  0x4C4F69

typedef struct window_t {
    int x;
    int y;
    int width;
    int height;
    int owidth;
    int oheight;

    char name[255];

    int frameless;

    fb_info_t *fb;
    uint32_t *under;

    struct window_t *next;
    struct window_t *prev;
} window_t;

extern window_t *g_window_stack;

void draw_window(window_t *win, int moving);
void refresh_window(window_t *win);
window_t *create_window(int x, int y, int w, int h, char *name);

#endif // WINDOW_H