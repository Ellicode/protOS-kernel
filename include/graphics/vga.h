#include "../limine/limine.h"

#ifndef VGA_H
#define VGA_H

typedef uint64_t color_t;

void graphics_init(struct limine_framebuffer *framebuffer);
void putpixel(uint32_t x, uint32_t y, color_t color);

#endif // VGA_H
