#include "utils/ticket_lock.h"  
#include "memory/freelist_pmm.h"
#include "graphics/console.h"
#include "debug/logger.h"
#include "globals.h"
#include "string.h"

#include "memory/vmm.h"

pt_entry_t* kernel_pml4;
ticketlock_t vmm_lock = {0};

void _load_cr3(uint64_t pml4_physical) {
    __asm__ volatile ("mov %0, %%cr3" : : "r" (pml4_physical) : "memory");
}

void _enable_paging() {
    uint64_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 16); // Set WP (Write Protect) bit
    __asm__ volatile ("mov %0, %%cr0" : : "r" (cr0));
}

uint64_t* _get_or_create(pt_entry_t* entry, uint64_t flags) {
    PageTableEntry entry_c = { .value = *entry };

    if (entry_c.present) {
        // OR in new flags in case this table was created kernel-only before
        *entry |= (flags & (F_USER | F_WRITE));
        uint64_t phys = entry_c.addr << 12;
        return (uint64_t*)(phys + g_lim_hhdm->offset);
    }

    uint64_t phys = (uint64_t)m_pmm_alloc_p();
    uint64_t* table_virt = (uint64_t*)(phys + g_lim_hhdm->offset);

    memset(table_virt, 0, PAGE_SIZE);
    *entry = (phys & 0x000FFFFFFFFFF000ULL) | F_PRESENT | F_WRITE | (flags & F_USER);

    return table_virt;
}

void map_page(uint64_t virt, uint64_t phys, uint64_t flags, pt_entry_t* pml4) {
    VirtualAddress v = { .value = virt };

    uint64_t* pdpt = _get_or_create(&pml4[v.pml4],          flags);
    uint64_t* pd   = _get_or_create(&pdpt[v.page_dir_ptr],  flags);
    uint64_t* pt   = _get_or_create(&pd[v.page_dir],        flags);

    pt_entry_t* entry = &pt[v.page_table];
    *entry = (phys & 0x000FFFFFFFFFF000ULL) | flags | F_PRESENT;
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

    k_debug("Page table entries dump: ");
    #if (PROTO_DEBUG == 1)
        print_f("pml4=%d pdpt=%d pd=%d pt=%d (pages=%d, %dMB)\n",
            pml4_count, pdpt_count, pd_count, pt_count,
            pt_count, (pt_count * PAGE_SIZE) / (uint64_t)(1024 * 1024));
    #endif
    k_debug("Page table size: ");
    #if (PROTO_DEBUG == 1)
        print_f("%d tables, %dMB\n",
            table_count, table_bytes / (uint64_t)(1024 * 1024));
    #endif
}

void *vmm_map_range(uint64_t cr3, uint64_t virt_start, size_t size, uint64_t flags) {
    pt_entry_t *pml4 = (pt_entry_t *)(cr3 + g_lim_hhdm->offset);

    if (pml4 == NULL) {
        k_warning("Tried to map before initializing paging\n");
    }

    int lock1r = ticketlock_lock(&vmm_lock);
    
    virt_start = PAGE_ALIGN(virt_start);  // ensure page alignment
    size_t num_pages = PAGE_ROUND(size) / PAGE_SIZE;

    for (uint64_t idx = 0; idx < num_pages; idx++) {
        uint64_t page = (uint64_t)m_pmm_alloc_p();

        if (page == 0) {
            k_error("out of memory!\n");
            ticketlock_unlock(&vmm_lock, lock1r);
            return NULL;
        }

        map_page(virt_start + idx*PAGE_SIZE, page, flags, pml4);
    }

    ticketlock_unlock(&vmm_lock, lock1r);

    return (void *)virt_start;
}

void *vmm_map_phys_range(uint64_t cr3, uint64_t virt_start, uint64_t phys_start, size_t size, uint64_t flags) {
    pt_entry_t *pml4 = (pt_entry_t *)(cr3 + g_lim_hhdm->offset);

    if (pml4 == NULL) {
        k_warning("Tried to map before initializing paging\n");
        return NULL;
    }

    int lock1r = ticketlock_lock(&vmm_lock);
    
    virt_start = PAGE_ALIGN(virt_start);
    phys_start = PAGE_ALIGN(phys_start);
    size_t num_pages = PAGE_ROUND(size) / PAGE_SIZE;

    for (uint64_t idx = 0; idx < num_pages; idx++) {
        uint64_t target_virt = virt_start + (idx * PAGE_SIZE);
        uint64_t target_phys = phys_start + (idx * PAGE_SIZE);
        
        map_page(target_virt, target_phys, flags, pml4);
    }

    ticketlock_unlock(&vmm_lock, lock1r);

    return (void *)virt_start;
}

void vmm_flush_tlb() {
    uint64_t cr3;
    __asm__ volatile (
        "mov %%cr3, %0\n"
        "mov %0, %%cr3\n"
        : "=r"(cr3) :: "memory"
    );
}

uint64_t create_user_pml4() {
    uint64_t phys = (uint64_t)m_pmm_alloc_p();
    uint64_t virt = phys + g_lim_hhdm->offset;
    memset((void*)virt, 0, PAGE_SIZE);

    pt_entry_t *new_pml4  = (pt_entry_t*)virt;
    pt_entry_t *kern_pml4 = kernel_pml4;  // already virtual, no offset needed!

    for (int i = 256; i < 512; i++) {
        new_pml4[i] = kern_pml4[i];
    }

    return phys;  // return physical for cr3!
}

void vmm_init() {
    uint64_t phys = (uint64_t)m_pmm_alloc_p();
    kernel_pml4 = (pt_entry_t*)(phys + g_lim_hhdm->offset);

    k_debug("pml4_phys=");
    #if (PROTO_DEBUG == 1)
        print_f("%x, pml4_virt=%x\n", phys, kernel_pml4);
    #endif

    memset(kernel_pml4, 0, PAGE_SIZE);

    // Map the kernel
    uint64_t kphys = g_lim_kaddr->physical_base;
    uint64_t kvirt = g_lim_kaddr->virtual_base;
    uint64_t ksize = _kernel_size();

    for (uint64_t off = 0; off < ksize; off += PAGE_SIZE) {
        map_page(kvirt + off, kphys + off, F_WRITE, kernel_pml4);
    }

    k_debug("Mapped kernel region\n");

    // Map HHDM entries
    for (size_t i = 0; i < g_lim_memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[i];
        for (uint64_t off = 0; off < entry->length; off += PAGE_SIZE) {
            map_page(entry->base + off + g_lim_hhdm->offset,
                    entry->base + off,
                    F_WRITE, kernel_pml4);
        }
    }

    k_debug("Mapped HHDM region\n");

    // Map framebuffer
    uint64_t fb_phys = (uint64_t)g_vga_active_framebuffer->address - g_lim_hhdm->offset;
    uint64_t fb_size = g_vga_active_framebuffer->pitch * g_vga_active_framebuffer->height;

    for (uint64_t off = 0; off < fb_size; off += PAGE_SIZE) {
        map_page(fb_phys + off + g_lim_hhdm->offset, fb_phys + off, F_WRITE | F_PCD, kernel_pml4);
    }

    k_debug("Mapped framebuffer region\n");

    // _tables_dump(pml4);

    _load_cr3(phys);
    _enable_paging();

    k_debug("CR3 loaded and paging enabled\n");

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

    // k_info("Reclaimed ", "proto.kernel.vmm_init");
    // print_f("%dMB from bootloader reclaimable memory\n", space_reclaimed / (uint64_t)(1024 * 1024));
}

uint64_t vmm_virt_to_phys(uint64_t cr3, uint64_t virt) {
    pt_entry_t *pml4 = (pt_entry_t*)(cr3 + g_lim_hhdm->offset);
    VirtualAddress v = { .value = virt };

    PageTableEntry pml4e = { .value = pml4[v.pml4] };
    if (!pml4e.present) { return 0; }
    uint64_t* pdpt = (uint64_t*)((pml4e.addr << 12) + g_lim_hhdm->offset);

    PageTableEntry pdpte = { .value = pdpt[v.page_dir_ptr] };
    if (!pdpte.present) { return 0; }
    uint64_t* pd = (uint64_t*)((pdpte.addr << 12) + g_lim_hhdm->offset);

    PageTableEntry pde = { .value = pd[v.page_dir] };
    if (!pde.present) { return 0; }
    uint64_t* pt = (uint64_t*)((pde.addr << 12) + g_lim_hhdm->offset);

    PageTableEntry pte = { .value = pt[v.page_table] };
    if (!pte.present) { return 0; }

    return (pte.addr << 12) | v.offset;
}