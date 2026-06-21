#include "globals.h"
#include "memory/heap.h"
#include "graphics/console.h"
#include "userspace/process.h"
#include "debug/errors.h"

#include "userspace/user_fb.h"

uint64_t sys_fetch_fb(fb_info_t *info) {    
    info->address = USER_FRAMEBUFFER_BASE;
    info->width   = g_vga_active_framebuffer->width;
    info->height  = g_vga_active_framebuffer->height;
    info->pitch   = g_vga_active_framebuffer->pitch;
    info->bpp     = g_vga_active_framebuffer->bpp;

    return PROTO_OK;
}