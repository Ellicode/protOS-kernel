#include "debug/logger.h"
#include "debug/serial.h"
#include "graphics/vga.h"
#include "graphics/console.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "drivers/ps2.h"
#include "memory/memory.h"
#include "memory/freelist_pmm.h"
#include "memory/pmm.h"
#include "memory/vmm.h"
#include "memory/heap.h"
#include "userspace/scheduler.h"
#include "filesystems/vfs.h"
#include "pit.h"
#include "gdt.h"
#include "globals.h"

#include "boot.h"

int k_init(
    struct limine_framebuffer *fb, 
    struct limine_memmap_response *memmap,
    struct limine_hhdm_response *hhdm,
    struct limine_executable_address_response *kaddr
) {
    // Triple line break to avoid overlapping issues with the QEMU logs :3
    print("\n\n\n");
    k_info("Starting boot sequence...\n", "proto.kernel.k_init");
    
    g_lim_memmap = memmap;
    g_lim_hhdm = hhdm;
    g_lim_kaddr = kaddr;

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

    // 5) INITIALIZE PAGING ==========================================================
    if (fpmm_init() == 1) {
        k_error("Couldn't initialize PMM.\n", "proto.kernel.k_init");
        return 1;
    }
    k_success("Initialized PMM.\n", "proto.kernel.k_init");
    
    vmm_init();
    k_success("Initialized Paging.\n", "proto.kernel.k_init");

    heap_init();
    k_success("Initialized Heap.\n", "proto.kernel.k_init");

    // 6) INITIALIZE PIC =============================================================
    pic_init();
    k_success("Initialized PIC.\n", "proto.kernel.k_init");

    // 7) INITIALIZE PIT =============================================================
    pit_init(100);
    k_success("Initialized PIT.\n", "proto.kernel.k_init");

    // 8) INITIALIZE PS2 KEYBOARD ====================================================
    ps2_init();
    k_success("Initialized PS2 Keyboard.\n", "proto.kernel.k_init");

    scheduler_init();
    k_success("Initialized Scheduler\n", "proto.kernel.k_init");

    enable_interrupts();
    k_success("Interrupts enabled.\n", "proto.kernel.k_init");

    heap_dump();

    vfs_init();

    // DONE! =========================================================================
    k_info("Boot sequence ended.\n", "proto.kernel.k_init");
    return 0;
}