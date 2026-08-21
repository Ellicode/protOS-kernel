#include "userspace/elf.h"
#include "debug/errors.h"
#include "globals.h"

#include "debug/symbols.h"

static elf_st_entry_t *symtab = NULL;
static uint64_t symtab_entries = 0;
static char *strtab = NULL;

char* get_symbol(uint64_t addr) {
    int closest_delta = 0;

    elf_st_entry_t *best_match = NULL;
    uint64_t highest_low_addr = 0;
    
    for (int i = 0; i < symtab_entries; i++) {
        elf_st_entry_t symbol = symtab[i];
        char *sym_name = (char *)strtab + symbol.st_name;

        if (symbol.st_value == 0) continue;
        if (symbol.st_size > 0 && addr >= symbol.st_value && addr < (symbol.st_value + symbol.st_size)) {
            return sym_name;
        } else if (addr >= symbol.st_value && symbol.st_value > highest_low_addr) {
            highest_low_addr = symbol.st_value;
            best_match = &symbol;
        }
    }
    
    if (best_match != NULL) { return (char *)strtab + best_match->st_name; }

    return "???";
}

int load_symbols() {
    uint64_t k_file_addr = (uint64_t)g_lim_kfile->executable_file->address;

    elf_t elf = (elf_t) { 0 };
    int res = elf_read(k_file_addr, &elf);
    if (res != PROTO_OK) { return res; }

    uint64_t shstrtab = k_file_addr + elf.shdrs[elf.header->e_shstrndx].sh_offset;

    for (int i = 0; i < elf.header->e_shnum; i++) {
        elf_section_header_t shdr = elf.shdrs[i];
        
        char *sname = (char *)shstrtab + shdr.sh_name;

        // SYMTAB
        if (shdr.sh_type == 0x2 && symtab == NULL) { 
            symtab = (elf_st_entry_t *)(k_file_addr + shdr.sh_offset); 
            symtab_entries = shdr.sh_size / sizeof(elf_st_entry_t);
        }

        // STRTAB
        if (shdr.sh_type == 0x3 && strtab == NULL) { strtab = (char *)(k_file_addr + shdr.sh_offset); }
    }

    get_symbol(0x0);

    return PROTO_OK;
}