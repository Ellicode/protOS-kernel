#include "graphics/tty.h"
#include "debug/logger.h"

#include "gdt.h"

gdt_entry_t gdt[7];
gdtr_t gdtr;
tss_entry_t tss __attribute__((aligned(0x1000))) = {0};

static gdt_entry_t _gdt_generate_descriptor(
    uint64_t base, 
    uint64_t limit,
    GDTEntryAccessByte access,
    uint8_t longa,
    uint8_t db,
    uint8_t gran
) {
    GDTEntry descriptor = (GDTEntry) { 0 };

    descriptor.base_low  = base & 0xFFFF;
    descriptor.base_mid  = (base >> 16) & 0xFF;
    descriptor.base_high = (base >> 24) & 0xFF;

    descriptor.limit_low  = limit & 0xFFFF;
    descriptor.limit_high = (limit >> 16) & 0x0F;

    descriptor.access_byte = access.value;

    descriptor._reserved  = 0;
    descriptor.longa      = longa; // set 1 ONLY for 64-bit code segment
    descriptor.db         = db; // 0 in long mode, 1 in legacy
    descriptor.granuality = gran;


    k_debug("gdt_entry (", "proto.kernel.gdt_init");
    print_f("%x): base=%x, limit=%x, ring=%d, exe=%d, r/w=%d\n", descriptor.value, base, limit, access.dpl, access.executable, access.read_write);

    return descriptor.value;
}

static void _tss_generate() {
    tss.iopb = sizeof(TSSEntry);
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

void gdt_init() {
    // Generate null descriptor
    gdt[0] = _gdt_generate_descriptor(0, 0, GDT_NULL_ENTRY, 0, 0, 0);

    // Generate kernel code segment
    gdt[1] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R0_CODE, 1, 0, 1);

    // Generate kernel data segment
    gdt[2] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R0_DATA, 0, 1, 1);

    // Generate user code segment
    gdt[3] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R3_CODE, 1, 0, 1);

    // Generate user data segment
    gdt[4] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R3_DATA, 0, 1, 1);

    // Generate TSS segment
    _tss_generate();

    intptr_t addr = (intptr_t)&tss;
    uint64_t lower  = (uint64_t)addr & 0xFFFFFFFF;
    uint64_t higher = (uint64_t)addr >> 32;
    uint64_t limit = sizeof(tss) - 1;

    gdt[5] = _gdt_generate_descriptor(lower, limit, (GDTEntryAccessByte) {{
        GDT_ACCESSED,
        GDT_OFF,
        GDT_OFF,
        GDT_ENTRY_EXECUTABLE,
        GDT_ENTRY_SEG_SYSTEM,
        GDT_ENTRY_DPL_KERNEL,
        GDT_ENTRY_PRESENT
    }}, 0, 0, 0);
    gdt[6] = higher;

    k_debug("gdt_entry (", "proto.kernel.gdt_init");
    print_f("%x): higher half tss\n", higher);

    // Generate GDTR to send to asm
    gdtr.base = (uint64_t)gdt;
    gdtr.limit = sizeof(gdt) - 1;

    k_debug("gdt base: ", "proto.kernel.gdt_init");
    print_f("%x, limit: %x\n", gdtr.base, gdtr.limit);

    __asm__ volatile (
        "lgdt %0"
        :
        : "m"(gdtr)
        : "memory"
    );

    _gdt_post_init();
}