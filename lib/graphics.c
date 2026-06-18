#include "proto.h"

int fetch_framebuffer(fb_info_t *fb) {
    return syscall(SYS_FETCH_FB, (uint64_t)fb, 0, 0);
}