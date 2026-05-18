#include <stdint.h>
#include <stddef.h>

#ifndef PMM_H
#define PMM_H

typedef uint8_t pmm_entry_t;

enum {
    PMM_USABLE = 1,
    PMM_USED   = 0,
};

int pmm_init();
void *pmm_allocate(size_t size);
void pmm_free(void *ptr, size_t size);

#endif // PMM_H