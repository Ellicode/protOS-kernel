#ifndef GRAPHICS_H
#define GRAPHICS_H

#include <proto.h>

typedef struct bmp_t {
    int         width;
    int         height;
    uint32_t    *data;
} bmp_t;

void putpixel(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t color);
void draw_rect(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t color);

bmp_t *load_bitmap(const char *path);
void draw_bitmap(fb_info_t *fb, bmp_t *bmp, int x, int y);
void free_bitmap(bmp_t *bmp);

#endif // GRAPHICS_H