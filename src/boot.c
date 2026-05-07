#include "debug/serial.h"
#include "graphics/vga.h"
#include "graphics/tty.h"
#include "gdt.h"
#include "debug/logger.h"

#include "boot.h"

int k_init(struct limine_framebuffer *fb) {
    // Triple line break to avoid overlapping issues with the QEMU logs :3
    print("\n\n\n");
    k_info("Starting boot sequence...\n", "proto.kernel.k_init");

    // 1) INITIALIZE SERIAL OUTPUT ===================================================
    if (serial_init() != 0) {
        k_error("Couldn't initialize serial output.\n", "proto.kernel.k_init");
        return 1;
    }
    k_success("Initialized serial output.\n", "proto.kernel.k_init");

    // 2) INITIALIZE GRAPHICS ========================================================
    graphics_init(fb);
    k_success("Initialized graphics.\n", "proto.kernel.k_init");

    // 3) INITIALIZE GDT =============================================================
    gdt_init();
    k_success("Initialized GDT.\n", "proto.kernel.k_init");

    // DONE! =========================================================================
    k_info("Boot script ended without errors.\n", "proto.kernel.k_init");
    return 0;
}