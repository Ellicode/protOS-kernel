#ifndef WINDOW_H
#define WINDOW_H

#define TITLEBAR_HEIGHT 20

typedef struct window_t {
    int x;
    int y;
    int width;
    int height;
    char name[255];

    int frameless;

    fb_info_t *fb;

    struct window_t *next;
    struct window_t *prev;
} window_t;

void draw_window(window_t *win);
window_t *create_window(int x, int y, int w, int h, char *name);

#endif // WINDOW_H