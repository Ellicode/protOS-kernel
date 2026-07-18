#include <stdlib.h>

#include "filesystems/vfs.h"
#include "memory/heap.h"
#include "memory/vmm.h"
#include "memory/pat.h"
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

int create_process(char *elf_path, uint8_t is_root, int *pid, char argv[16][64], int argc) {
    file_descriptor_t *f = vfs_open(rootfs->root, elf_path, FD_READ);

    if (f == NULL) {
        k_assert(PROTO_ERR_FILE_NOT_FOUND);
        return PROTO_ERR_FILE_NOT_FOUND;
    }

    size_t size = f->inode->size;
    char *buffer = k_alloc(size);
    if (buffer == NULL) {
        vfs_close(f);
        k_assert(PROTO_ERR_OUT_OF_MEMORY);
        return PROTO_ERR_OUT_OF_MEMORY;
    }

    uint64_t ret = vfs_read(f, size, buffer);
    if (ret < PROTO_OK) {
        vfs_close(f);
        k_assert(PROTO_ERR_ELF_CORRUPTED);
        return PROTO_ERR_ELF_CORRUPTED;
    }

    vfs_close(f);

    uint64_t pml4 = create_user_pml4();

    // map heap
    vmm_map_range(
        pml4,
        USER_HEAP_BASE,
        USER_HEAP_MIN_SIZE,
        F_USER | F_WRITE
    );
    
    uint64_t entry = elf_load(buffer, size, pml4);
    k_free(buffer);

    if (entry == 1) {
        k_assert(PROTO_ERR_ELF_CANNOT_LOAD);
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
    process->msg_queue.waiters = k_alloc(sizeof(wait_queue_t));

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
            memcpy((void *)process->fd_table, (void *)g_current_thread->process->fd_table, sizeof(g_current_thread->process->fd_table));
        }
    }

    tss.rsp0 = (uint64_t)process->kernel_stack + KERNEL_STACK_SIZE;

    LL_APPEND(process, g_active_processes);

    thread_t *thread = create_user_thread(process, entry);

    if (argv != NULL) {
        uint64_t user_rsp = thread->context.rsp;

        user_rsp -= (uint64_t)64 * 16;
        uint64_t phys = vmm_virt_to_phys(process->cr3, user_rsp);
        void *dst = (void *)(phys + g_lim_hhdm->offset);
        memcpy(dst, argv, (uint64_t)64 * 16);
        thread->context.rdi = user_rsp;

        user_rsp -= 8;
        thread->context.rsi = argc;

        user_rsp &= ~0xFULL;
        user_rsp -= 8;
        thread->context.rsp = user_rsp;
    }
    return PROTO_OK;
}