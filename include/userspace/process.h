#ifndef PROCESS_H
#define PROCESS_H

#include "filesystems/vfs.h"
#include "userspace/ipc.h"

#define USER_STACK_BASE         0x0000300000000000ULL
#define USER_STACK_SIZE         0x100000ULL

#define KERNEL_STACK_SIZE       16384

#define USER_FRAMEBUFFER_BASE   0x0000200000000000ULL

#define USER_HEAP_MIN_SIZE      0x0000000000400000ULL // 4 MiB
#define USER_HEAP_MAX_SIZE      0x00000FFFFFFFFFFFULL // A lot of GiB
#define USER_HEAP_BASE          0x0000100000000000ULL

#define PROCESS_MAX_FDS 255

typedef enum{
    PROCESS_OTHER,
    PROCESS_APPLICATION,
    PROCESS_DAEMON,
    PROCESS_SCRIPT,
} process_type_t;

typedef struct process_t {
    uint64_t            pid;
    char                pname[255];
    process_type_t      ptype;

    void                *kernel_stack;
    uint64_t            cr3;
    inode_t             *cwd;
    file_descriptor_t   *fd_table[PROCESS_MAX_FDS];

    ipc_queue_t         msg_queue;

    struct process_t    *next;
    struct process_t    *prev;
} process_t;

extern process_t *g_active_processes;

int create_process(char *elf_path, uint8_t is_root, int *pid, char argv[16][64]);

#endif // PROCESS_H