#include <proto/core.h>
#include <proto/graphics.h>

#include "utils.h"
#include "window.h"
#include "globals.h"

window_t *window_stack;

void draw_window(window_t *win) {
    draw_rect(g_fb, win->x, win->y, win->width, win->height + TITLEBAR_HEIGHT, 0x1e1e2e);
    font_print(g_fb, g_small_font, win->name, win->x+5, win->y+(TITLEBAR_HEIGHT-g_small_font->height)/2, 0xFFFFFF);
    draw_img(g_fb, (uint32_t *)win->fb->address, win->x, win->y + TITLEBAR_HEIGHT, win->width, win->height);
}

window_t *create_window(int x, int y, int w, int h, char *name) {
    window_t *win = malloc(sizeof(window_t));
    if (!win) { return NULL; }

    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    strncpy(win->name, name, 255);

    size_t fb_size = (size_t)w * h;
    fb_info_t *fb = malloc(sizeof(fb_info_t));
    if (!fb) {
        free(win);
        return NULL;
    }

    fb->bpp       = g_fb->bpp;
    fb->width     = w;
    fb->height    = h;
    fb->pitch     = (uint64_t)w * fb->bpp / 8;

    void* fb_data = malloc(fb->pitch * h);
    if (!fb_data) {
        free(win);
        free(fb);
        return NULL;
    }

    fb->address = (uint64_t)fb_data;
    win->fb     = fb;

    LL_APPEND(win, window_stack);
    draw_window(win);

    return win;
}