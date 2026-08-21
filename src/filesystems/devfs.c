#include <stdint.h>

#include "memory/heap.h"
#include "graphics/console.h"
#include "utils/linked_lists.h"
#include "memory/freelist_pmm.h"
#include "debug/errors.h"
#include "globals.h"
#include "string.h"
#include "filesystems/vfs.h"
#include "filesystems/aio.h"

#include "filesystems/devfs.h"

superblock_t *devfs_superblock;
int devfs_next_id = 1;
int pty_next_id = 1;
devfs_node_t *devfs_root;
inode_t *pty_dir;

tty_data_t *g_console;

int devfs_lookup(inode_t *dir, char *name, inode_t **result) {
    devfs_node_t *node = dir->fs_data;
    devfs_node_t *current = node->child;

    if (strcmp(name,  ".") == 0) { 
        *result = dir;
        return PROTO_OK;
    }
    if (strcmp(name,  "..") == 0) { 
        if (node->parent) {
            current = node->parent;
            *result = &current->inode;
            return PROTO_OK;
        } else {
            *result = dir; 
            return PROTO_OK;
        }
    }

    while (current != NULL && strcmp(current->name, name) != 0) {
        current = current->next;
    }

    if (current == NULL) {
        *result = NULL;
        // k_assert(PROTO_ERR_FILE_NOT_FOUND);
        return PROTO_ERR_FILE_NOT_FOUND;
    }

    *result = &current->inode;
    return PROTO_OK;
}

int devfs_create(inode_t *dir, char *name, inode_t **result) {
    if (dir == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    devfs_node_t *parent_node = dir->fs_data;
    devfs_node_t *node = k_alloc(sizeof(devfs_node_t));
    strcpy(node->name, name);
    node->type              = INODE_FILE;
    node->parent            = parent_node;
    node->inode.type        = INODE_FILE;
    node->inode.fs_data     = node;
    node->inode.parent_sb   = devfs_superblock;
    node->inode.id          = devfs_next_id++;

    node->next              = parent_node->child;
    parent_node->child      = node;
    if (result != NULL) {
        (*result) = &node->inode;
    }
    
    return PROTO_OK;
}

int devfs_create_dir(inode_t *dir, char *name, inode_t **result) {
    if (dir == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    inode_t *res;

    if (devfs_lookup(dir, name, &res) == PROTO_OK) {
        k_assert(PROTO_ERR_ALREADY_EXISTS);
        return PROTO_ERR_ALREADY_EXISTS; // Folder already exists
    }

    devfs_node_t *parent_node = dir->fs_data;
    devfs_node_t *node = k_alloc(sizeof(devfs_node_t));
    strcpy(node->name, name);
    node->type              = INODE_FOLDER;
    node->parent            = parent_node;
    node->inode.type        = INODE_FOLDER;
    node->inode.fs_data     = node;
    node->inode.parent_sb   = devfs_superblock;
    node->inode.id          = devfs_next_id++;
    
    node->next              = parent_node->child;
    parent_node->child      = node;

    if (result != NULL) {
        (*result) = &node->inode;
    }

    return PROTO_OK;
}

int devfs_read_wait(file_descriptor_t *fd, wait_queue_t *wq) {
    if (fd->flags & FD_ASYNC) {
        aio_enqueue(fd, AIO_OP_READ);
        return -PROTO_ERR_WOULD_BLOCK;
    } else {
        queue_sleep(wq, g_current_thread);
        return PROTO_OK;
    }
}

int devfs_wake_all(file_descriptor_t *fd, wait_queue_t *wq) {
    if (fd != NULL && (fd->flags & FD_ASYNC)) {
        return aio_notify(fd, AIO_OP_READ);
    } else {
        queue_wake_all(wq);
        return PROTO_OK;
    }
}

int devfs_read(file_descriptor_t *fd, uint64_t size, void *buffer) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return -PROTO_ERR_INVALID_ARGUMENT;
    }

    devfs_node_t *node = inode->fs_data;
    if (node == NULL) {
        k_assert(PROTO_ERR_UNKNOWN);
        return -PROTO_ERR_UNKNOWN;
    }

    switch (node->dev_type)
    {
        case DEV_PTY_MASTER:
            pty_master_data_t *master_data = fd->extra_data;
            pty_data_t *pty_slave = master_data->pty;

            if (master_data->did_read == 0) {
                master_data->did_read = 1;
                snprintf(buffer, 12, "/dev/pty/%d", pty_slave->id);
                return strlen(buffer);
            } else {
                if (circ_buffer_is_empty(&pty_slave->output_buffer)) {
                    int res = devfs_read_wait(fd, &pty_slave->master_read_waiters);
                    if (res != PROTO_OK) { return res; }
                }
                return circ_buffer_read(&pty_slave->output_buffer, buffer, size);
            }
        case DEV_TTY:
            tty_data_t *tty_data = (tty_data_t *)node->extra_data;
            if (tty_data == NULL) {
                k_assert(PROTO_ERR_UNKNOWN);
                return -PROTO_ERR_UNKNOWN;
            }

            return tty_data->read(tty_data, buffer);
        case DEV_PTY_SLAVE:
            pty_data_t *pty_data = (pty_data_t *)node->extra_data;
            if (pty_data == NULL) {
                k_assert(PROTO_ERR_UNKNOWN);
                return -PROTO_ERR_UNKNOWN;
            }
            if (circ_buffer_is_empty(&pty_data->input_buffer)) {
                int res = devfs_read_wait(fd, &pty_data->read_waiters);
                if (res != PROTO_OK) { return res; }
            }

            return circ_buffer_read(&pty_data->input_buffer, buffer, size);
        case DEV_ABOUT:
            int memsz = getmemsz();
            int memused = getmemused();

            about_data_t about_data = (about_data_t) {
                .os_name        = PROTO_NAME,
                .os_version     = PROTO_VERSION,
                .os_arch        = PROTO_ARCH,

                .mem_size       = memsz,
                .mem_used       = memused
            };
            memcpy(buffer, &about_data, sizeof(about_data_t));
            return sizeof(about_data_t);
        default: // No match
            k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
            return -PROTO_ERR_FILE_UNSUPPORTED_OP;
    }

    return -PROTO_ERR_UNKNOWN;
}

int devfs_stat(file_descriptor_t *fd, dentry_t *buffer) {
    inode_t *inode = fd->inode;

    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    devfs_node_t *node = inode->fs_data;
    if (node == NULL) {
        k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }
    strcpy(buffer->name, node->name);
    buffer->inode = inode;
    buffer->size = node->size;

    return PROTO_OK;
}

int devfs_write(file_descriptor_t *fd, uint64_t size, const void *buffer) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return -PROTO_ERR_INVALID_ARGUMENT;
    }
    devfs_node_t *node = inode->fs_data;
    if (node == NULL) {
        k_assert(PROTO_ERR_UNKNOWN);
        return -PROTO_ERR_UNKNOWN;
    }

    switch (node->dev_type)
    {
        case DEV_TTY:
            tty_data_t *tty_data = (tty_data_t *)node->extra_data;
            if (tty_data == NULL) {
                k_assert(PROTO_ERR_UNKNOWN);
                return -PROTO_ERR_UNKNOWN;
            }
            
            return tty_data->write(tty_data, buffer);
        case DEV_PTY_SLAVE:
            pty_data_t *pty_data = (pty_data_t *)node->extra_data;
            if (pty_data == NULL) {
                k_assert(PROTO_ERR_UNKNOWN);
                return -PROTO_ERR_UNKNOWN;
            }
            int sz = circ_buffer_write(&pty_data->output_buffer, buffer, size);
            devfs_wake_all(pty_data->master_fd, &pty_data->master_read_waiters);
            return sz;
        case DEV_PTY_MASTER:
            pty_master_data_t *master_data = fd->extra_data;
            pty_data_t *pty_slave = master_data->pty;

            int sz2 = circ_buffer_write(&pty_slave->input_buffer, buffer, size);
            devfs_wake_all(pty_slave->slave_fd, &pty_slave->read_waiters);

            return sz2;
        default: // No match
            k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
            return -PROTO_ERR_FILE_UNSUPPORTED_OP;
    }
}

int devfs_read_dir(file_descriptor_t *fd, dentry_t *entries, int *num_entries) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    devfs_node_t *node = (devfs_node_t *)inode->fs_data;
    if (node == NULL) {
        k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }
    if (node->type != INODE_FOLDER) {
        return PROTO_ERR_NOT_A_DIRECTORY;
    }

    *num_entries = 0;
    devfs_node_t *entry = node->child;
    while (entry != NULL) {
        entries[*num_entries].inode = &entry->inode;
        strcpy(entries[*num_entries].name, entry->name);
        entries[*num_entries].size = entry->size;
        entries[*num_entries].type = entry->type;
        
        entry = entry->next;
        *num_entries += 1;
    }

    return PROTO_OK;
}

int devfs_open(file_descriptor_t *fd) {
    devfs_node_t *node = (devfs_node_t *)fd->inode->fs_data;

    switch (node->dev_type)
    {
        case DEV_PTY_MASTER:
            inode_t *pty_inode;
            devfs_create(pty_dir, int_to_string(pty_next_id), &pty_inode);
            devfs_node_t *pty_data = pty_inode->fs_data;

            pty_data->dev_type = DEV_PTY_SLAVE;
            pty_data->extra_data = k_alloc(sizeof(pty_data_t));

            char *ibuffer = k_alloc(4096);
            char *obuffer = k_alloc(4096);

            if (ibuffer == NULL || obuffer == NULL) {
                k_assert(PROTO_ERR_OUT_OF_MEMORY);
                return PROTO_ERR_OUT_OF_MEMORY;
            }
            pty_data_t *pty = (pty_data_t *)pty_data->extra_data;

            circ_buffer_init(&pty->input_buffer, ibuffer, 4096);
            circ_buffer_init(&pty->output_buffer, obuffer, 4096);
            pty->id = pty_next_id;
            pty->master_fd = fd;

            pty_master_data_t *master_data = k_alloc(sizeof(pty_master_data_t));
            master_data->pty = pty_data->extra_data;
            master_data->did_read = 0;

            fd->extra_data = master_data;
            queue_wake_all(&pty->master_read_waiters);

            pty_next_id++;
            return PROTO_OK;
        case DEV_PTY_SLAVE:
            pty_data_t *slave_pty = (pty_data_t *)node->extra_data;
            if (slave_pty != NULL) {
                slave_pty->slave_fd = fd;
            }
            return PROTO_OK;
        default: // No match
            return PROTO_OK;
    }
    return PROTO_OK;
}

superblock_t *devfs_init() {
    devfs_root                          = k_alloc(sizeof(devfs_node_t));
    devfs_root->type                    = INODE_FOLDER;
    devfs_root->inode.type              = INODE_FOLDER;
    devfs_root->inode.fs_data           = devfs_root;
    devfs_root->inode.id                = devfs_next_id++;

    devfs_superblock                    = k_alloc(sizeof(superblock_t));
    devfs_superblock->ops               = k_alloc(sizeof(vfs_ops_t));
    devfs_superblock->ops->lookup       = devfs_lookup;
    devfs_superblock->ops->create       = devfs_create;
    devfs_superblock->ops->create_dir   = devfs_create_dir;
    devfs_superblock->ops->read_dir     = devfs_read_dir;
    devfs_superblock->ops->write        = devfs_write;
    devfs_superblock->ops->read         = devfs_read;
    devfs_superblock->ops->stat         = devfs_stat;
    devfs_superblock->ops->open         = devfs_open;
    devfs_superblock->fs_type           = FS_DEVFS;
    devfs_superblock->root              = &devfs_root->inode;

    devfs_root->inode.parent_sb         = devfs_superblock;

    inode_t *about;
    devfs_create(devfs_superblock->root, "about", &about);
    devfs_node_t *about_data = about->fs_data;
    about_data->dev_type = DEV_ABOUT;
    about_data->size = sizeof(about_data_t);

    inode_t *console;
    devfs_create(devfs_superblock->root, "console", &console);
    devfs_node_t *console_data = console->fs_data;
    console_data->dev_type = DEV_TTY;
    console_data->extra_data = k_alloc(sizeof(tty_data_t));
    ((tty_data_t *)console_data->extra_data)->write = tty_write;
    ((tty_data_t *)console_data->extra_data)->read  = tty_read;
    g_console = console_data->extra_data;

    inode_t *ptymx;
    devfs_create(devfs_superblock->root, "ptymx", &ptymx);
    devfs_node_t *ptymx_data = ptymx->fs_data;
    ptymx_data->dev_type = DEV_PTY_MASTER;

    devfs_create_dir(devfs_superblock->root, "pty", &pty_dir);

    return devfs_superblock;
}
