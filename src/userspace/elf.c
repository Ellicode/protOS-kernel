#include <stddef.h>

#include "utils/utils.h"
#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "globals.h"
#include "string.h"

#include "userspace/elf.h"

void load_phdr(elf_header_t* ehdr, elf_program_header_t* phdr, uint64_t load_base) {
    if (phdr->p_type != PT_LOAD) {
        return;
    }

    uint64_t vaddr = phdr->p_vaddr + load_base;
    uint64_t vmm_flags = F_PRESENT | F_USER;

    if (phdr->p_flags & PF_W) {
        vmm_flags |= F_WRITE;
    }
    
    if (!(phdr->p_flags & PF_X)) {
        vmm_flags |= F_NX;
    }

    void* dest = vmm_map_range(vaddr, phdr->p_memsz, vmm_flags);

    print_f("load_phdr: vaddr=%x memsz=%x dest=%x\n", vaddr, phdr->p_memsz, dest);

    memcpy(dest + g_lim_hhdm->offset, (void*)ehdr + phdr->p_offset, phdr->p_filesz);
    memset(dest + g_lim_hhdm->offset + phdr->p_filesz, 0, phdr->p_memsz - phdr->p_filesz);
}

void *elf_load(char *data, size_t size)
{
    elf_header_t *ehdr = (elf_header_t *)data;

    if (ehdr->e_ident[0] != 0x7F ||
        ehdr->e_ident[1] != 'E'  ||
        ehdr->e_ident[2] != 'L'  ||
        ehdr->e_ident[3] != 'F')
    {
        k_error("The ELF file specified is not a valid ELF file.", "proto.kernel.load_elf");
        return NULL;
    }

    if (ehdr->e_machine != EM_X86_64)
    {
        k_error("The ELF file specified is not an x86_64 ELF file.", "proto.kernel.load_elf");
        return NULL;
    }

    uint64_t load_base = (ehdr->e_type == ET_DYN) ? USER_LOAD_BASE : 0;

    elf_program_header_t *phdrs = (elf_program_header_t*)((uintptr_t)ehdr + ehdr->e_phoff);

    for (uint64_t i = 0; i < ehdr->e_phnum; i++)
    {
        elf_program_header_t phdr = phdrs[i];
        load_phdr(ehdr, &phdr, load_base);
    }

    return (void*)(ehdr->e_entry + load_base);
}