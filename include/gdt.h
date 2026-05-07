#include <stdint.h>

#ifndef GDT_H
#define GDT_H

enum {
    GDT_ENTRY_DPL_KERNEL = 0,
    GDT_ENTRY_DPL_USER = 3
};
typedef uint8_t gdt_dpl_t;

enum {
    GDT_ENTRY_SEG_SYSTEM = 0,
    GDT_ENTRY_SEG_CODE_DATA = 1
};
typedef uint8_t gdt_type_t;

enum {
    GDT_ENTRY_GRANULARITY_BYTE = 0,
    GDT_ENTRY_GRANULARITY_PAGES = 1
};
typedef uint8_t gdt_gran_t;

enum {
    GDT_ENTRY_SIZE_16 = 0,
    GDT_ENTRY_SIZE_32 = 1
};
typedef uint8_t gdt_size_t;

enum {
    GDT_ENTRY_PRESENT = 1,
    GDT_ENTRY_EXECUTABLE = 1,
    GDT_ENTRY_EXPAND_DOWN = 1,
    GDT_READ_ALLOWED = 1,
    GDT_WRITE_ALLOWED = 1,
    GDT_ACCESSED = 1,

    GDT_OFF = 0,
};

typedef union {
    struct {
        uint8_t    accessed      : 1;
        uint8_t    read_write    : 1;
        uint8_t    direction     : 1;
        uint8_t    executable    : 1;
        gdt_type_t type          : 1;
        gdt_dpl_t  dpl           : 2;
        uint8_t    present       : 1;
    } __attribute__((packed));
    uint8_t value;
} GDTEntryAccessByte;

typedef union {
    struct {
        uint8_t    reserved      : 1;
        uint8_t    long_mode     : 1;
        gdt_size_t size          : 1;
        gdt_gran_t granularity   : 1;
    } __attribute__((packed));
    uint8_t value;
} GDTEntryFlags;

typedef uint64_t gdt_entry_t;

#define GDT_R0_CODE ((GDTEntryAccessByte) { \
    GDT_OFF,                 /* accessed (bit 0) */ \
    GDT_READ_ALLOWED,        /* read/write (bit 1) */ \
    GDT_OFF,                 /* direction (bit 2) */ \
    GDT_ENTRY_EXECUTABLE,    /* executable (bit 3) */ \
    GDT_ENTRY_SEG_CODE_DATA, /* type (bit 4) */ \
    GDT_ENTRY_DPL_KERNEL,    /* dpl (bits 5–6) */ \
    GDT_ENTRY_PRESENT        /* present (bit 7) */ \
})

#define GDT_R0_DATA ((GDTEntryAccessByte) { \
    GDT_OFF,                 /* accessed */ \
    GDT_WRITE_ALLOWED,       /* read/write */ \
    GDT_OFF,                 /* direction */ \
    GDT_OFF,                 /* executable */ \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_KERNEL, \
    GDT_ENTRY_PRESENT \
})

#define GDT_R3_CODE ((GDTEntryAccessByte) { \
    GDT_OFF, \
    GDT_READ_ALLOWED, \
    GDT_OFF, \
    GDT_ENTRY_EXECUTABLE, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_USER, \
    GDT_ENTRY_PRESENT \
})

#define GDT_R3_DATA ((GDTEntryAccessByte) { \
    GDT_OFF, \
    GDT_WRITE_ALLOWED, \
    GDT_OFF, \
    GDT_OFF, \
    GDT_ENTRY_SEG_CODE_DATA, \
    GDT_ENTRY_DPL_USER, \
    GDT_ENTRY_PRESENT \
})

#define GDT_NULL_ENTRY (GDTEntryAccessByte) { 0 }
#define GDT_NULL_FLAGS (GDTEntryFlags) { 0 }

#define GDT_ENTRY_BASE  0x00000
#define GDT_ENTRY_LIMIT 0xFFFFF

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) LGDTDescriptor;

typedef LGDTDescriptor lgdt_descriptor_t;

typedef struct
{
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t _reserved_1;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    uint64_t _reserved_2;
    uint16_t _reserved_3;
    uint16_t iopb;
} __attribute__((packed)) TSSEntry;

typedef TSSEntry tss_entry_t;

void gdt_init();
extern void setGDT(uint16_t limit, uint64_t base);

#endif // GDT_H