#include "limine/limine.h"

#ifndef BOOT_H
#define BOOT_H

int k_init(
    struct limine_framebuffer *fb, 
    struct limine_memmap_response *memmap, 
    struct limine_hhdm_response *hhdm,
    struct limine_executable_address_response *kaddr,
    struct limine_module_response *modules
);

#endif