#include <stdint.h>

#ifndef PMM_H
#define PMM_H

typedef uint8_t pmm_entry_t;

enum {
    PMM_USABLE = 1,
    PMM_USED   = 0,
};

int pmm_init(struct limine_memmap_response *memmap, struct limine_hhdm_response *hhdm);

#endif // PMM_H