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

void draw_img(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h) {
    if (img == NULL) { 
        draw_notex(fb, x, y, w, h);
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
    if (fb == NULL || buf == NULL) { return; }

    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;

    int i = 0;
    for (uint32_t row = y; row < (y + h); row++) {
        for (uint32_t col = x; col < (x + w); col++) {
            if (row < 0 || col < 0) { continue; }
            if ((uint32_t)row >= fb->height || (uint32_t)col >= fb->width) { continue; }

            buf[i] = fb_ptr[row * fb->width + col];
            // buf[i] = (buf[i] & 0x00FFFFFF) | (0xFF << 24);
            i++;
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