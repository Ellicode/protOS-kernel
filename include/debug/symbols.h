#ifndef SYMBOLS_H
#define SYMBOLS_H

typedef struct elf_st_entry_t {
    uint32_t st_name;
    unsigned char  st_info;
    unsigned char  st_other;
    uint16_t st_shndx;
    uint64_t st_value;
    uint64_t st_size;
} elf_st_entry_t;

int load_symbols();
char* get_symbol(uint64_t addr);

#endif // SYMBOLS_H