#include <stdint.h>

#ifndef USER_FB_H
#define USER_FB_H

typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint64_t bpp;
} fb_info_t;

uint64_t sys_fetch_fb(fb_info_t *info);

#endif  // USER_FB_H