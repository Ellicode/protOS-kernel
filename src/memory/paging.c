#include "memory/paging.h"

void load_cr3(uint64_t pml4_physical) {
    __asm__ volatile ("mov %0, %%cr3" : : "r" (pml4_physical) : "memory");
}

void _enable_paging() {
    uint64_t cr0;
    __asm__ volatile ("mov %%cr0, %0" : "=r" (cr0));
    cr0 |= (1 << 16); // Set WP (Write Protect) bit
    __asm__ volatile ("mov %0, %%cr0" : : "r" (cr0));
}

void paging_init(struct limine_memmap_response *memmap, struct limine_hhdm_response *hhdm) {

}