#include <proto/core.h>
#include <proto/graphics.h>

#include "utils.h"
#include "window.h"
#include "globals.h"

window_t *g_window_stack;

void refresh_window(window_t *win) {
    draw_img(g_fb, (uint32_t *)win->fb->address, win->x+WIN_PADDING, win->y+TITLEBAR_HEIGHT, win->width, win->height);
}

// TODO: refresh_window_rect

void draw_window(window_t *win, int moving) {
    capture_rect(g_fb, win->under, win->x, win->y, win->owidth, win->oheight);

    if (moving == 1) {
        draw_rect_o(g_fb, win->x, win->y, win->owidth, win->oheight, 0xFFFFFF);
    } else {
        draw_rect(g_fb, win->x, win->y, win->owidth, TITLEBAR_HEIGHT, WIN_BACKGROUND);
        draw_box(g_fb, win->x, win->y, win->owidth, win->oheight, WIN_BACKGROUND, 2, 0);
        font_print(g_fb, g_small_font, win->name, win->x+5, win->y+(TITLEBAR_HEIGHT-g_small_font->height)/2, WIN_FOREGROUND);
        refresh_window(win);
    }
}

window_t *create_window(int x, int y, int w, int h, char *name) {
    window_t *win = malloc(sizeof(window_t));
    if (!win) { return NULL; }

    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    win->owidth = w+2*WIN_PADDING;
    win->oheight = h+TITLEBAR_HEIGHT+WIN_PADDING;
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

    LL_APPEND(win, g_window_stack);

    int outer_w = w + 2 * WIN_PADDING;
    int outer_h = h + TITLEBAR_HEIGHT + WIN_PADDING;

    win->under = malloc((size_t)outer_w * (size_t)outer_h * sizeof(uint32_t));
    if (!win->under) {
        return NULL;
    }

    capture_rect(g_fb, win->under, x, y, outer_w, outer_h);

    draw_window(win, 0);

    return win;
}