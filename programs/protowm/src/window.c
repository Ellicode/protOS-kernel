#include <proto/core.h>
#include <proto/graphics.h>
#include <stdint.h>

#include "cursor.h"
#include "utils.h"
#include "window.h"
#include "globals.h"

window_t *g_window_stack;
int max_wid = 0;

void refresh_window_clipped(window_t *win, int cx, int cy, int cw, int ch) {
    if (win->frameless) {
        draw_img_clip(g_fb, (uint32_t *)win->fb->address, win->x, win->y, win->width, win->height, cx, cy, cw, ch);
    } else {
        draw_img_clip(g_fb, (uint32_t *)win->fb->address, win->x+WIN_PADDING+1, win->y+TITLEBAR_HEIGHT+WIN_PADDING+TITLEBAR_BOTTOM_PADDING+1, win->width, win->height, cx, cy, cw, ch); 
    }

    if (collide_point(g_prev_x, g_prev_y, win->x, win->y, win->width, win->height)) {
        _draw_cursor_impl(g_prev_x, g_prev_y); // Redraw cursor if it's under the window!
    }
}

void _refresh_window(window_t *win) {
    refresh_window_clipped(win, 0, 0, g_fb->width, g_fb->height);
}

void draw_window_clipped(window_t *win, int clip_x, int clip_y,
                         int clip_w, int clip_h) {
    if (!win->frameless) {
        int content_x = win->x + 1;
        int content_y = win->y + 1;
        int content_w = win->owidth - 2 - SHADOW_SIZE;
        int content_h = win->oheight - 2 - SHADOW_SIZE;

        draw_rect_clip(
            g_fb,
            content_x, content_y,
            content_w,
            TITLEBAR_HEIGHT + WIN_PADDING + TITLEBAR_BOTTOM_PADDING,
            WIN_BACKGROUND,
            clip_x, clip_y, clip_w, clip_h
        );

        draw_box_clip(
            g_fb,
            content_x, content_y,
            content_w, content_h,
            WIN_BACKGROUND,
            WIN_PADDING,
            0,
            clip_x, clip_y, clip_w, clip_h
        );

        draw_rect_c_clip(
            g_fb,
            content_x + WIN_PADDING,
            content_y + WIN_PADDING,
            content_w - 2 * WIN_PADDING,
            TITLEBAR_HEIGHT,
            WIN_BACKGROUND,
            darken(WIN_BACKGROUND, FX_ONE / 4),
            clip_x, clip_y, clip_w, clip_h
        );

        draw_text_clip(
            g_fb,
            g_small_font,
            win->name,
            content_x + WIN_PADDING + 2,
            content_y + WIN_PADDING +
                (TITLEBAR_HEIGHT - g_small_font->height) / 2,
            WIN_FOREGROUND,
            clip_x, clip_y, clip_w, clip_h
        );

        draw_rect_o_clip(
            g_fb,
            win->x,
            win->y,
            win->owidth - SHADOW_SIZE,
            win->oheight - SHADOW_SIZE,
            0x000000,
            clip_x, clip_y, clip_w, clip_h
        );

        draw_rect_c_clip(
            g_fb,
            win->x + SHADOW_SIZE,
            win->y + win->oheight - SHADOW_SIZE,
            win->owidth - SHADOW_SIZE,
            SHADOW_SIZE,
            0x00000000,
            0xFF000000,
            clip_x, clip_y, clip_w, clip_h
        );

        draw_rect_c_clip(
            g_fb,
            win->x + win->owidth - SHADOW_SIZE,
            win->y + SHADOW_SIZE,
            SHADOW_SIZE,
            win->oheight - SHADOW_SIZE,
            0x00000000,
            0xFF000000,
            clip_x, clip_y, clip_w, clip_h
        );
    }

    refresh_window_clipped(
        win,
        clip_x, clip_y, clip_w, clip_h
    );
}

void draw_window(window_t *win) {
    draw_window_clipped(win, 0, 0, g_fb->width, g_fb->height);
}

window_t *_create_window(int x, int y, int w, int h, char *name, int frameless) {
    window_t *win = malloc(sizeof(window_t));
    if (!win) { return NULL; }

    win->x = x;
    win->y = y;
    win->width = w;
    win->height = h;
    if (frameless) {
        win->owidth = w;
        win->oheight = h;
    } else {
        win->owidth = w+2*WIN_PADDING+SHADOW_SIZE+2;
        win->oheight = h+TITLEBAR_HEIGHT+2*WIN_PADDING+TITLEBAR_BOTTOM_PADDING+SHADOW_SIZE+2;
    }
    win->frameless = frameless;
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
    win->id     = max_wid++;

    LL_APPEND(win, g_window_stack);

    draw_window(win);

    return win;
}

window_t *get_win_from_id(int id) {
    window_t *win = g_window_stack;
    while (win != NULL) {
        if (win->id == id) {
            return win;
        }
        win = win->next;
    }
    return NULL;
}
