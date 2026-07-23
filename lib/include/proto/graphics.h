#ifndef PROTO_GRAPHICS_H
#define PROTO_GRAPHICS_H

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint64_t bpp;
} fb_info_t;

int fetch_framebuffer(fb_info_t *fb);

/*****************************************************************************
 * Basic VFX Operations
 *****************************************************************************/

 typedef struct bmp_t {
    int         width;
    int         height;
    uint32_t    *data;
} bmp_t;

void putpixel(fb_info_t *fb, int x, int y, uint32_t color);
void putpixel_a(fb_info_t *fb, int x, int y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
void draw_rect(fb_info_t *fb, int x, int y, int w, int h, uint32_t color);
void draw_img(fb_info_t *fb, const uint32_t *img, int x, int y, int w, int h);
void capture_rect(fb_info_t *fb, uint32_t *buf, int x, int y, int w, int h);

/*****************************************************************************
 * BMP file support
 *****************************************************************************/

bmp_t *bmp_load(const char *path);
void bmp_draw(fb_info_t *fb, bmp_t *bmp, int x, int y);
void bmp_free(bmp_t *bmp);

#endif // PROTO_GRAPHICS_H