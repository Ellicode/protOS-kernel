/********************************************************************************
 * @file        gdt.c
 * @brief       Global Descriptor Table (GDT) setup utilites.
 * 
 * @author      Elliot Laborieux
 * @copyright   Copyright (c) 2026 Ellicode
 ********************************************************************************/

#include "graphics/console.h"
#include "debug/logger.h"
#include "utils/utils.h"

#include "gdt.h"

gdt_entry_t gdt[7];
gdtr_t gdtr;
tss_entry_t g_tss __attribute__((aligned(PAGE_SIZE))) = {0};

/**
 * Generates a GDT descriptor with the correct parameters
 * 
 * @param base      A 64-bit value containing the linear address where the segment begins.
 * @param limit     A 20-bit value, tells the maximum addressable unit, either in 1 byte units, or in 4KiB pages.
 * @param access    The access byte object of the descriptor
 * @param longa     Long-mode code flag. If set, the descriptor defines a 64-bit code segment
 * @param db        Size flag. If clear, the descriptor defines a 16-bit protected mode segment. If set, it defines a 32-bit protected mode segment.
 * @param gran      Granularity flag, indicates the size the Limit value is scaled by. If clear, the Limit is in 1 Byte blocks (byte granularity). If set, the Limit is in 4 KiB blocks (page granularity).
 * 
 * @return Formatted GDT entry integer.
 */
gdt_entry_t _gdt_generate_descriptor(
    uint64_t            base, 
    uint64_t            limit,
    GDTEntryAccessByte  access,
    bool                longa,
    bool                db,
    bool                gran
) {
    GDTEntry descriptor = (GDTEntry) { 0 };

    descriptor.base_low  = base & 0xFFFF;
    descriptor.base_mid = (base >> 16) & 0xFF;
    descriptor.base_high = (base >> 24) & 0xFF;

    descriptor.limit_low  = limit & 0xFFFF;
    descriptor.limit_high = (limit >> 16) & 0x0F;

    descriptor.access_byte = access.value;

    descriptor._reserved  = 0;
    descriptor.longa      = longa; // set 1 ONLY for 64-bit code segment
    descriptor.db         = db; // 0 in long mode, 1 in legacy
    descriptor.granuality = gran;


    k_debug("gdt_entry (");
    #if (PROTO_DEBUG == 1)
        print_f("%x): base=%x, limit=%x, ring=%d, exe=%d, r/w=%d\n", 
            descriptor.value, base, limit, access.dpl, access.executable, access.read_write);
    #endif

    return descriptor.value;
}

static void _tss_generate() {
    g_tss.iopb = sizeof(tss_entry_t);
}

void _gdt_post_init() {
    __asm__ volatile (
        "pushq %[cs]\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "pushq %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov %[ds], %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%ss\n\t"
        "xor %%ax, %%ax\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        :
        : [cs] "i"(GDT_OFFSET_KERNEL_CODE),
        [ds] "i"(GDT_OFFSET_KERNEL_DATA)
        : "rax", "memory"
    );

    __asm__ volatile ("ltr %0" :: "r"((uint16_t)GDT_OFFSET_TSS));
}

/**
 * Initializes the GDT with correct kernel/user code/data segments
 */
void gdt_init() {
    // Generate null descriptor
    gdt[0] = _gdt_generate_descriptor(0, 0, GDT_NULL_ENTRY, 0, 0, 0);

    // Generate kernel code segment
    gdt[1] = _gdt_generate_descriptor(
        GDT_ENTRY_BASE, 
        GDT_ENTRY_LIMIT, 
        GDT_R0_CODE, 
        1, 
        0, 
        GDT_ENTRY_GRANULARITY_PAGES
    );

    // Generate kernel data segment
    gdt[2] = _gdt_generate_descriptor(
        GDT_ENTRY_BASE, 
        GDT_ENTRY_LIMIT, 
        GDT_R0_DATA, 
        0, 
        1, 
        GDT_ENTRY_GRANULARITY_PAGES
    );

    // Generate user code segment
    gdt[3] = _gdt_generate_descriptor(
        GDT_ENTRY_BASE, 
        GDT_ENTRY_LIMIT, 
        GDT_R3_CODE, 
        1, 
        0, 
        GDT_ENTRY_GRANULARITY_PAGES
    );

    // Generate user data segment
    gdt[4] = _gdt_generate_descriptor(
        GDT_ENTRY_BASE, 
        GDT_ENTRY_LIMIT, 
        GDT_R3_DATA, 
        0, 
        1, 
        GDT_ENTRY_GRANULARITY_PAGES
    );


    // Generate TSS segment
    _tss_generate();

    intptr_t addr = (uintptr_t)&g_tss;
    uint64_t lower  = (uint64_t)addr & 0xFFFFFFFF;
    uint64_t higher = (uint64_t)addr >> 32;
    uint64_t limit = sizeof(g_tss) - 1;

    gdt[5] = _gdt_generate_descriptor(lower, limit, (GDTEntryAccessByte) {{
        GDT_ACCESSED,
        GDT_OFF,
        GDT_OFF,
        GDT_OFF,
        GDT_ENTRY_SEG_SYSTEM,
        GDT_ENTRY_DPL_KERNEL,
        GDT_ENTRY_PRESENT
    }}, 0, 0, 0);
    gdt[6] = higher;

    k_debug("gdt_entry (");
    #if (PROTO_DEBUG == 1)
        print_f("%x): higher half tss\n", higher);
    #endif

    // Generate GDTR to send to asm
    gdtr.base = (uint64_t)gdt;
    gdtr.limit = sizeof(gdt) - 1;

    k_debug("gdt_base: ");
    #if (PROTO_DEBUG == 1)
        print_f("%x, limit: %x\n", gdtr.base, gdtr.limit);
    #endif

    // Load GDT to the GDTR register
    __asm__ volatile (
        "lgdt %0"
        :
        : "m"(gdtr)
        : "memory"
    );

    _gdt_post_init();
}