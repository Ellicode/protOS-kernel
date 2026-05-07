#include <stddef.h>

#include "globals.h"

#include "graphics/vga.h"

void graphics_init(struct limine_framebuffer *framebuffer)
{
    g_vga_active_framebuffer = framebuffer;
}

void putpixel(uint32_t x, uint32_t y, color_t color)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }
    if (x < 0 || y < 0 ||
        x >= g_vga_active_framebuffer->width ||
        y >= g_vga_active_framebuffer->height)
    {
        return;
    }
    volatile uint32_t *fb_ptr = g_vga_active_framebuffer->address;
    fb_ptr[y * (g_vga_active_framebuffer->pitch / 4) + x] = color;
}

