#include <stdint.h>

#include "graphics/tty.h"
#include "graphics/vga.h"

#ifndef GLOBALS_H
#define GLOBALS_H

extern uint64_t g_tty_cursor_x;
extern uint64_t g_tty_cursor_y;
extern color_t g_tty_current_color;

extern uint64_t g_pit_frequency;
extern uint64_t g_pit_ticks;

extern uint8_t g_kbd_enable;

extern struct limine_framebuffer *g_vga_active_framebuffer;
extern struct limine_hhdm_response *g_lim_hhdm;
extern struct limine_memmap_response *g_lim_memmap;
extern struct limine_executable_address_response *g_lim_kaddr;

extern char g_kernel_start;
extern char g_kernel_end;

extern void* (*m_pmm_alloc_p)(void);
extern void (*m_pmm_free_p)(void *);

#endif // GLOBALS_H