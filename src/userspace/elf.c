#include <stddef.h>

#include "utils/utils.h"
#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "string.h"

#include "userspace/elf.h"

uint64_t load_elf(uint8_t *data, size_t size)
{
    elf_header_t *ehdr = (elf_header_t *)data;

    if (ehdr->e_ident[0] != 0x7F ||
        ehdr->e_ident[1] != 'E'  ||
        ehdr->e_ident[2] != 'L'  ||
        ehdr->e_ident[3] != 'F')
    {
        k_error("The ELF file specified is not a valid ELF file.", "proto.kernel.load_elf");
        return 1;
    }

    if (ehdr->e_machine != EM_X86_64)
    {
        k_error("The ELF file specified is not an x86_64 ELF file.", "proto.kernel.load_elf");
        return 1;
    }
    for (uint64_t i = 0; i < ehdr->e_phnum; i++)
    {
        elf_program_header_t *phdr = (elf_program_header_t *)((ptrdiff_t)data + ehdr->e_phoff + i * ehdr->e_phentsize);
        if (phdr->p_type != PT_LOAD) {
            continue;
        }
        vmm_map_range(PAGE_ALIGN(phdr->p_vaddr), phdr->p_memsz, F_USER | F_WRITE);
        vmm_flush_tlb();
        memcpy((void *)phdr->p_vaddr, data + phdr->p_offset, phdr->p_filesz);
        if (phdr->p_memsz > phdr->p_filesz) {
            memset((void *)(phdr->p_vaddr + phdr->p_filesz), 0, phdr->p_memsz - phdr->p_filesz);
        }
    }

    return ehdr->e_entry;
}