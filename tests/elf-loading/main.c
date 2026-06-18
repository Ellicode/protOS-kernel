#include <stdint.h>
#include <stdlib.h>

#include "../../lib/proto.h"

void draw_rect(fb_info_t *fb, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint64_t color)
{
    if (fb == NULL)
    {
        return;
    }

    uint32_t *fb_ptr = (uint32_t *)fb->address;
    
    // Pitch is in bytes, so divide by 4 to use with a uint32_t pointer
    uint32_t fb_pitch = fb->pitch / 4;

    for (uint32_t row = y; row < y + h; row++) {
        for (uint32_t col = x; col < x + w; col++) {
            fb_ptr[row * fb_pitch + col] = color;
        }
    }
}

void putpixel(fb_info_t *fb, uint32_t x, uint32_t y, uint64_t color)
{
    if (fb == NULL)
    {
        return;
    }
    if (x >= fb->width ||
        y >= fb->height)
    {
        return;
    }
    volatile uint32_t *fb_ptr = (uint32_t *)fb->address;
    fb_ptr[y * (fb->pitch / 4) + x] = color;
}


int _start()
{
    fb_info_t fb;
    int res = fetch_framebuffer(&fb);

    int res2 = write(1, 0, "Triggered before read() \n");
    if (res2 == -1) {
        draw_rect(&fb, 0, 0, 50, 50, 0xff0000);
    }
    if (res2 == 1) {
        draw_rect(&fb, 0, 0, 50, 50, 0x00ff00);
    }


    char buffer[100] = "meow meow mroww-- :3";
    read(0, 0, buffer);

    write(1, 0, "Triggered after read() \n");
    write(1, 0, buffer);

    // if (res == 0) {
    //     draw_rect(&fb, 0, 0, 100, 100, 0xff0000);
    // }

    while (1)
    {
        
    }
}