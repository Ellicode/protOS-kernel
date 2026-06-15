#include "debug/logger.h"
#include "graphics/console.h"
#include "string.h"
#include "globals.h"
#include "memory/heap.h"
#include "filesystems/vfs.h"

#include <stdint.h>

#include "filesystems/tarfs.h"

char *initramfs_start;
uint64_t initramfs_size;
superblock_t *initramfs_superblock;
ustar_node_t *initramfs_root;

ustar_node_t *find_or_create_child(ustar_node_t *parent, char *name, inode_type_t type) {
    ustar_node_t *child = parent->child;
    while (child) {
        if (strcmp(child->name, name) == 0) {
            return child;
        }
        child = child->next;
    }

    ustar_node_t *node = k_alloc(sizeof(ustar_node_t));
    strcpy(node->name, name);
    node->type = type;
    node->parent = parent;
    node->inode.type = type;
    node->inode.fs_data = node;
    node->inode.parent_sb = initramfs_superblock;

    node->next = parent->child;
    parent->child = node;
    return node;
}


int tarfs_lookup(inode_t *dir, char *name, inode_t **result) {
    ustar_node_t *node = dir->fs_data;
    ustar_node_t *current = node->child;

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

int tarfs_read(inode_t *inode, uint64_t size, char *buffer) {
    ustar_node_t *node = (ustar_node_t *)inode->fs_data;
    void *data = (void *)node->offset;
    memcpy(buffer, data, size);

    return 0;
}


superblock_t *tarfs_init() {
    if (g_lim_modules == NULL || g_lim_modules->module_count == 0) {
        k_error("Could not load limine modules\n", "proto.kernel.tarfs_init");
        return NULL;
    }
    
    initramfs_start = g_lim_modules->modules[0]->address;
    initramfs_size = g_lim_modules->modules[0]->size;


    initramfs_root = k_alloc(sizeof(ustar_node_t));
    initramfs_root->type = INODE_FOLDER;
    initramfs_root->inode.type    = INODE_FOLDER;
    initramfs_root->inode.fs_data = initramfs_root;

    initramfs_superblock = k_alloc(sizeof(superblock_t));
    initramfs_superblock->ops = k_alloc(sizeof(vfs_ops_t));
    initramfs_superblock->fs_type = FS_USTAR;
    initramfs_superblock->ops->lookup = tarfs_lookup;
    initramfs_superblock->ops->read = tarfs_read;
    initramfs_superblock->root = &initramfs_root->inode;

    initramfs_root->inode.parent_sb = initramfs_superblock;

    ustar_header_t *header = (ustar_header_t *)initramfs_start;
    unsigned int offset = 0;

    while (offset < initramfs_size) { // For each file in the tar archive
        if (header->file_name[0] == '\0') { break; }

        if (strncmp(header->ustar_magic, USTAR_MAGIC, 5) != 0) {
            k_error("Invalid USTAR header magic\n", "proto.kernel.tarfs_init");
            return NULL;
        }

        size_t file_size = (size_t)strtol(header->file_size, (char **)NULL, 8);

        char *path;
        if (strncmp(header->file_name, USTAR_PATH_PREFIX, USTAR_PATH_PREFIX_SIZE) == 0) {
            path = header->file_name + 2;
        } else {
            path = header->file_name;
        }

        if (strlen(path) == 0) {
            unsigned int aligned_size = ((file_size + 511) / 512) * 512;
            offset += 512 + aligned_size;
            header = (ustar_header_t *)((uintptr_t)initramfs_start + offset);

            continue;
        } else {
            char segments[32][256];
            int seg_count = split_path(path, segments, 32);

            if (seg_count == 0) {
                print("seg_count == 0\n");
                return NULL;
            }

            ustar_node_t *current = initramfs_root;
            for (int i = 0; i < seg_count - 1; i++) {
                current = find_or_create_child(current, segments[i], INODE_FOLDER);
            }

            uint8_t is_dir = (header->type == USTAR_DIRECTORY);
            inode_type_t type = is_dir ? INODE_FOLDER : INODE_FILE;
            ustar_node_t *node = find_or_create_child(current, segments[seg_count - 1], type);

            if (!is_dir) {
                node->offset = (uintptr_t)header + sizeof(ustar_header_t);
                node->size   = file_size;
                node->inode.size = file_size;
            }
        }

        unsigned int aligned_size = ((file_size + 511) / 512) * 512;
        offset += 512 + aligned_size;

        header = (ustar_header_t *)((uintptr_t)initramfs_start + offset);
    }

    return initramfs_superblock;
}
