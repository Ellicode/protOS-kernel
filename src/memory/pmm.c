#include <stddef.h>

#include "debug/logger.h"
#include "graphics/tty.h"
#include "memory/paging.h"
#include "limine/limine.h"

#include "memory/pmm.h"

pmm_entry_t *pmm;
size_t pmm_size;

int get_pmm(struct limine_memmap_response *memmap, void **out) {
    uint64_t total_usable = 0;
    uint8_t found = 0;

    for (size_t idx = 0; idx < memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = memmap->entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            total_usable += entry->length;
        }
    }

    pmm_size = total_usable / PAGE_SIZE;
    k_debug("pmm_size=", "proto.kernel.get_pmm");
    print_f("%x\n", pmm_size);

    for (size_t idx = 0; idx < memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = memmap->entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE &&  entry->base != 0 && entry->length >= pmm_size) {
            *out = (void *)entry->base;
            found = 1;
            break;
        }
    }

    if (found == 0) { 
        k_error("No region big enough for PMM!\n", "proto.kernel.pmm_init");
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

int pmm_init(struct limine_memmap_response *memmap, struct limine_hhdm_response *hhdm) {
    void *pmm_phys = NULL;
    if (get_pmm(memmap, &pmm_phys) != 0) {
        k_error("No region big enough for PMM!\n", "proto.kernel.pmm_init");
        return 1;
    }

    // apply HHDM offset so we can actually write to it from higher half
    pmm = (pmm_entry_t *)((uint64_t)pmm_phys + hhdm->offset);
    // pmm_map(pmm, (uint64_t)pmm, )

    k_debug("pmm info: ", "proto.kernel.pmm_init");
    print_f("pmm_phys=%x, pmm_virt=%x\n", (uint64_t)pmm_phys, (uint64_t)pmm);

    for (size_t idx = 0; idx < memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = memmap->entries[idx];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            pmm_map(pmm, entry->base, entry->length, PMM_USABLE);
        } else {
            pmm_map(pmm, entry->base, entry->length, PMM_USED);
        }
    }

    uint64_t free_pages = 0;
    for (uint64_t i = 0; i < pmm_size; i++) {
        if (pmm[i] == PMM_USABLE) { free_pages++; }
    }

    k_debug("free pages: ", "proto.kernel.pmm_init");
    print_f("%d (%dMB)\n", free_pages, (free_pages * PAGE_SIZE) / (1024 * 1024));

    return 0;
}
