#include "globals.h"
#include "memory/heap.h"
#include "graphics/console.h"
#include "userspace/process.h"
#include "memory/vmm.h"
#include "utils/utils.h"
#include "memory/pat.h"
#include "debug/errors.h"

#include "userspace/user_fb.h"

uint64_t sys_fetch_fb(fb_info_t *info) {
    if (g_current_thread->process == NULL) {
        k_assert(PROTO_ERR_INVALID_CONTEXT);
        return PROTO_ERR_INVALID_CONTEXT;
    }

    uint64_t physical_fb = (uint64_t)g_vga_active_framebuffer->address - g_lim_hhdm->offset;
    size_t fb_size = PAGE_ROUND(g_vga_active_framebuffer->height * g_vga_active_framebuffer->pitch);
    
    // map framebuffer
    vmm_map_phys_range(
        g_current_thread->process->cr3, 
        USER_FRAMEBUFFER_BASE, 
        physical_fb, 
        fb_size, 
        F_USER | F_WRITE | F_CACHE_WC
    );

    info->address = USER_FRAMEBUFFER_BASE;
    info->width   = g_vga_active_framebuffer->width;
    info->height  = g_vga_active_framebuffer->height;
    info->pitch   = g_vga_active_framebuffer->pitch;
    info->bpp     = g_vga_active_framebuffer->bpp;

    return PROTO_OK;
}