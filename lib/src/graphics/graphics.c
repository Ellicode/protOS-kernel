#include <proto/core.h>
#include <proto/graphics.h>

static inline uint32_t *fb_pixel_ptr(fb_info_t *fb, int x, int y) {
    return ((uint32_t *)fb->address) + (uint32_t)y * (fb->pitch / 4) + (uint32_t)x;
}

static inline uint32_t *fb_row_ptr(fb_info_t *fb, int y) {
    return ((uint32_t *)fb->address) + (uint32_t)y * (fb->pitch / 4);
}

int fetch_framebuffer(fb_info_t *fb) {
    return syscall(SYS_FETCH_FB, (uint64_t)fb, 0, 0);
}

// VFX

void putpixel(fb_info_t *fb, int x, int y, uint32_t color) {
    if (fb == NULL) { return; }
    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    *fb_pixel_ptr(fb, x, y) = color;
}

void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (fb == NULL) { return; }
    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    volatile uint32_t *fb_ptr = fb_pixel_ptr(fb, x, y);
    if (a == 255) {
        *fb_ptr = (r << 16) | (g << 8) | b;
        return;
    }
    if (a == 0) { return; }

    uint32_t bg_pixel = *fb_ptr;
    uint8_t bg_r = (bg_pixel >> 16) & 0xFF;
    uint8_t bg_g = (bg_pixel >> 8) & 0xFF;
    uint8_t bg_b = bg_pixel & 0xFF;

    uint8_t out_r = ((r * a) + (bg_r * (255 - a))) / 255;
    uint8_t out_g = ((g * a) + (bg_g * (255 - a))) / 255;
    uint8_t out_b = ((b * a) + (bg_b * (255 - a))) / 255;

    *fb_ptr = (out_r << 16) | (out_g << 8) | out_b;
}

void invertpixel(fb_info_t *fb, int x, int y) {
    if (!fb || !fb->address) { return; }
    if (x < 0 || y < 0 || x >= (int)fb->width || y >= (int)fb->height) { return; }

    volatile uint32_t *fb_ptr = fb_pixel_ptr(fb, x, y);
    uint32_t p = *fb_ptr;
    *fb_ptr = p ^ 0x00FFFFFF;
}

void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    if (!fb || !fb->address || w <= 0 || h <= 0) { return; }

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

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t fb_pitch = fb->pitch / 4;

    for (int row = 0; row < clip_h; row++) {
        uint32_t *row_ptr = fb_ptr + (clip_y + row) * fb_pitch + clip_x;
        for (int col = 0; col < clip_w; col++) {
            row_ptr[col] = color;
        }
    }
}

void draw_rect_o(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    if (!fb || !fb->address || w <= 0 || h <= 0) { return; }

    int right = x + w - 1;
    int bottom = y + h - 1;

    if (x >= 0 && x < (int)fb->width) {
        uint32_t *row_ptr = fb_row_ptr(fb, y);
        for (int i = x; i <= right; i++) {
            if (i >= 0 && i < (int)fb->width) { row_ptr[i] = color; }
        }
    }
    if (bottom >= 0 && bottom < (int)fb->height) {
        uint32_t *row_ptr = fb_row_ptr(fb, bottom);
        for (int i = x; i <= right; i++) {
            if (i >= 0 && i < (int)fb->width) { row_ptr[i] = color; }
        }
    }

    if (y >= 0 && y < (int)fb->height) {
        for (int i = y; i <= bottom; i++) {
            if (i >= 0 && i < (int)fb->height) {
                uint32_t *pixel = fb_pixel_ptr(fb, x, i);
                if (x >= 0 && x < (int)fb->width) { *pixel = color; }
            }
        }
    }
    if (right >= 0 && right < (int)fb->width) {
        for (int i = y; i <= bottom; i++) {
            if (i >= 0 && i < (int)fb->height) {
                uint32_t *pixel = fb_pixel_ptr(fb, right, i);
                if (right >= 0 && right < (int)fb->width) { *pixel = color; }
            }
        }
    }
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

void draw_box(fb_info_t *fb, int x, int y, int w, int h, uint32_t color, int border) {
    if (!fb || !fb->address || w <= 0 || h <= 0) { return; }

    draw_rect(fb, x, y, w, h, color);

    if (border <= 0) { return; }
    if (border > w / 2) { border = w / 2; }
    if (border > h / 2) { border = h / 2; }
    if (border <= 0) { return; }

    volatile uint32_t *fb_ptr = (volatile uint32_t *)fb->address;
    const uint32_t pitch = fb->pitch >> 2; // /4

    const uint32_t light = lighten(color, FX_ONE / 4);
    const uint32_t dark  = darken(color, FX_ONE / 4);

    // Bottom bevel (dark)
    for (int row = 0; row < border; row++) {
        int inset = (border - 1) - row;
        int x0 = x + inset;
        int x1 = x + w - inset; // exclusive
        volatile uint32_t *p = fb_ptr + (y + h - border + row) * pitch + x0;
        for (int xx = x0; xx < x1; xx++) { *p++ = dark; }
    }

    // Top bevel (light)
    for (int row = 0; row < border; row++) {
        int x0 = x + row;
        int x1 = x + w - row; // exclusive
        volatile uint32_t *p = fb_ptr + (y + row) * pitch + x0;
        for (int xx = x0; xx < x1; xx++) { *p++ = light; }
    }

    // Left bevel (light)
    for (int col = 0; col < border; col++) {
        int y0 = y + col;
        int y1 = y + h - col; // exclusive
        int xx = x + col;
        for (int yy = y0; yy < y1; yy++) {
            fb_ptr[yy * pitch + xx] = light;
        }
    }

    // Right bevel (dark)
    for (int col = 0; col < border; col++) {
        int inset = (border - 1) - col;
        int y0 = y + inset;
        int y1 = y + h - inset; // exclusive
        int xx = x + w - border + col;
        for (int yy = y0; yy < y1; yy++) {
            fb_ptr[yy * pitch + xx] = dark;
        }
    }
}

void draw_img(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h) {
    if (!fb || !fb->address || !img) {
        draw_notex(fb, x, y, w, h);
        return;
    }

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

    int src_x = (x < 0) ? -x : 0;
    int src_y = (y < 0) ? -y : 0;
    uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t fb_pitch = fb->pitch / 4;

    for (int row = 0; row < clip_h; row++) {
        uint32_t *dst = fb_ptr + (clip_y + row) * fb_pitch + clip_x;
        const uint32_t *src = img + (src_y + row) * w + src_x;
        for (int col = 0; col < clip_w; col++) {
            dst[col] = src[col];
        }
    }
}

void draw_img_a(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h) {
    if (!fb || !fb->address || !img) {
        draw_notex(fb, x, y, w, h);
        return;
    }

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

    int src_x = (x < 0) ? -x : 0;
    int src_y = (y < 0) ? -y : 0;
    uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t fb_pitch = fb->pitch / 4;

    for (int row = 0; row < clip_h; row++) {
        uint32_t *dst = fb_ptr + (clip_y + row) * fb_pitch + clip_x;
        const uint32_t *src = img + (src_y + row) * w + src_x;
        for (int col = 0; col < clip_w; col++) {
            uint32_t pixel = src[col];
            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            if (a == 255) {
                dst[col] = (r << 16) | (g << 8) | b;
            } else if (a != 0) {
                uint32_t bg_pixel = dst[col];
                uint8_t bg_r = (bg_pixel >> 16) & 0xFF;
                uint8_t bg_g = (bg_pixel >> 8) & 0xFF;
                uint8_t bg_b = bg_pixel & 0xFF;

                uint8_t out_r = ((r * a) + (bg_r * (255 - a))) / 255;
                uint8_t out_g = ((g * a) + (bg_g * (255 - a))) / 255;
                uint8_t out_b = ((b * a) + (bg_b * (255 - a))) / 255;

                dst[col] = (out_r << 16) | (out_g << 8) | out_b;
            }
        }
    }
}

void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h) {
    if (!fb || !buf || !fb->address || w <= 0 || h <= 0) { return; }

    volatile uint32_t *fb_ptr = (volatile uint32_t *)fb->address;
    uint32_t pitch = fb->pitch / 4;

    for (int row = 0; row < h; row++) {
        int sy = y + row;
        if (sy < 0 || sy >= (int)fb->height) {
            continue;
        }

        uint32_t *dst = buf + (size_t)row * (size_t)w;
        for (int col = 0; col < w; col++) {
            int sx = x + col;
            if (sx >= 0 && sx < (int)fb->width) {
                dst[col] = fb_ptr[(uint32_t)sy * pitch + (uint32_t)sx];
            } else {
                dst[col] = 0;
            }
        }
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