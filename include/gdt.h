#include <stdint.h>

#ifndef GDT_H
#define GDT_H

enum {
    GDT_ENTRY_DPL_KERNEL        = 0,
    GDT_ENTRY_DPL_USER          = 3
};
typedef uint8_t gdt_dpl_t;

enum {
    GDT_ENTRY_SEG_SYSTEM        = 0,
    GDT_ENTRY_SEG_CODE_DATA     = 1
};
typedef uint8_t gdt_type_t;

enum {
    GDT_ENTRY_GRANULARITY_BYTE  = 0,
    GDT_ENTRY_GRANULARITY_PAGES = 1
};
typedef uint8_t gdt_gran_t;

enum {
    GDT_ENTRY_SIZE_16           = 0,
    GDT_ENTRY_SIZE_32           = 1
};
typedef uint8_t gdt_size_t;

enum {
    GDT_ENTRY_PRESENT           = 1,
    GDT_ENTRY_EXECUTABLE        = 1,
    GDT_ENTRY_EXPAND_DOWN       = 1,
    GDT_READ_ALLOWED            = 1,
    GDT_WRITE_ALLOWED           = 1,
    GDT_ACCESSED                = 1,

    GDT_OFF                     = 0,
};

typedef union {
    struct {
        uint8_t    accessed       : 1;
        uint8_t    read_write     : 1;
        uint8_t    direction      : 1;
        uint8_t    executable     : 1;
        gdt_type_t type           : 1;
        gdt_dpl_t  dpl            : 2;
        uint8_t    present        : 1;
    } __attribute__((packed));
    uint8_t value;
} GDTEntryAccessByte;
typedef uint8_t access_byte_t;

typedef union {
    struct {
        uint16_t      limit_low;
        uint16_t      base_low;
        uint8_t       base_mid;
        access_byte_t access_byte;
        uint8_t       limit_high  : 4;
        uint8_t       _reserved   : 1;
        uint8_t       longa       : 1;
        uint8_t       db          : 1;
        uint8_t       granuality  : 1;
        uint8_t       base_high;
    } __attribute__((packed));
    uint64_t value;
} GDTEntry;
typedef uint64_t gdt_entry_t;

#define GDT_R0_CODE ((GDTEntryAccessByte) {{ \
    GDT_ACCESSED, \
    GDT_READ_ALLOWED, \
    GDT_OFF, \
    GDT_ENTRY_EXECUTABLE, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_KERNEL, \
    GDT_ENTRY_PRESENT \
}})

#define GDT_R0_DATA ((GDTEntryAccessByte) {{ \
    GDT_ACCESSED, \
    GDT_WRITE_ALLOWED, \
    GDT_OFF, \
    GDT_OFF, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_KERNEL, \
    GDT_ENTRY_PRESENT \
}})

#define GDT_R3_CODE ((GDTEntryAccessByte) {{ \
    GDT_ACCESSED, \
    GDT_READ_ALLOWED, \
    GDT_OFF, \
    GDT_ENTRY_EXECUTABLE, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_USER, \
    GDT_ENTRY_PRESENT \
}})

#define GDT_R3_DATA ((GDTEntryAccessByte) {{ \
    GDT_ACCESSED, \
    GDT_WRITE_ALLOWED, \
    GDT_OFF, \
    GDT_OFF, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_USER, \
    GDT_ENTRY_PRESENT \
}})

#define GDT_NULL_ENTRY (GDTEntryAccessByte) { 0 }

#define GDT_ENTRY_BASE  0x00000
#define GDT_ENTRY_LIMIT 0xFFFFF

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) GDTR;

typedef GDTR gdtr_t;

typedef struct {
    uint32_t reserved0;

    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;

    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;

    uint64_t reserved1;

    uint16_t reserved2;
    uint16_t iopb;
} __attribute__((packed)) TSSEntry;
typedef TSSEntry tss_entry_t;

#define GDT_OFFSET_KERNEL_CODE (1*8)
#define GDT_OFFSET_KERNEL_DATA (2*8)
#define GDT_OFFSET_USER_CODE   (3*8)
#define GDT_OFFSET_USER_DATA   (4*8)
#define GDT_OFFSET_TSS         (5*8)

void gdt_init();

#endif // GDT_H