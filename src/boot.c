#include "debug/logger.h"
#include "debug/serial.h"
#include "debug/errors.h"
#include "graphics/graphics.h"
#include "graphics/console.h"
#include "interrupts/idt.h"
#include "interrupts/pic.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/ps2/mouse.h"
#include "drivers/ps2/ps2.h"
#include "memory/freelist_pmm.h"
#include "memory/vmm.h"
#include "memory/heap.h"
#include "memory/pat.h"
#include "userspace/scheduler.h"
#include "userspace/syscalls.h"
#include "filesystems/vfs.h"
#include "pit.h"
#include "gdt.h"
#include "globals.h"

#include "boot.h"

int k_init(
    struct limine_framebuffer *fb, 
    struct limine_memmap_response *memmap,
    struct limine_hhdm_response *hhdm,
    struct limine_executable_address_response *kaddr,
    struct limine_module_response *modules
) {
    // Triple line break to avoid overlapping issues with the QEMU logs :3
    print("\n\n\n");
    k_info("Starting boot sequence...\n");

    g_lim_memmap = memmap;
    g_lim_hhdm = hhdm;
    g_lim_kaddr = kaddr;
    g_lim_modules = modules;

    if (serial_init() != PROTO_OK) {
        k_assert(PROTO_ERR_INIT_FAILED);
        return PROTO_ERR_INIT_FAILED;
    }
    k_success("Initialized serial output.\n");

    graphics_init(fb);
    k_success("Initialized graphics.\n");

    if (modules == NULL || modules->module_count == 0) {
        k_warning("No modules loaded!\n");
    } else {
        for (uint64_t i = 0; i < modules->module_count; i++) {
            k_info("Module: ");
            print_f("%s @ %x (%d bytes)\n",
                modules->modules[i]->path,
                modules->modules[i]->address,
                modules->modules[i]->size);
        }
    }

    gdt_init();
    k_success("Initialized GDT.\n");

    idt_init();
    k_success("Initialized IDT.\n");

    
    if (fpmm_init() != PROTO_OK) {
        k_assert(PROTO_ERR_INIT_FAILED);
        return PROTO_ERR_INIT_FAILED;
    }
    k_success("Initialized PMM.\n");
    
    pat_init();
    k_success("Initialized PAT.\n");
    
    vmm_init();
    k_success("Initialized Paging.\n");

    heap_init();
    k_success("Initialized Heap.\n");

    terminal_init();
    k_success("Initialized Terminal Output.\n");

    pic_init();
    k_success("Initialized PIC.\n");

    ps2_init();
    ps2keyboard_init();
    ps2mouse_init();
    k_success("Initialized PS2 Keyboard and Mice.\n");

    pit_init(100);
    k_success("Initialized PIT.\n");

    vfs_init();
    k_success("Initialized VFS.\n");

    scheduler_init();
    k_success("Initialized Scheduler\n");

    // // Reclaim bootloader reclaimable regions
    // uint64_t space_reclaimed = 0;

    // for (size_t i = 0; i < g_lim_memmap->entry_count; i++) {
    //     struct limine_memmap_entry *entry = g_lim_memmap->entries[i];

    //     if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
    //         for (uint64_t idx = 0; idx < PAGE_ROUND(entry->length) / PAGE_SIZE; idx++) {
    //             m_pmm_free_p((void *)(entry->base + idx * PAGE_SIZE));
    //         }

    //         space_reclaimed += entry->length;
    //     }
    // }

    // k_info("Reclaimed ");
    // print_f("%dMB from bootloader reclaimable memory\n", space_reclaimed / (uint64_t)(1024 * 1024));

    // DONE! =========================================================================

    k_info("Welcome to ");
    set_color(PROTO_BG, PROTO_BLUE);
    print_f("ProtOS");
    set_color(PROTO_WHITE, PROTO_BG);
    print_f("! System will halt...\n");

    return PROTO_OK;
}