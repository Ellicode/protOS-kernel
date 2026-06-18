#include "debug/logger.h"
#include "filesystems/vfs.h"
#include "graphics/console.h"
#include "userspace/process.h"
#include "memory/heap.h"
#include "globals.h"
#include "string.h"
#include "userspace/scheduler.h"
#include "userspace/user_fb.h"

#include "userspace/syscalls.h"

void __not_implemented () {}

void _sys_exit() {
    exit_thread(g_current_thread);
}

int sys_open(char *path, char *flags) {
    if (!g_current_thread || !g_current_thread->process || !g_current_thread->process->cwd) {
        return -1; 
    }

    process_t *proc = g_current_thread->process;

    uint8_t flags_int = 0x0;
    uint64_t num_flags = strlen(flags);

    for (uint64_t i = 0; i < num_flags; i++) {
        char flag_str = flags[i];
        switch (flag_str)
        {
            case 'r':
                flags_int |= 1;
                break;
            case 'w':
                flags_int |= 2;
                break;
            default:
                break;
        }
    }

    int descriptor_idx = -1;
    for (int i = 0; i < PROCESS_MAX_FDS; i++) {
        if (proc->fd_table[i] == NULL) {
            descriptor_idx = i;
            break; // Found an empty slot, stop looking
        }
    }

    if (descriptor_idx == -1) {
        k_warning("Process hit maximum file descriptor limit", "sys_open");
        return -1;
    }    

    file_descriptor_t *desc = vfs_open(proc->cwd, path, flags_int);
    if (desc == NULL) {
        return -1;
    }

    proc->fd_table[descriptor_idx] = desc;
    return descriptor_idx;
}

int sys_read(uint64_t fd, size_t size, void *buffer) {
    if (!g_current_thread || !g_current_thread->process) {
        return -1; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        return -1;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        return -1; 
    }
    return vfs_read(fd_pointer, size, buffer);
}

int sys_write(uint64_t fd, size_t size, const void *buffer) {
    if (!g_current_thread || !g_current_thread->process) {
        return -1; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        return -1;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        return -1; 
    }
    return vfs_write(fd_pointer, size, buffer);
}

int sys_close(uint64_t fd) {
    if (!g_current_thread || !g_current_thread->process) {
        return -1; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        return -1;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        return -1; 
    }

    int ret = vfs_close(fd_pointer);

    fd_pointer = NULL;
}

int sys_create_process(char *path) {
    process_t *process = create_process(path, 0);
    if (process == NULL) {
        return -1;
    }
    
    return 0;
}


void* syscall_handlers[] = {
    [SYS_EXIT]              = _sys_exit,
    [SYS_READ]              = sys_read,
    [SYS_WRITE]             = sys_write,
    [SYS_OPEN]              = sys_open,
    [SYS_CLOSE]             = sys_close,
    [SYS_CREATE_PROCESS]    = sys_create_process,
    [SYS_FETCH_FB]          = sys_fetch_fb,
};

void syscall_handler(idt_frame_t *frame) {
    uint64_t syscall_id = frame->rax;

    if (syscall_id > NUM_SYSCALLS - 1) {
        k_warning("Syscall ID out of bounds!", "proto.interrupt_frame.syscall_handler");
        frame->rax = -1; // Commonly returned for errors
        return;
    }

    syscall_t handler = (syscall_t)syscall_handlers[syscall_id];
    uint64_t ret = handler(
        frame->rdi,   // Arg 1
        frame->rsi,   // Arg 2
        frame->rdx,   // Arg 3
        frame->r10,   // Arg 4 (Make sure your idt_frame_t saves r10!)
        frame->r8,    // Arg 5
        frame->r9     // Arg 6
    );

    frame->rax = ret;
}