#ifndef ELF_H
#define ELF_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint8_t  e_ident[16];   // magic + class + data + version + OS/ABI
    uint16_t e_type;        // ET_EXEC = 2
    uint16_t e_machine;     // EM_X86_64 = 0x3E
    uint32_t e_version;
    uint64_t e_entry;       // entry point virtual address
    uint64_t e_phoff;       // offset to program header table
    uint64_t e_shoff;       // offset to section header table
    uint32_t e_flags;
    uint16_t e_ehsize;      // size of this header (64 bytes)
    uint16_t e_phentsize;   // size of one program header entry
    uint16_t e_phnum;       // number of program headers
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} ElfHeader;
typedef ElfHeader elf_header_t; 

typedef struct {
    uint32_t p_type;        // PT_LOAD = 1
    uint32_t p_flags;       // PF_R=4, PF_W=2, PF_X=1
    uint64_t p_offset;      // offset of segment in file
    uint64_t p_vaddr;       // virtual address to map to
    uint64_t p_paddr;       // physical address (usually ignored)
    uint64_t p_filesz;      // size in file
    uint64_t p_memsz;       // size in memory (>= filesz, rest is zeroed)
    uint64_t p_align;
} ElfProgramHeader;
typedef ElfProgramHeader elf_program_header_t;

#define EM_X86_64   0x3E
#define PT_LOAD     1
#define USER_LOAD_BASE 0x400000

#define ET_EXEC  2   // wait...
#define ET_DYN   3

#define PF_X        0x1  // Executable segment
#define PF_W        0x2  // Writable segment
#define PF_R        0x4  // Readable segment

uint64_t elf_load(char *data, size_t size, uint64_t cr3);

#endif // ELF_H