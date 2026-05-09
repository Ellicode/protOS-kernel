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

extern struct limine_framebuffer *g_vga_active_framebuffer;

#endif // GLOBALS_H