#include "debug/logger.h"
#include "debug/serial.h"
#include "graphics/vga.h"
#include "graphics/tty.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "drivers/ps2.h"
#include "pit.h"
#include "gdt.h"

#include "boot.h"

int k_init(struct limine_framebuffer *fb, struct limine_memmap_response *memmap) {
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

    // 4) INITIALIZE IDT =============================================================
    idt_init();
    k_success("Initialized IDT.\n", "proto.kernel.k_init");

    // 5) INITIALIZE PIC =============================================================
    pic_init();
    k_success("Initialized PIC.\n", "proto.kernel.k_init");

    // 6) INITIALIZE PIT =============================================================
    pit_init(100);
    k_success("Initialized PIT.\n", "proto.kernel.k_init");

    // 7) INITIALIZE PS2 KEYBOARD ====================================================
    ps2_init();
    k_success("Initialized PS2 Keyboard.\n", "proto.kernel.k_init");

    __asm__ volatile("sti");
    k_success("Interrupts enabled.\n", "proto.kernel.k_init");

    print_f("|------------------------------------------------|\n");
    print_f("|  Physical Base     | Region size        | Type |\n");
    print_f("|------------------------------------------------|\n");
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            set_color(PROTO_BLUE);
        } else if (entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            set_color(PROTO_CYAN);
        } else {
            set_color(PROTO_GREY);
        }
        print_f("| %18x | %18x | %4d |\n", entry->base, entry->length, entry->type);
        set_color(PROTO_WHITE);
    }
    print_f("|------------------------------------------------|\n");

    // DONE! =========================================================================
    k_info("Boot script ended without errors.\n", "proto.kernel.k_init");
    return 0;
}