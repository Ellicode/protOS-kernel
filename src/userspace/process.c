#include <stdlib.h>

#include "filesystems/vfs.h"
#include "memory/heap.h"
#include "memory/vmm.h"
#include "userspace/elf.h"
#include "userspace/scheduler.h"
#include "userspace/userspace.h"
#include "gdt.h"
#include "utils/utils.h"
#include "utils/linked_lists.h"
#include "debug/logger.h"
#include "debug/errors.h"
#include "string.h"
#include "globals.h"

#include "userspace/process.h"

uint64_t curr_pid = 0;
process_t *g_active_processes = NULL;

int create_process(char *elf_path, uint8_t is_root, int *pid, char argv[16][64]) {
    file_descriptor_t *f = vfs_open(rootfs->root, elf_path, FD_READ);

    if (f == NULL) {
        // k_assert(PROTO_ERR_FILE_NOT_FOUND);
        return PROTO_ERR_FILE_NOT_FOUND;
    }

    size_t size = f->inode->size;
    char *buffer = k_alloc(size);
    if (buffer == NULL) {
        // k_assert(PROTO_ERR_OUT_OF_MEMORY);
        return PROTO_ERR_OUT_OF_MEMORY;
    }

    uint64_t ret = vfs_read(f, size, buffer);
    if (ret != 0) {
        // k_assert(PROTO_ERR_ELF_CORRUPTED);
        return PROTO_ERR_ELF_CORRUPTED;
    }

    vfs_close(f);

    uint64_t pml4 = create_user_pml4();
    uint64_t physical_fb = (uint64_t)g_vga_active_framebuffer->address - g_lim_hhdm->offset;
    size_t fb_size = PAGE_ROUND(g_vga_active_framebuffer->width * g_vga_active_framebuffer->height);
    
    vmm_map_phys_range(
        pml4, 
        USER_FRAMEBUFFER_BASE, 
        physical_fb, 
        fb_size, 
        F_USER | F_WRITE | F_PCD 
    );
    
    uint64_t entry = elf_load(buffer, size, pml4);
    k_free(buffer);

    if (entry == 1) {
        // k_assert(PROTO_ERR_ELF_CANNOT_LOAD);
        return PROTO_ERR_ELF_CANNOT_LOAD;
    }
    
    process_t *process = k_alloc(sizeof(process_t));

    process->pid = ++curr_pid;
    if (g_current_thread->process != NULL) {
        process->cwd = g_current_thread->process->cwd;
    } else {
        process->cwd = rootfs->root;
    }
    process->ptype = PROCESS_OTHER;
    strcpy(process->pname, elf_path);
    process->kernel_stack = k_alloc(KERNEL_STACK_SIZE);
    process->cr3 = pml4;

    if (pid != NULL) {
        *pid = process->pid;
    }
    
    if (is_root) {
        file_descriptor_t *stdin = vfs_open(process->cwd, "/Devices/stdin", FD_READ);
        process->fd_table[0] = stdin;
        
        file_descriptor_t *stdout = vfs_open(process->cwd, "/Devices/stdout", FD_WRITE);
        process->fd_table[1] = stdout;

        file_descriptor_t *stderr = vfs_open(process->cwd, "/Devices/stderr", FD_WRITE);
        process->fd_table[2] = stderr;
    } else {
        if (g_current_thread->process != NULL) {
            memcpy(process->fd_table, g_current_thread->process->fd_table, sizeof(g_current_thread->process->fd_table));
        }
    }

    tss.rsp0 = (uint64_t)process->kernel_stack + KERNEL_STACK_SIZE;

    LL_APPEND(process, g_active_processes);

    thread_t *thread = create_user_thread(process, entry);

    if (argv != NULL) {
        uint64_t user_rsp = thread->context.rsp;

        user_rsp -= 64 * 16;

        uint64_t phys = vmm_virt_to_phys(process->cr3, user_rsp);
        void *dst = (void *)(phys + g_lim_hhdm->offset);

        memcpy(dst, argv, 64 * 16);

        thread->context.rsp = user_rsp;
        thread->context.rdi = user_rsp;
    }
    return PROTO_OK;
}