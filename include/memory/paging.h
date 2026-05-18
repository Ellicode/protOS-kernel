#include <stdint.h>

#include "limine/limine.h"

#ifndef PAGING_H
#define PAGING_H

typedef union {
    struct {
        uint8_t present           :  1;
        uint8_t writeable         :  1;
        uint8_t user              :  1;
        uint8_t write_trough      :  1;
        uint8_t cache_disable     :  1;
        uint8_t accessed          :  1;
        uint8_t dirty             :  1;
        uint8_t pat               :  1;
        uint8_t global            :  1;
       uint64_t avl_low           :  3;
       uint64_t addr              : 40;
       uint64_t avl_high          : 11;
       uint64_t no_execute        :  1;
    } __attribute__((packed));
    uint64_t value;
} PageTableEntry;
typedef uint64_t pt_entry_t;

typedef union {
    struct {
       uint64_t offset            : 12;  // Bits 0-11  : Byte offset within the 4 KiB page
       uint64_t page_table        :  9;  // Bits 12-20 : Index into Page Table (PT)
       uint64_t page_dir          :  9;  // Bits 21-29 : Index into Page Directory (PD)
       uint64_t page_dir_ptr      :  9;  // Bits 30-38 : Index into Page Directory Pointer Table (PDPT)
       uint64_t pml4              :  9;  // Bits 39-47 : Index into Page Map Level 4 (PML4)
       uint64_t sign_ext          : 16;  // Bits 48-63 : Must mirror Bit 47 (0x0000 or 0xFFFF)
    } __attribute__((packed));
    uint64_t value;
} VirtualAddress;
typedef uint64_t virtual_address_t;

#define PAGE_SIZE 4096

#define F_PRESENT   (1ULL << 0)
#define F_WRITE     (1ULL << 1)
#define F_USER      (1ULL << 2)
#define F_PWT       (1ULL << 3)
#define F_PCD       (1ULL << 4)
#define F_ACCESSED  (1ULL << 5)
#define F_DIRTY     (1ULL << 6)
#define F_HUGE      (1ULL << 7)
#define F_GLOBAL    (1ULL << 8)
#define F_NX        (1ULL << 63)

void paging_init();

#endif // PAGING_H