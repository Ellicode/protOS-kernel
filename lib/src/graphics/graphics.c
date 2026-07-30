#include <proto/core.h>
#include <proto/graphics.h>

int fetch_framebuffer(fb_info_t *fb) {
    return syscall(SYS_FETCH_FB, (uint64_t)fb, 0, 0);
}

// VFX 

void putpixel(fb_info_t *fb, int x, int y, uint32_t color) {
    if (fb == NULL) { return; }

    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    fb_ptr[y * (fb->pitch / 4) + x] = color;
}


void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
    if (fb == NULL) { return; }

    if (x < 0 || y < 0) { return; }
    if (x >= fb->width || y >= fb->height) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t idx = (uint32_t)y * (fb->pitch / 4) + (uint32_t)x;

    if (a == 255) {
        fb_ptr[idx] = (r << 16) | (g << 8) | b;
        return;
    }
    if (a == 0) { return; }

    uint32_t bg_pixel = fb_ptr[idx];
    uint8_t bg_r = (bg_pixel >> 16) & 0xFF;
    uint8_t bg_g = (bg_pixel >> 8) & 0xFF;
    uint8_t bg_b = bg_pixel & 0xFF;

    uint8_t out_r = ((r * a) + (bg_r * (255 - a))) / 255;
    uint8_t out_g = ((g * a) + (bg_g * (255 - a))) / 255;
    uint8_t out_b = ((b * a) + (bg_b * (255 - a))) / 255;

    fb_ptr[idx] = (out_r << 16) | (out_g << 8) | out_b;
}

void invertpixel(fb_info_t *fb, int x, int y) {
    if (!fb || !fb->address) { return; }
    if (x < 0 || y < 0 || x >= (int)fb->width || y >= (int)fb->height) { return; }

    volatile uint32_t *fb_ptr = (volatile uint32_t *)fb->address;
    uint32_t idx = (uint32_t)y * (fb->pitch >> 2) + (uint32_t)x;

    uint32_t p = fb_ptr[idx];          
    fb_ptr[idx] = p ^ 0x00FFFFFF;
}
void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    if (fb == NULL) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    
    // Pitch is in bytes, so divide by 4 to use with a uint32_t pointer
    uint32_t fb_pitch = fb->pitch / 4;

    for (uint32_t row = y; row < y + h; row++) {
        for (uint32_t col = x; col < x + w; col++) {
            if (row < 0 || col < 0) { continue; }
            if ((uint32_t)row >= fb->height || (uint32_t)col >= fb->width) { continue; }
            fb_ptr[row * fb_pitch + col] = color;
        }
    }
}

void draw_rect_o(fb_info_t *fb, int x, int y, int w, int h, uint32_t color) {
    for (int i = x; i < x + w; i++) {
        putpixel(fb, i, y, color);              // Top edge
        putpixel(fb, i, y + h - 1, color);  // Bottom edge
    }
    // Draw left and right vertical lines
    for (int i = y; i < y + h; i++) {
        putpixel(fb, x, i, color);              // Left edge
        putpixel(fb, x + w - 1, i, color);  // Right edge
    }
}

void invert_rect_o(fb_info_t *fb, int x, int y, int w, int h) {
    for (int i = x; i < x + w; i++) {
        invertpixel(fb, i, y);              // Top edge
        invertpixel(fb, i, y + h - 1);  // Bottom edge
    }
    // Draw left and right vertical lines
    for (int i = y; i < y + h; i++) {
        invertpixel(fb, x, i);              // Left edge
        invertpixel(fb, x + w - 1, i);  // Right edge
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
    if (img == NULL) { 
        draw_notex(fb, x, y, w, h);
        return;
    }

    for (uint32_t row = 0; row < h; row++) {
        for (uint32_t col = 0; col < w; col++) {
            if (x + (int)col >= fb->width ||  y + (int)row >= fb->height) { continue; }
            putpixel(fb, x + (int)col, y + (int)row, img[row * w + col]);
        }
    }
}

void draw_img_a(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h) {
    if (img == NULL) { 
        draw_notex(fb, x, y, w, h);
    }

    for (uint32_t row = 0; row < h; row++) {
        for (uint32_t col = 0; col < w; col++) {
            uint32_t pixel = img[row * w + col];

            uint8_t a = (pixel >> 24) & 0xFF;
            uint8_t r = (pixel >> 16) & 0xFF;
            uint8_t g = (pixel >> 8) & 0xFF;
            uint8_t b = pixel & 0xFF;

            if (x + (int)col >= fb->width ||  y + (int)row >= fb->height) { continue; }
            putpixel_a(fb, x + (int)col, y + (int)row, r, g, b, a);
        }
    }
}

void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h) {
    if (!fb || !buf || !fb->address || w <= 0 || h <= 0) { return; }

    volatile uint32_t *fb_ptr = (volatile uint32_t *)fb->address;
    uint32_t pitch = fb->pitch / 4;

    for (int row = 0; row < h; row++) {
        int sy = y + row;
        for (int col = 0; col < w; col++) {
            int sx = x + col;

            uint32_t out = 0;
            if (sx >= 0 && sy >= 0 && sx < (int)fb->width && sy < (int)fb->height) {
                out = fb_ptr[(uint32_t)sy * pitch + (uint32_t)sx];
            }
            buf[(size_t)row * (size_t)w + (size_t)col] = out;
        }
    }
}

void draw_notex(fb_info_t *fb, int x, int y, int w, int h) {
    if (!fb) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    uint32_t fb_pitch = fb->pitch / 4;

    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            if (row < 0 || col < 0) { continue; }
            if ((uint32_t)row >= fb->height || (uint32_t)col >= fb->width) { continue; }

            uint32_t color = (((row / 16 + col / 16) % 2) == 0) ? 0xFF00FF : 0x000000;
            fb_ptr[(uint32_t)row * fb_pitch + (uint32_t)col] = color;
        }
    }
}