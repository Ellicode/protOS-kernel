#include <stdlib.h>

#include "graphics/tty.h"
#include "debug/logger.h"

#include "memory/memory.h"

void memmap_dump(struct limine_memmap_response *memmap) {
    k_debug("        Limine memmap debug dump\n", "proto.kernel.memmap_dump");
    print_f("        +------------------+------------------+----+\n");
    print_f("        |BASE              |LENGTH            |TYPE|\n");
    print_f("        +------------------+------------------+----+\n");

    for (size_t idx = 0; idx < memmap->entry_count; idx++) {
        struct limine_memmap_entry *entry = memmap->entries[idx];

        print_f("        |%18x|%18x|%4d|\n", entry->base, entry->length, entry->type);
    }
    print_f("        +------------------+------------------+----+\n");
}

