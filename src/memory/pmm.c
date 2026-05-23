#include "debug/logger.h"
#include "graphics/tty.h"
#include "memory/vmm.h"
#include "limine/limine.h"
#include "globals.h"
#include "string.h"

#include "memory/pmm.h"

pmm_entry_t *pmm;
size_t pmm_size;

int get_pmm(void **out) {
    uint8_t found = 0;
    uint64_t highest = 0;

    for (size_t idx = 0; idx < g_lim_memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[idx];
        uint64_t end = entry->base + entry->length;
        if (end > highest) { highest = end; }
    }
    

    pmm_size = highest / PAGE_SIZE;
    uint64_t pmm_bytes_needed = pmm_size * sizeof(pmm_entry_t);

    k_debug("pmm_size=", "proto.kernel.get_pmm");
    #if (PROTO_DEBUG == 1)
        print_f("%d pages, needs %dKB\n", pmm_size, pmm_bytes_needed / 1024);
    #endif

    for (size_t idx = 0; idx < g_lim_memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE &&
            entry->base != 0 &&
            entry->length >= pmm_bytes_needed) {
            *out = (void *)entry->base;
            found = 1;
            break;
        }
    }

    if (found == 0) {
        return 1;
    }

    return 0;
}

void pmm_map(pmm_entry_t* pmm, uint64_t base, uint64_t size, pmm_entry_t entry) {
    uint64_t num_pages = size / PAGE_SIZE;
    uint64_t idx;

    for (uint64_t page = 0; page < num_pages; page++) {
        idx = (base / PAGE_SIZE) + page;
        if (idx >= pmm_size) {
            break;
        }
        pmm[idx] = entry;
    };
}

void *pmm_alloc(size_t size) {
    size_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    uint64_t run_start = UINT64_MAX;
    size_t run_len = 0;

    for (uint64_t idx = 1; idx < pmm_size; idx++) {
        if (pmm[idx] == PMM_USABLE) {
            if (run_start == UINT64_MAX) {
                run_start = idx;
                run_len = 1;
            } else {
                run_len++;
            }
            if (run_len == page_count) {
                for (uint64_t i = run_start; i <= idx; i++) {
                    pmm[i] = PMM_USED;
                }

                return (void *)(run_start * PAGE_SIZE);
            }
        } else {
            run_start = UINT64_MAX;
            run_len = 0;
        }
    }

    k_error("out of memory!\n", "proto.kernel.pmm_alloc");
    return NULL;
}

void *pmm_allocate_page() {
    for (uint64_t idx = 1; idx < pmm_size; idx++) {
        if (pmm[idx] == PMM_USABLE) {
            return (void *)(idx * PAGE_SIZE);
        }
    }
    
    k_error("out of memory!\n", "proto.kernel.pmm_alloc");
    return NULL;
}

void pmm_free(void *ptr, size_t size) {
    if ((uint64_t)ptr % PAGE_SIZE != 0) {
        k_warning("unaligned pointer", "proto.kernel.pmm_free");
        return;
    }

    uint64_t base_idx = (uint64_t)ptr / PAGE_SIZE;
    size_t page_count = (size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t i = 0; i < page_count; i++) {
        if (i+base_idx >= pmm_size) {
            k_warning("out of bounds! ", "proto.kernel.pmm_free");
            print_f("base_idx=%d, size=%d, i=%d\n", base_idx, size, i);

            break;
        }
        if (pmm[i+base_idx] == PMM_USED) {
            pmm[i+base_idx] = PMM_USABLE;
        }
    }
}

void pmm_free_page(void *ptr) {
    uint64_t pmm_idx = (uint64_t)ptr / PAGE_SIZE;

    if (pmm_idx >= pmm_size) {
        k_warning("out of bounds! ", "proto.kernel.pmm_free");
        print_f("pmm_idx=%d");
    }   

    if (pmm[pmm_idx] == PMM_USED) {
        pmm[pmm_idx] = PMM_USABLE;
    }
}

int pmm_init() {
    void *pmm_phys = NULL;
    if (get_pmm(&pmm_phys) != 0) {
        k_error("no region big enough for PMM!\n", "proto.kernel.pmm_init");
        return 1;
    }

    pmm = (pmm_entry_t *)((uint64_t)pmm_phys + g_lim_hhdm->offset);

    // zero the array first so no garbage entries
    memset(pmm, 0, pmm_size * sizeof(pmm_entry_t));

    k_debug("pmm info: ", "proto.kernel.pmm_init");
    #if (PROTO_DEBUG == 1)
        print_f("pmm_phys=%x, pmm_virt=%x\n", (uint64_t)pmm_phys, (uint64_t)pmm);
    #endif

    // mark everything according to memmap
    for (size_t idx = 0; idx < g_lim_memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            pmm_map(pmm, entry->base, entry->length, PMM_USABLE);
        } else {
            pmm_map(pmm, entry->base, entry->length, PMM_USED);
        }
    }

    // mark PMM array itself as used AFTER, so the loop above can't undo it
    pmm_map(pmm, (uint64_t)pmm_phys, pmm_size * sizeof(pmm_entry_t), PMM_USED);

    m_pmm_alloc_p = pmm_allocate_page;
    m_pmm_free_p = pmm_free_page;

    return 0;
}
