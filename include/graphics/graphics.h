#ifndef VGA_H
#define VGA_H

#include "../limine/limine.h"

typedef uint64_t color_t;

void graphics_init(struct limine_framebuffer *framebuffer);
void putpixel(uint32_t x, uint32_t y, color_t color);
void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color);
void fill_screen(color_t color);

#endif // VGA_H
