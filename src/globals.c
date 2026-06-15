#include "globals.h"

uint64_t g_tty_cursor_x                     = 0;
uint64_t g_tty_cursor_y                     = 0;
color_t g_tty_current_color       = PROTO_WHITE;

uint64_t g_pit_frequency                    = 0;
uint64_t g_pit_ticks                        = 0;

uint8_t g_kbd_enable                        = 1;

struct limine_framebuffer *g_vga_active_framebuffer;
struct limine_hhdm_response *g_lim_hhdm;
struct limine_memmap_response *g_lim_memmap;
struct limine_executable_address_response *g_lim_kaddr;
struct limine_module_response *g_lim_modules;

// METHODS 

void* (*m_pmm_alloc_p)(void);
void (*m_pmm_free_p)(void *);
