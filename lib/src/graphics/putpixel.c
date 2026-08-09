#include <proto/graphics.h>
#include <stddef.h>

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