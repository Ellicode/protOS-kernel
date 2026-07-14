#include "userspace/process.h"
#include "debug/logger.h"
#include "debug/errors.h"
#include "filesystems/vfs.h"
#include "graphics/console.h"
#include "memory/heap.h"
#include "globals.h"
#include "string.h"
#include "userspace/scheduler.h"
#include "userspace/user_fb.h"
#include "memory/vmm.h"
#include "utils/linked_lists.h"
#include "userspace/ipc.h"

#include "userspace/syscalls.h"

void __not_implemented () {}

void sys_exit()
{
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return; 
    }
    
    thread_t *thread = g_threads;

    while (thread)
    {
        if (thread->state == THREAD_SLEEPING &&
            thread->waiting_for == g_current_thread->process->pid)
        {
            thread->state = THREAD_RUNNING;
            thread->waiting_for = -1;
        }

        thread = thread->next;
    }

    g_current_thread->state = THREAD_STOPPED;
    scheduler_yield();
}

int sys_open(char *path, char *flags) {
    if (!g_current_thread || !g_current_thread->process || !g_current_thread->process->cwd) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
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
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }    

    file_descriptor_t *desc = vfs_open(proc->cwd, path, flags_int);
    if (desc == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }

    proc->fd_table[descriptor_idx] = desc;
    return descriptor_idx;
}

int sys_read(uint64_t fd, size_t size, void *buffer) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        // k_assert(PROTO_ERR_INVALID_FD);
        return PROTO_ERR_INVALID_FD; 
    }
    return vfs_read(fd_pointer, size, buffer);
}

int sys_read_dir(uint64_t fd, dentry_t *entries, int *num_entries) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        // k_assert(PROTO_ERR_INVALID_FD);
        return PROTO_ERR_INVALID_FD; 
    }
    return vfs_read_dir(fd_pointer, entries, num_entries);
}

int sys_write(uint64_t fd, size_t size, const void *buffer) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        // k_assert(PROTO_ERR_INVALID_FD);
        return PROTO_ERR_INVALID_FD; 
    }
    return vfs_write(fd_pointer, size, buffer);
}

int sys_stat(uint64_t fd, dentry_t *buffer) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        // k_assert(PROTO_ERR_INVALID_FD);
        return PROTO_ERR_INVALID_FD; 
    }
    return vfs_stat(fd_pointer, buffer);
}

int sys_close(uint64_t fd) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    if (fd >= PROCESS_MAX_FDS) {
        // k_assert(PROTO_ERR_MAX_FD_REACHED);
        return PROTO_ERR_MAX_FD_REACHED;
    }

    file_descriptor_t *fd_pointer = proc->fd_table[fd];

    if (fd_pointer == NULL) {
        // k_assert(PROTO_ERR_INVALID_FD);
        return PROTO_ERR_INVALID_FD; 
    }

    return vfs_close(fd_pointer);
}

int sys_create_process(char *path, char argv[16][64], int argc) {    
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    g_current_thread->state = THREAD_SLEEPING;
    int pid;
    int ret = create_process(path, 0, &pid, argv, argc);
    g_current_thread->waiting_for = pid;

    return ret;
}

int sys_chdir(char *path) {
    if (!g_current_thread || !g_current_thread->process) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;

    inode_t *newcwd = vfs_lookup(proc->cwd, path);
    if (newcwd == NULL) {
        // k_assert(PROTO_ERR_FILE_NOT_FOUND);
        return PROTO_ERR_FILE_NOT_FOUND;
    }

    if (newcwd->type != INODE_FOLDER) {
        return PROTO_ERR_NOT_A_DIRECTORY;
    }

    proc->cwd = newcwd;
    return PROTO_OK;
}

int sys_getcwd(char *user_buf, size_t user_buf_size) {
    if (!g_current_thread || !g_current_thread->process || !user_buf) {
        return PROTO_ERR_UNKNOWN; 
    }

    process_t *proc = g_current_thread->process;
    inode_t *cwd = proc->cwd;
    
    char path_accumulator[512];
    path_accumulator[0] = '\0';

    dentry_t statbuf;

    while (cwd != NULL) {
        inode_t *parent = NULL;
        parent = vfs_lookup(cwd, "..");
        if (parent == NULL) {
            break;
        }

        if (cwd->id == parent->id) {
            break; 
        }

        file_descriptor_t *fd = vfs_open(cwd, ".", 0x0);
        if (fd) {
            if (vfs_stat(fd, &statbuf) == PROTO_OK) {
                char temp[512];
                snprintf(temp, sizeof(temp), "/%s%s", statbuf.name, path_accumulator);
                strncpy(path_accumulator, temp, sizeof(path_accumulator));
            }
            vfs_close(fd); // Proper resource teardown
        }

        cwd = parent;
    }

    if (strlen(path_accumulator) == 0) {
        strncpy(path_accumulator, "/", sizeof(path_accumulator));
    }

    size_t final_len = strlen(path_accumulator) + 1;
    if (final_len > user_buf_size) {
        return PROTO_ERR_OUT_OF_MEMORY; // ERANGE
    }
    memcpy(user_buf, path_accumulator, final_len);

    return PROTO_OK;
}

int sys_wait_for_process(int pid) {
    if (!g_current_thread) {
        return PROTO_ERR_UNKNOWN; 
    }

    g_current_thread->state = THREAD_SLEEPING;
    g_current_thread->waiting_for = pid;

    print_f("%s -> (state=%d, waiting=%d)", g_current_thread->process->pname, g_current_thread->state, g_current_thread->waiting_for);

    scheduler_yield();
    
    return PROTO_OK;
}

int sys_send(int pid, ipc_syscall_payload *msg) {
    if (msg == NULL) { return PROTO_ERR_INVALID_ARGUMENT; }

    ipc_syscall_payload *payload = k_alloc(sizeof(ipc_syscall_payload));

    strncpy(payload->message, msg->message, 255);
    payload->data = k_alloc(msg->size);
    memcpy(payload->data, msg->data, msg->size);
    payload->size = msg->size;

    return ipc_send(pid, payload->message, payload->data, payload->size);
}

int sys_getpid() {
    if (g_current_thread == NULL || g_current_thread->process == NULL) { return -1; }
    return g_current_thread->process->pid;
}

void* syscall_handlers[] = {
    [SYS_EXIT]              = sys_exit,
    [SYS_READ]              = sys_read,
    [SYS_WRITE]             = sys_write,
    [SYS_OPEN]              = sys_open,
    [SYS_CLOSE]             = sys_close,
    [SYS_STAT]              = sys_stat,
    [SYS_READ_DIR]          = sys_read_dir,
    [SYS_CREATE_PROCESS]    = sys_create_process,
    [SYS_FETCH_FB]          = sys_fetch_fb,
    [SYS_CHDIR]             = sys_chdir,
    [SYS_GETCWD]            = sys_getcwd,
    [SYS_WAIT_FOR_PROCESS]  = sys_wait_for_process,
    [SYS_SEND]              = sys_send,
    [SYS_RECIEVE]           = ipc_recieve,
    [SYS_GETPID]            = sys_getpid,
    [SYS_PANIC]             = panic,
};

void syscall_handler(idt_frame_t *frame) {
    uint64_t syscall_id = frame->rax;

    if (syscall_id > NUM_SYSCALLS - 1) {
        // k_assert(PROTO_ERR_SYSCALL_OUT_OF_BOUNDS);
        frame->rax = PROTO_ERR_SYSCALL_OUT_OF_BOUNDS;
        return;
    }

    syscall_t handler = (syscall_t)syscall_handlers[syscall_id];
    uint64_t ret = handler(
        frame->rdi,
        frame->rsi,
        frame->rdx,
        frame->r10,
        frame->r8,
        frame->r9
    );

    frame->rax = ret;
}