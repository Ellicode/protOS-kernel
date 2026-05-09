#include "graphics/tty.h"

#include "memory.h"

void memmap_dump(struct limine_memmap_response *memmap) {
    print_f("|------------------------------------------------|\n");
    print_f("|  Physical Base     | Region size        | Type |\n");
    print_f("|------------------------------------------------|\n");
    for (uint64_t i = 0; i < memmap->entry_count; i++) {
        struct limine_memmap_entry *entry = memmap->entries[i];
        if (entry->type == LIMINE_MEMMAP_USABLE) {
            set_color(PROTO_BLUE);
        } else if (entry->type == LIMINE_MEMMAP_ACPI_RECLAIMABLE || entry->type == LIMINE_MEMMAP_BOOTLOADER_RECLAIMABLE) {
            set_color(PROTO_CYAN);
        } else {
            set_color(PROTO_GREY);
        }
        print_f("| %18x | %18x | %4d |\n", entry->base, entry->length, entry->type);
        set_color(PROTO_WHITE);
    }
    print_f("|------------------------------------------------|\n");

}