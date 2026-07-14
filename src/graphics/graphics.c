#include <stddef.h>

#include "globals.h"
#include "debug/logger.h"
#include "memory/heap.h"
#include "glyphs/kpanic.h"
#include "string.h"

#include "graphics/graphics.h"

void graphics_init(struct limine_framebuffer *framebuffer)
{
    g_vga_active_framebuffer = framebuffer;
    fill_screen(PROTO_BG);
}

void putpixel(uint32_t x, uint32_t y, color_t color)
{
    if (g_vga_active_framebuffer == NULL) { return; }
    if (x >= g_vga_active_framebuffer->width || y >= g_vga_active_framebuffer->height) { return; }
    volatile uint32_t *fb_ptr = g_vga_active_framebuffer->address;
    fb_ptr[y * (g_vga_active_framebuffer->pitch / 4) + x] = color;
}

void draw_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, color_t color)
{
    if (g_vga_active_framebuffer == NULL) { return; }

    uint32_t *fb_ptr = g_vga_active_framebuffer->address;
    
    // Pitch is in bytes, so divide by 4 to use with a uint32_t pointer
    uint32_t fb_pitch = g_vga_active_framebuffer->pitch / 4;

    for (uint32_t row = y; row < y + h; row++) {
        for (uint32_t col = x; col < x + w; col++) {
            fb_ptr[row * fb_pitch + col] = color;
        }
    }
}

void fill_screen(color_t color) {
    if (g_vga_active_framebuffer == NULL) { return; }

    draw_rect(0, 0, g_vga_active_framebuffer->width, g_vga_active_framebuffer->height, color);
}

void draw_glyph(glyph_t glyph, int pos_x, int pos_y) {
    int g_width  = 0;
    int g_height = 0;
    int g_bpp    = 0;
    const char *g_data = NULL;

    switch (glyph)
    {
        case GLYPH_KPANIC:
            g_width  = kpanic_img.width;
            g_height = kpanic_img.height;
            g_bpp    = kpanic_img.bytes_per_pixel;
            g_data   = (const char*)kpanic_img.pixel_data;
            break;

        default:
            return;
    }

    int i = 0;

    for (uint32_t y = 0; y < g_height; y++) {
        uint32_t screen_y = pos_y + y;
        if (screen_y >= g_vga_active_framebuffer->height) { break; }

        for (uint32_t x = 0; x < g_width; x++) {
            uint32_t screen_x = pos_x + x;
            if (screen_x >= g_vga_active_framebuffer->width) { break; }

            uint8_t r = g_data[i + 0];
            uint8_t g = g_data[i + 1];
            uint8_t b = g_data[i + 2];

            uint32_t pixel_color = (b << 16) | (g << 8) | r;

            if (pixel_color != 0x0) {
                putpixel(screen_x, screen_y, pixel_color);
            }
            
            i += g_bpp;
        }
    } 
}