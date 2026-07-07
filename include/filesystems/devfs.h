#ifndef DEVFS_H
#define DEVFS_H

#include "filesystems/vfs.h"
#include "userspace/scheduler.h"

typedef enum {
    DEV_STDIN,
    DEV_STDOUT,
    DEV_STDERR,
    DEV_STATIC,
    DEV_ABOUT
} dev_type_t;

typedef struct about_data_t {
    char                    os_name[256];
    char                    os_version[64];
    char                    os_arch[64];

    int                     mem_size;
    int                     mem_used;
} about_data_t;

typedef struct stdin_data_t {
    char                    kbd_buf[256];
} stdin_data_t;

typedef struct devfs_node_t {
    char                    name[256];
    inode_type_t            type;
    inode_t                 inode;
    dev_type_t              dev_type;
    wait_queue_t            waiters;
    uint64_t                size;

    void                    *extra_data;

    struct devfs_node_t*    parent;
    struct devfs_node_t*    child;
    struct devfs_node_t*    next;
} devfs_node_t;

extern inode_t *g_stdin;
extern inode_t *g_stdout;
extern inode_t *g_stderr;

superblock_t *devfs_init();

#endif // DEVFS_H