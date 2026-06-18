#include <stdint.h>

#include "memory/heap.h"
#include "graphics/console.h"
#include "utils/linked_lists.h"
#include "string.h"

#include "filesystems/devfs.h"

superblock_t *devfs_superblock;
devfs_node_t *devfs_root;

inode_t *g_stdin;
inode_t *g_stdout;
inode_t *g_stderr;

int devfs_lookup(inode_t *dir, char *name, inode_t **result) {
    devfs_node_t *node = dir->fs_data;
    devfs_node_t *current = node->child;

    if (strcmp(name,  ".") == 0) { 
        *result = dir;
        return 0;
    }
    if (strcmp(name,  "..") == 0) { 
        if (node->parent) {
            current = node->parent;
            *result = &current->inode;
            return 0;
        } else {
            *result = NULL;
            return 1;
        }
    }

    while (current != NULL && strcmp(current->name, name) != 0) {
        current = current->next;
    }

    if (current == NULL) {
        *result = NULL;
        return -1;
    }

    *result = &current->inode;
    return 0;
}

int devfs_create(inode_t *dir, char *name, inode_t **result) {
    if (dir == NULL) {
        return -1;
    }

    inode_t *res;

    if (devfs_lookup(dir, name, &res) == 0) {
        return -1; // File already exists
    }

    devfs_node_t *parent_node = dir->fs_data;
    devfs_node_t *node = k_alloc(sizeof(devfs_node_t));
    
    node->type              = INODE_FILE;
    node->next              = parent_node->child;
    strcpy(node->name, name);
    node->parent            = parent_node;
    node->inode.type        = INODE_FILE;
    node->inode.fs_data     = node;
    node->inode.parent_sb   = devfs_superblock;

    parent_node->child      = node;

    (*result) = &node->inode;

    return 0;
}

int devfs_create_dir(inode_t *dir, char *name, inode_t **result) {
    if (dir == NULL) {
        return -1;
    }

    inode_t *res;

    if (devfs_lookup(dir, name, &res) == 0) {
        return -1; // Folder already exists
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

    parent_node->child      = node;

    (*result) = &node->inode;

    return 0;
}

int devfs_read(inode_t *inode, uint64_t size, void *buffer) {
    devfs_node_t *node = inode->fs_data;
    stdin_data_t *stdin_data = node->extra_data;

    print_f("percent dihhh: %d\n", node->dev_type);

    switch (node->dev_type)
    {
        case DEV_STDIN:
            while (stdin_data->kbd_read == stdin_data->kbd_write) {
                queue_sleep(&node->waiters, g_current_thread);
            }

            print_f("after eep!\n");

            uint64_t i = 0;
            while (i < size && stdin_data->kbd_read != stdin_data->kbd_write) {
                ((char*)buffer)[i++] = stdin_data->kbd_buf[stdin_data->kbd_read++];
            }
            return i;

            break;
        default: // No match
            return 1;
    }

    return 0;
}

int devfs_write(inode_t *inode, uint64_t size, const void *buffer) {
    devfs_node_t *node = inode->fs_data;

    switch (node->dev_type)
    {
        case DEV_STDOUT:
            print(buffer);
            break;
        case DEV_STDERR:
            set_color(PROTO_RED);
            print(buffer);
            set_color(PROTO_WHITE);
            break;
        default: // No match
            print_f("no match for %d", node->dev_type);
            return 1;
    }

    return 0;
}

superblock_t *devfs_init() {
    print_f("init devfs\n");

    devfs_root                          = k_alloc(sizeof(devfs_node_t));
    devfs_root->type                    = INODE_FOLDER;
    devfs_root->inode.type              = INODE_FOLDER;
    devfs_root->inode.fs_data           = devfs_root;

    devfs_superblock                    = k_alloc(sizeof(superblock_t));
    devfs_superblock->ops               = k_alloc(sizeof(vfs_ops_t));
    devfs_superblock->ops->lookup       = devfs_lookup;
    devfs_superblock->ops->create       = devfs_create;
    devfs_superblock->ops->create_dir   = devfs_create_dir;
    devfs_superblock->ops->write        = devfs_write;
    devfs_superblock->ops->read         = devfs_read;
    devfs_superblock->fs_type           = FS_DEVFS;
    devfs_superblock->root              = &devfs_root->inode;

    devfs_root->inode.parent_sb         = devfs_superblock;

    devfs_create(devfs_superblock->root, "stdin", &g_stdin);
    devfs_node_t *stdin_data = g_stdin->fs_data;
    stdin_data->dev_type = DEV_STDIN;
    stdin_data->extra_data = k_alloc(sizeof(stdin_data_t));

    print_f("init stdin\n");

    devfs_create(devfs_superblock->root, "stdout", &g_stdout);
    devfs_node_t *stdout_data = g_stdout->fs_data;
    stdout_data->dev_type = DEV_STDOUT;

    print_f("init stdout\n");

    devfs_create(devfs_superblock->root, "stderr", &g_stderr);
    devfs_node_t *stderr_data = g_stderr->fs_data;
    stderr_data->dev_type = DEV_STDERR;

    print_f("init stderr\n");

    return devfs_superblock;
}