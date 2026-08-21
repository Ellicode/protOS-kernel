#ifndef DEVFS_H
#define DEVFS_H

#include "filesystems/vfs.h"
#include "userspace/scheduler.h"
#include "utils/circ_buffer.h"

typedef enum {
    DEV_TTY,
    DEV_PTY_MASTER,
    DEV_PTY_SLAVE,
    DEV_ABOUT
} dev_type_t;

typedef struct about_data_t {
    char                    os_name[256];
    char                    os_version[64];
    char                    os_arch[64];

    int                     mem_size;
    int                     mem_used;
} about_data_t;

typedef struct tty_data_t {
    char buffer[4096];

    wait_queue_t read_waiters;

    size_t (*write)(struct tty_data_t *tty, const char *buffer);
    size_t (*read)(struct tty_data_t *tty, char *buffer);

    void *private_data;
} tty_data_t;

typedef struct pty_data_t {
    int id;
    
    circ_buffer_t input_buffer;
    circ_buffer_t output_buffer;

    wait_queue_t read_waiters;
    wait_queue_t master_read_waiters;
    
    file_descriptor_t *master_fd;
    file_descriptor_t *slave_fd;

    void *private_data;
} pty_data_t;

typedef struct pty_master_data_t {
    pty_data_t *pty;
    int did_read;
} pty_master_data_t;


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

extern tty_data_t *g_console;

superblock_t *devfs_init();

#endif // DEVFS_H
