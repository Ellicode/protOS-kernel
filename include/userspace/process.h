#include "filesystems/vfs.h"

#ifndef PROCESS_H
#define PROCESS_H

#define USER_STACK_BASE 0x7FFFF000ULL
#define USER_STACK_SIZE 0x100000ULL

#define USER_FRAMEBUFFER_BASE 0x6000000000000ULL

#define PROCESS_MAX_FDS 255


enum {
    PROCESS_OTHER = 0,
    PROCESS_APPLICATION = 1,
    PROCESS_DAEMON = 2,
    PROCESS_SCRIPT = 3,
};
typedef uint64_t process_type_t;

typedef struct process_t {
    uint64_t pid;
    char pname[255];
    process_type_t ptype;

    void *kernel_stack;
    uint64_t cr3;
    inode_t *cwd;
    file_descriptor_t *fd_table[PROCESS_MAX_FDS];

    struct process_t *next;
    struct process_t *prev;
} process_t;

extern process_t *g_active_processes;

int create_process(char *elf_path);

#endif // PROCESS_H