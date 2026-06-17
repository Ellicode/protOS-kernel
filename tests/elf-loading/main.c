#include <stdint.h>
#include <stdlib.h>

typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint64_t bpp;
} fb_info_t;

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t ret;
    
    __asm__ __volatile__(
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return ret;
}

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
    uint64_t res = syscall(6, (uint64_t)&fb, 0, 0);

    if (res == 0) {
        draw_rect(&fb, 0, 0, 100, 100, 0xff0000);
    }

    while (1)
    {
        
    }
}