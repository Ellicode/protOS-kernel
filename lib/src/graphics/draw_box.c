#include <proto/graphics.h>
#include <stddef.h>

void draw_box_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int border,
    char fill,
    int cx, int cy, int cw, int ch
) {
    if (!fb || !fb->address || w <= 0 || h <= 0)
        return;

    if (fill) {
        draw_rect_clip(
            fb,
            x, y, w, h,
            color,
            cx, cy, cw, ch
        );
    }

    if (border <= 0)
        return;

    if (border > w / 2)
        border = w / 2;

    if (border > h / 2)
        border = h / 2;

    if (border <= 0)
        return;

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    size_t pitch = fb->pitch / sizeof(uint32_t);

    uint32_t light = lighten(color, FX_ONE / 4);
    uint32_t dark  = darken(color, FX_ONE / 4);

    // bottom bevel
    for (int row = 0; row < border; row++) {
        int inset = (border - 1) - row;

        int x0 = x + inset;
        int x1 = x + w - inset;

        int y0 = y + h - border + row;

        int rx, ry, rw, rh;

        if (!clip_rect(
            fb,
            x0, y0, x1 - x0, 1,
            cx, cy, cw, ch,
            &rx, &ry, &rw, &rh
        )) {
            continue;
        }

        uint32_t *dst =
            fb_ptr + (size_t)ry * pitch + rx;

        for (int i = 0; i < rw; i++)
            dst[i] = dark;
    }

    // top bevel
    for (int row = 0; row < border; row++) {
        int x0 = x + row;
        int x1 = x + w - row;

        int y0 = y + row;

        int rx, ry, rw, rh;

        if (!clip_rect(
            fb,
            x0, y0, x1 - x0, 1,
            cx, cy, cw, ch,
            &rx, &ry, &rw, &rh
        )) {
            continue;
        }

        uint32_t *dst =
            fb_ptr + (size_t)ry * pitch + rx;

        for (int i = 0; i < rw; i++)
            dst[i] = light;
    }

    // left bevel
    for (int col = 0; col < border; col++) {
        int y0 = y + col;
        int y1 = y + h - col;

        int x0 = x + col;

        int rx, ry, rw, rh;

        if (!clip_rect(
            fb,
            x0, y0, 1, y1 - y0,
            cx, cy, cw, ch,
            &rx, &ry, &rw, &rh
        )) {
            continue;
        }

        for (int i = 0; i < rh; i++) {
            fb_ptr[(size_t)(ry + i) * pitch + rx] = light;
        }
    }

    // right bevel
    for (int col = 0; col < border; col++) {
        int inset = (border - 1) - col;

        int y0 = y + inset;
        int y1 = y + h - inset;

        int x0 = x + w - border + col;

        int rx, ry, rw, rh;

        if (!clip_rect(
            fb,
            x0, y0, 1, y1 - y0,
            cx, cy, cw, ch,
            &rx, &ry, &rw, &rh
        )) {
            continue;
        }

        for (int i = 0; i < rh; i++) {
            fb_ptr[(size_t)(ry + i) * pitch + rx] = dark;
        }
    }
}

void draw_box(fb_info_t *fb, int x, int y, int w, int h, uint32_t color, int border, char fill) {
    draw_box_clip(
        fb,
        x, y, w, h,
        color, border, fill,
        0, 0,
        fb->width, fb->height
    );
}
