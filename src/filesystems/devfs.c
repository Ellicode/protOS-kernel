#include <stdint.h>

#include "memory/heap.h"
#include "graphics/console.h"
#include "utils/linked_lists.h"
#include "memory/freelist_pmm.h"
#include "debug/errors.h"
#include "globals.h"
#include "string.h"

#include "filesystems/devfs.h"

superblock_t *devfs_superblock;
uint64_t devfs_next_id = 1;
devfs_node_t *devfs_root;

inode_t *g_stdin;
inode_t *g_stdout;
inode_t *g_stderr;

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
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    inode_t *res;

    devfs_node_t *parent_node = dir->fs_data;
    devfs_node_t *node = k_alloc(sizeof(devfs_node_t));
    
    node->type              = INODE_FILE;
    node->next              = parent_node->child;
    strcpy(node->name, name);
    node->parent            = parent_node;
    node->inode.type        = INODE_FILE;
    node->inode.fs_data     = node;
    node->inode.parent_sb   = devfs_superblock;

    node->inode.id          = devfs_next_id++;
    parent_node->child      = node;
    (*result) = &node->inode;

    return PROTO_OK;
}

int devfs_create_dir(inode_t *dir, char *name, inode_t **result) {
    if (dir == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    inode_t *res;

    if (devfs_lookup(dir, name, &res) == PROTO_OK) {
        // k_assert(PROTO_ERR_ALREADY_EXISTS);
        return PROTO_ERR_ALREADY_EXISTS; // Folder already exists
    }

    devfs_node_t *parent_node = dir->fs_data;
    devfs_node_t *node = k_alloc(sizeof(devfs_node_t));
    
    node->type              = INODE_FOLDER;
    node->next              = parent_node->child;
    strcpy(node->name, name);
    node->parent            = parent_node;
    node->inode.type        = INODE_FOLDER;
    node->inode.fs_data     = node;
    node->inode.parent_sb   = devfs_superblock;

    node->inode.id          = devfs_next_id++;
    parent_node->child      = node;
    (*result) = &node->inode;

    return PROTO_OK;
}

int devfs_read(inode_t *inode, uint64_t size, void *buffer) {
    if (inode == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    devfs_node_t *node = inode->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }

    switch (node->dev_type)
    {
        case DEV_STDIN:
            stdin_data_t *stdin_data = node->extra_data;
            queue_sleep(&node->waiters, g_current_thread);
            strcpy(buffer, stdin_data->kbd_buf);
            memset(stdin_data->kbd_buf, 0, sizeof(stdin_data->kbd_buf)); // clean junk ew
            break;
        case DEV_STATIC:
            memcpy(buffer, node->extra_data, node->size);
            break;
        case DEV_ABOUT:
            int memsz = getmemsz();
            int memused = 0;

            about_data_t about_data = (about_data_t) {
                .os_name        = PROTO_NAME,
                .os_version     = PROTO_VERSION,
                .os_arch        = PROTO_ARCH,

                .mem_size       = memsz,
                .mem_used       = memused
            };
            memcpy(buffer, &about_data, sizeof(about_data_t));
            break;
        default: // No match
            // k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
            return PROTO_ERR_FILE_UNSUPPORTED_OP;
    }

    return PROTO_OK;
}

int devfs_stat(inode_t *inode, dentry_t *buffer) {
    if (inode == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }
    devfs_node_t *node = (devfs_node_t *)inode->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }
    strcpy(buffer->name, node->name);
    buffer->inode = inode;
    buffer->size = node->size;

    return PROTO_OK;
}

int devfs_write(inode_t *inode, uint64_t size, const void *buffer) {
    if (inode == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }
    devfs_node_t *node = inode->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }

    switch (node->dev_type)
    {
        case DEV_STDOUT:
            print(buffer);
            break;
        case DEV_STDERR:
            set_color(PROTO_RED, PROTO_BG);
            print(buffer);
            set_color(PROTO_WHITE, PROTO_BG);
            break;
        default: // No match
            // k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
            return PROTO_ERR_FILE_UNSUPPORTED_OP;
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
    devfs_superblock->ops->write        = devfs_write;
    devfs_superblock->ops->read         = devfs_read;
    devfs_superblock->ops->stat         = devfs_stat;
    devfs_superblock->fs_type           = FS_DEVFS;
    devfs_superblock->root              = &devfs_root->inode;

    devfs_root->inode.parent_sb         = devfs_superblock;

    devfs_create(devfs_superblock->root, "stdin", &g_stdin);
    devfs_node_t *stdin_data = g_stdin->fs_data;
    stdin_data->dev_type = DEV_STDIN;
    stdin_data->waiters.lock = k_alloc(sizeof(ticketlock_t));
    stdin_data->extra_data = k_alloc(sizeof(stdin_data_t));
    stdin_data->size = 0;

    devfs_create(devfs_superblock->root, "stdout", &g_stdout);
    devfs_node_t *stdout_data = g_stdout->fs_data;
    stdout_data->dev_type = DEV_STDOUT;
    stdout_data->size = 0;

    devfs_create(devfs_superblock->root, "stderr", &g_stderr);
    devfs_node_t *stderr_data = g_stderr->fs_data;
    stderr_data->dev_type = DEV_STDERR;
    stderr_data->size = 0;

    inode_t *about;
    devfs_create(devfs_superblock->root, "about", &about);
    devfs_node_t *about_data = about->fs_data;
    about_data->dev_type = DEV_ABOUT;
    about_data->size = 0;

    return devfs_superblock;
}