#include "graphics/tty.h"
#include "debug/logger.h"

#include "gdt.h"

gdt_entry_t gdt[7];
lgdt_descriptor_t gdtr;
tss_entry_t tss __attribute__((aligned(0x1000))) = {0};

static gdt_entry_t _gdt_generate_descriptor(
    uint32_t base, 
    uint32_t limit,
    GDTEntryAccessByte access,
    GDTEntryFlags flags
) {
    gdt_entry_t descriptor = 0x00000000;
    
    descriptor |= ((gdt_entry_t)(base & 0xFF000000) << 32);
    descriptor |= ((gdt_entry_t)(base & 0x00FFFFFF) << 16);    
    descriptor |= ((gdt_entry_t)(limit & 0x0000FFFF));
    descriptor |= ((gdt_entry_t)(limit & 0x000F0000) << 32);

    descriptor |= ((gdt_entry_t)access.value << 40);
    descriptor |= ((gdt_entry_t)(flags.value & 0x0F) << 52);

    k_debug("gdt_entry (", "proto.kernel._gdt_generate_descriptor");
    print_f("%x): base=%x, limit=%x, ring=%d, exe=%d, r/w=%d\n", descriptor, base, limit, access.dpl, access.executable, access.read_write);

    return descriptor;
}

static void _tss_generate() {
    tss.iopb = sizeof(TSSEntry);
}

void gdt_init() {
    // Generate null descriptor
    gdt[0] = _gdt_generate_descriptor(0, 0, GDT_NULL_ENTRY, GDT_NULL_FLAGS);

    // Generate kernel code segment
    gdt[1] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R0_CODE, GDT_NULL_FLAGS);

    // Generate kernel data segment
    gdt[2] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R0_DATA, GDT_NULL_FLAGS);

    // Generate user code segment
    gdt[3] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R3_CODE, GDT_NULL_FLAGS);

    // Generate user data segment
    gdt[4] = _gdt_generate_descriptor(GDT_ENTRY_BASE, GDT_ENTRY_LIMIT, GDT_R3_DATA, GDT_NULL_FLAGS);

    // Generate TSS segment
    _tss_generate();

    intptr_t addr = (intptr_t)&tss;
    uint32_t lower = (uint32_t)addr;
    uint32_t higher = (uint32_t)(addr >> 32);
    uint64_t limit = sizeof(tss) - 1;

    gdt[5] = _gdt_generate_descriptor(lower, limit, (GDTEntryAccessByte) {
        GDT_ENTRY_PRESENT,
        GDT_ENTRY_DPL_KERNEL,
        GDT_ENTRY_SEG_SYSTEM,
        GDT_OFF,
        GDT_OFF,
        GDT_OFF,
        GDT_OFF
    }, GDT_NULL_FLAGS);
    gdt[6] = higher;

    // Generate GDTR to send to asm
    gdtr.base = (uint64_t)gdt;
    gdtr.limit = sizeof(gdt) - 1;

    __asm__ __volatile__ (
        "lgdt %0"
        ::"m"(gdtr)
    );
}