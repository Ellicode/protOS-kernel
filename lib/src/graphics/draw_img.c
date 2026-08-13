#include <proto/graphics.h>
#include <proto/core.h>

void draw_img_clip(
    fb_info_t *fb,
    const uint32_t *img,
    int x, int y, int w, int h,
    int cx, int cy, int cw, int ch
) {
    if (!fb || !fb->address || !img ||
        w <= 0 || h <= 0 ||
        cw <= 0 || ch <= 0) {
        return;
    }

    int rx, ry, rw, rh;

    if (!clip_rect(
        fb,
        x, y, w, h,
        cx, cy, cw, ch,
        &rx, &ry, &rw, &rh
    )) {
        return;
    }

    int src_x = rx - x;
    int src_y = ry - y;

    size_t fb_pitch = fb->pitch / sizeof(uint32_t);

    uint32_t *dst =
        (uint32_t *)fb->address + (size_t)ry * fb_pitch + rx;
    const uint32_t *src =
        img + (size_t)src_y * w + src_x;

    size_t pairs = (size_t)rw >> 1;
    int has_odd_tail = rw & 1;

    for (int row = 0; row < rh; row++) {
        uint64_t *dst64 = (uint64_t *)dst;
        const uint64_t *src64 = (const uint64_t *)src;

        size_t col = 0;
        for (; col + 1 < pairs; col += 2) {
            dst64[col]     = src64[col];
            dst64[col + 1] = src64[col + 1];
        }
        for (; col < pairs; col++) {
            dst64[col] = src64[col];
        }

        if (has_odd_tail) {
            dst[rw - 1] = src[rw - 1];
        }

        dst += fb_pitch;
        src += w;
    }
}

void draw_img(fb_info_t * fb, const uint32_t * img, int x, int y, int w, int h) {
    draw_img_clip(
        fb, img,
        x, y, w, h,
        0, 0,
        fb->width, fb->height
    );
}

void draw_img_a_clip(
    fb_info_t *fb,
    const uint32_t *img,
    int x, int y, int w, int h,
    int cx, int cy, int cw, int ch
) {
    if (!fb || !fb->address || !img ||
        w <= 0 || h <= 0 ||
        cw <= 0 || ch <= 0) {
        return;
    }

    int rx, ry, rw, rh;

    if (!clip_rect(
        fb,
        x, y, w, h,
        cx, cy, cw, ch,
        &rx, &ry, &rw, &rh
    )) {
        return;
    }

    int src_x = rx - x;
    int src_y = ry - y;

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    size_t pitch = fb->pitch / sizeof(uint32_t);

    for (int row = 0; row < rh; row++) {
        uint32_t *dst =
            fb_ptr + (size_t)(ry + row) * pitch + rx;

        const uint32_t *src =
            img + (size_t)(src_y + row) * w + src_x;

        for (int col = 0; col < rw; col++) {
            uint32_t pixel = src[col];

            uint32_t a = pixel >> 24;

            if (a == 255) {
                dst[col] = pixel & 0x00FFFFFF;
                continue;
            }

            if (a == 0)
                continue;

            uint32_t bg = dst[col];

            uint32_t r  = (pixel >> 16) & 0xFF;
            uint32_t g  = (pixel >> 8)  & 0xFF;
            uint32_t b  = pixel & 0xFF;

            uint32_t br = (bg >> 16) & 0xFF;
            uint32_t bgc = (bg >> 8) & 0xFF;
            uint32_t bb = bg & 0xFF;

            uint32_t inv_a = 255 - a;

            uint32_t out_r = (r * a + br * inv_a) / 255;
            uint32_t out_g = (g * a + bgc * inv_a) / 255;
            uint32_t out_b = (b * a + bb * inv_a) / 255;

            dst[col] =
                (out_r << 16) |
                (out_g << 8) |
                out_b;
        }
    }
}

void draw_img_a(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h) {
    draw_img_a_clip(
        fb, img,
        x, y, w, h,
        0, 0,
        fb->width, fb->height
    );
}

void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h) {
    if (!fb || !buf || !fb->address || w <= 0 || h <= 0 ||
        x >= fb->width || y >= fb->height || x < 0 || y < 0) { return; }

    if (x + w > fb->width) {
        w = (int)fb->width - x;
    }
    if (y + h > fb->height) {
        h = (int)fb->height - y;
    }

    const uint32_t *fb_ptr = (void*)fb->address;

    for (int row = y; row < y + h; row++) {
        uint32_t *dst = buf + (row-y) * w;
        const uint32_t *src = fb_ptr + row * fb->pitch / 4 + x;
        memcpy(dst, src, w * sizeof(uint32_t));
    }
}

void draw_notex(fb_info_t *fb, int x, int y, int w, int h) {
    if (!fb || !fb->address) { return; }

    int clip_x = x;
    int clip_y = y;
    int clip_w = w;
    int clip_h = h;

    if (clip_x < 0) {
        clip_w += clip_x;
        clip_x = 0;
    }
    if (clip_y < 0) {
        clip_h += clip_y;
        clip_y = 0;
    }
    if (clip_x + clip_w > (int)fb->width) {
        clip_w = (int)fb->width - clip_x;
    }
    if (clip_y + clip_h > (int)fb->height) {
        clip_h = (int)fb->height - clip_y;
    }

    if (clip_w <= 0 || clip_h <= 0) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t fb_pitch = fb->pitch / 4;

    for (int row = 0; row < clip_h; row++) {
        for (int col = 0; col < clip_w; col++) {
            uint32_t color = (((clip_y + row) / 16 + (clip_x + col) / 16) % 2) == 0 ? 0xFF00FF : 0x000000;
            fb_ptr[(clip_y + row) * fb_pitch + (clip_x + col)] = color;
        }
    }
}
