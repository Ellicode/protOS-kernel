#include <stddef.h>

#include "utils/utils.h"
#include "debug/errors.h"
#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "globals.h"
#include "string.h"

#include "userspace/elf.h"

void load_phdr(elf_header_t* ehdr, elf_program_header_t* phdr, uint64_t load_base, uint64_t cr3) {
    if (phdr->p_type != PT_LOAD) { return; }

    uint64_t vaddr     = phdr->p_vaddr + load_base;
    uint64_t vmm_flags = F_PRESENT | F_USER;
    if (phdr->p_flags & PF_W)       { vmm_flags |= F_WRITE; }
    if (!(phdr->p_flags & PF_X))    { vmm_flags |= F_NX;    }

    // map into user PML4
    vmm_map_range(cr3, vaddr, phdr->p_memsz, vmm_flags);

    // write page by page through HHDM
    uint64_t written = 0;
    uint64_t filesz  = phdr->p_filesz;
    uint64_t memsz   = phdr->p_memsz;

    while (written < memsz) {
        uint64_t page_vaddr = vaddr + written;

        // walk the user PML4 to get physical address
        uint64_t phys = vmm_virt_to_phys(cr3, page_vaddr);
        void    *hhdm = (void*)(phys + g_lim_hhdm->offset);

        uint64_t to_write = PAGE_SIZE;

        if (written < filesz) {
            uint64_t src_off = phdr->p_offset + written;
            uint64_t copy    = MIN(to_write, filesz - written);
            memcpy(hhdm, (void*)ehdr + src_off, copy);
            if (copy < to_write) {
                memset(hhdm + copy, 0, to_write - copy);  // zero BSS portion
            }
        } else {
            memset(hhdm, 0, to_write);  // pure BSS page
        }

        written += PAGE_SIZE;
    }
}

int elf_read(uint64_t addr, elf_t *elf) {
    elf_header_t *ehdr = (elf_header_t *)addr;

    if (strncmp(ehdr->e_ident, "\x7FELF", 4) == 0) {
        k_assert(PROTO_ERR_ELF_INVALID_HDR);
        return PROTO_ERR_ELF_INVALID_HDR;
    }

    if (ehdr->e_machine != EM_X86_64) {
        k_assert(PROTO_ERR_ELF_UNSUPPORTED);
        return PROTO_ERR_ELF_UNSUPPORTED;
    }

    elf_program_header_t *phdrs = (elf_program_header_t*)((uintptr_t)ehdr + ehdr->e_phoff);
    elf_section_header_t *shdrs = (elf_section_header_t*)((uintptr_t)ehdr + ehdr->e_shoff);

    elf->header = ehdr;
    elf->phdrs = phdrs;
    elf->shdrs = shdrs;

    return PROTO_OK;
}

uint64_t elf_load(uint64_t addr, uint64_t cr3) {
    elf_t elf = (elf_t) { 0 };
    int res = elf_read(addr, &elf);
    if (res != PROTO_OK) { return res; }

    uint64_t load_base = (elf.header->e_type == ET_DYN) ? USER_LOAD_BASE : 0;

    for (uint64_t i = 0; i < elf.header->e_phnum; i++) {
        elf_program_header_t phdr = elf.phdrs[i];
        load_phdr(elf.header, &phdr, load_base, cr3);
    }

    return (elf.header->e_entry + load_base);
}