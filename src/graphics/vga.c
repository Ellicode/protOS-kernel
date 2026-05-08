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

void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color)
{
    if (g_vga_active_framebuffer == NULL)
    {
        return;
    }

    uint32_t *fb_ptr = g_vga_active_framebuffer->address;
    
    // Pitch is in bytes, so divide by 4 to use with a uint32_t pointer
    int fb_pitch = g_vga_active_framebuffer->pitch / 4;

    for (int row = y; row < y + h; row++) {
        for (int col = x; col < x + w; col++) {
            fb_ptr[row * fb_pitch + col] = color;
        }
    }
}