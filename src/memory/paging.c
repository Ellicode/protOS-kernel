#include "memory/pmm.h"
#include "graphics/tty.h"
#include "debug/logger.h"
#include "globals.h"
#include "string.h"

#include "memory/paging.h"

void _load_cr3(uint64_t pml4_physical) {
    __asm__ volatile ("mov %0, %%cr3" : : "r" (pml4_physical) : "memory");
}

void _enable_paging() {
    uint64_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 16); // Set WP (Write Protect) bit
    __asm__ volatile ("mov %0, %%cr0" : : "r" (cr0));
}

uint64_t* _get_or_create(pt_entry_t* entry) {
    PageTableEntry entry_c = { .value = *entry };

    if (entry_c.present) {
        uint64_t phys = entry_c.addr << 12;
        return (uint64_t*)(phys + g_lim_hhdm->offset);
    }

    uint64_t phys = (uint64_t)pmm_alloc(PAGE_SIZE);
    uint64_t* table_virt = (uint64_t*)(phys + g_lim_hhdm->offset);

    memset(table_virt, 0, PAGE_SIZE);
    *entry = (phys & 0x000FFFFFFFFFF000ULL) | 0b11;

    return table_virt;
}

void map_page(uint64_t virt, uint64_t phys, uint64_t flags, pt_entry_t* pml4) {
    VirtualAddress v = { .value = virt };

    uint64_t* pdpt = _get_or_create(&pml4[v.pml4]);
    uint64_t* pd   = _get_or_create(&pdpt[v.page_dir_ptr]);
    uint64_t* pt   = _get_or_create(&pd[v.page_dir]);

    pt_entry_t* entry = &pt[v.page_table];

    uint64_t value = 0;

    value |= (phys & 0x000FFFFFFFFFF000ULL);
    value |= (flags | F_PRESENT);

    *entry = value;
}

uint64_t _kernel_size() {
    return (uint64_t)&g_kernel_end - (uint64_t)&g_kernel_start;
}

void _tables_dump(pt_entry_t* pml4) {
    uint64_t pml4_count = 0;
    uint64_t pdpt_count = 0;
    uint64_t pd_count   = 0;
    uint64_t pt_count   = 0;

    for (int i = 0; i < 512; i++) {
        PageTableEntry pml4e = { .value = pml4[i] };
        if (!pml4e.present) { continue; }
        pml4_count++;

        uint64_t* pdpt = (uint64_t*)((pml4e.addr << 12) + g_lim_hhdm->offset);
        for (int j = 0; j < 512; j++) {
            PageTableEntry pdpte = { .value = pdpt[j] };
            if (!pdpte.present) { continue; }
            pdpt_count++;

            uint64_t* pd = (uint64_t*)((pdpte.addr << 12) + g_lim_hhdm->offset);
            for (int k = 0; k < 512; k++) {
                PageTableEntry pde = { .value = pd[k] };
                if (!pde.present) { continue; }
                pd_count++;

                uint64_t* pt = (uint64_t*)((pde.addr << 12) + g_lim_hhdm->offset);
                for (int l = 0; l < 512; l++) {
                    PageTableEntry pte = { .value = pt[l] };
                    if (!pte.present) { continue; }
                    pt_count++;
                }
            }
        }
    }

    // 1 pml4 + all pdpt/pd/pt tables, each is one page
    uint64_t table_count = 1 + pml4_count + pdpt_count + pd_count;
    uint64_t table_bytes = table_count * PAGE_SIZE;

    k_debug("Page table entries dump: ", "proto.kernel._tables_dump");
    print_f("pml4=%d pdpt=%d pd=%d pt=%d (pages=%d, %dMB)\n",
        pml4_count, pdpt_count, pd_count, pt_count,
        pt_count, (pt_count * PAGE_SIZE) / (1024 * 1024));
    k_debug("Page table size: ", "proto.kernel._tables_dump");
    print_f("%d tables, %dMB\n",
        table_count, table_bytes / (1024 * 1024));
}

void paging_init() {
    uint64_t phys = (uint64_t)pmm_alloc(PAGE_SIZE);
    pt_entry_t* pml4 = (pt_entry_t*)(phys + g_lim_hhdm->offset);

    k_debug("pml4_phys=", "proto.kernel.paging_init");
    print_f("%x, pml4_virt=%x\n", phys, pml4);

    memset(pml4, 0, PAGE_SIZE);

    // Map the kernel
    uint64_t kphys = g_lim_kaddr->physical_base;
    uint64_t kvirt = g_lim_kaddr->virtual_base;
    uint64_t ksize = _kernel_size();

    for (uint64_t off = 0; off < ksize; off += PAGE_SIZE) {
        map_page(kvirt + off, kphys + off, F_WRITE, pml4);
    }

    k_debug("Mapped kernel region\n", "proto.kernel.paging_init");

    // Map the memory map entries
    for (size_t i = 0; i < g_lim_memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[i];
        for (uint64_t off = 0; off < entry->length; off += PAGE_SIZE) {
            map_page(entry->base + off + g_lim_hhdm->offset,
                    entry->base + off,
                    F_WRITE, pml4);
        }
    }

    k_debug("Mapped entries\n", "proto.kernel.paging_init");

    // Map framebuffer
    uint64_t fb_phys = (uint64_t)g_vga_active_framebuffer->address - g_lim_hhdm->offset; // or however you access it
    uint64_t fb_size = g_vga_active_framebuffer->pitch * g_vga_active_framebuffer->height;

    for (uint64_t off = 0; off < fb_size; off += PAGE_SIZE) {
        map_page(fb_phys + off + g_lim_hhdm->offset, fb_phys + off, F_WRITE, pml4);
    }

    k_debug("Mapped framebuffer\n", "proto.kernel.paging_init");

    // _tables_dump(pml4);

    _load_cr3(phys);
    _enable_paging();

    k_debug("CR3 loaded and paging enabled\n", "proto.kernel.paging_init");

    // Reclaim bootloader reclaimable regions
    uint64_t space_reclaimed = 0;

    for (size_t i = 0; i < g_lim_memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[i];

        if (entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            pmm_free((void *)entry->base, entry->length);

            space_reclaimed += entry->length;
        }
    }

    k_success("Reclaimed ", "proto.kernel.paging_init");
    print_f("%dMB from bootloader reclaimable memory\n", space_reclaimed / (1024 * 1024));
}