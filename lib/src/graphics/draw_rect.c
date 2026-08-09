#include <proto/graphics.h>
#include <stddef.h>

void draw_rect_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int cx, int cy, int cw, int ch
) {
    int rx, ry, rw, rh;

    if (!clip_rect(
        fb, x, y, w, h,
        cx, cy, cw, ch,
        &rx, &ry, &rw, &rh
    )) {
        return;
    }

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    size_t pitch = fb->pitch / sizeof(uint32_t);

    for (int row = 0; row < rh; row++) {
        uint32_t *dst =
            fb_ptr + (size_t)(ry + row) * pitch + rx;

        for (int col = 0; col < rw; col++) {
            dst[col] = color;
        }
    }
}

void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    draw_rect_clip(
        fb,
        x, y, w, h,
        color,
        0, 0,
        fb->width, fb->height
    );
}


void draw_rect_c_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color1,
    uint32_t color2,
    int cx, int cy, int cw, int ch
) {
    int rx, ry, rw, rh;

    if (!clip_rect(
        fb, x, y, w, h,
        cx, cy, cw, ch,
        &rx, &ry, &rw, &rh
    )) {
        return;
    }

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    size_t pitch = fb->pitch / sizeof(uint32_t);

    for (int row = 0; row < rh; row++) {
        int sy = ry + row;
        uint32_t *dst =
            fb_ptr + (size_t)sy * pitch + rx;

        for (int col = 0; col < rw; col++) {
            int sx = rx + col;

            uint32_t color =
                ((sx + sy) & 1) ? color2 : color1;

            if (color != 0x00000000)
                dst[col] = color;
        }
    }
}

void draw_rect_c(fb_info_t *fb, int x, int y, int w, int h, uint32_t color1, uint32_t color2) {
    draw_rect_c_clip(
        fb,
        x, y, w, h,
        color1,
        color2,
        0, 0,
        fb->width, fb->height
    );
}

void draw_rect_o_clip(
    fb_info_t *fb,
    int x, int y, int w, int h,
    uint32_t color,
    int cx, int cy, int cw, int ch
) {
    if (!fb || !fb->address || w <= 0 || h <= 0)
        return;

    // Top
    draw_rect_clip(
        fb,
        x, y,
        w, 1,
        color,
        cx, cy, cw, ch
    );

    // Bottom
    draw_rect_clip(
        fb,
        x, y + h - 1,
        w, 1,
        color,
        cx, cy, cw, ch
    );

    // Left
    draw_rect_clip(
        fb,
        x, y,
        1, h,
        color,
        cx, cy, cw, ch
    );

    // Right
    draw_rect_clip(
        fb,
        x + w - 1, y,
        1, h,
        color,
        cx, cy, cw, ch
    );
}

void draw_rect_o(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    draw_rect_o_clip(
        fb,
        x, y, w, h,
        color,
        0, 0,
        fb->width, fb->height
    );
}

void invert_rect_o(fb_info_t *fb, int x, int y, int w, int h) {
    if (!fb || !fb->address || w <= 0 || h <= 0) { return; }

    int right = x + w - 1;
    int bottom = y + h - 1;

    if (x >= 0 && x < (int)fb->width) {
        for (int i = y; i <= bottom; i++) {
            if (i >= 0 && i < (int)fb->height) {
                uint32_t *pixel = fb_pixel_ptr(fb, x, i);
                if (x >= 0 && x < (int)fb->width) { *pixel ^= 0x00FFFFFF; }
            }
        }
    }
    if (right >= 0 && right < (int)fb->width) {
        for (int i = y; i <= bottom; i++) {
            if (i >= 0 && i < (int)fb->height) {
                uint32_t *pixel = fb_pixel_ptr(fb, right, i);
                if (right >= 0 && right < (int)fb->width) { *pixel ^= 0x00FFFFFF; }
            }
        }
    }
    if (y >= 0 && y < (int)fb->height) {
        for (int i = x; i <= right; i++) {
            if (i >= 0 && i < (int)fb->width) {
                uint32_t *pixel = fb_pixel_ptr(fb, i, y);
                if (y >= 0 && y < (int)fb->height) { *pixel ^= 0x00FFFFFF; }
            }
        }
    }
    if (bottom >= 0 && bottom < (int)fb->height) {
        for (int i = x; i <= right; i++) {
            if (i >= 0 && i < (int)fb->width) {
                uint32_t *pixel = fb_pixel_ptr(fb, i, bottom);
                if (bottom >= 0 && bottom < (int)fb->height) { *pixel ^= 0x00FFFFFF; }
            }
        }
    }
}
