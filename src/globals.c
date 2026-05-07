#include "globals.h"

uint64_t g_tty_cursor_x = 0;
uint64_t g_tty_cursor_y = 0;
color_t g_tty_current_color = PROTO_WHITE;

struct limine_framebuffer *g_vga_active_framebuffer;
