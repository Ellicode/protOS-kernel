#include <stdint.h>

#include "debug/logger.h"
#include "debug/errors.h"
#include "graphics/console.h"
#include "string.h"
#include "globals.h"
#include "memory/heap.h"
#include "filesystems/vfs.h"

#include "filesystems/tarfs.h"

char *initramfs_start;
uint64_t initramfs_size;
uint64_t initramfs_next_id = 1;
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
    node->inode.id = initramfs_next_id++;

    node->next = parent->child;
    parent->child = node;
    return node;
}


int tarfs_lookup(inode_t *dir, char *name, inode_t **result) {
    ustar_node_t *node = dir->fs_data;
    ustar_node_t *current = node->child;

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

int tarfs_stat(inode_t *inode, dentry_t *buffer) {
    if (inode == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }
    ustar_node_t *node = (ustar_node_t *)inode->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }
    strcpy(buffer->name, node->name);
    buffer->inode = inode;
    buffer->size = node->size;

    return PROTO_OK;
}

int tarfs_read(inode_t *inode, uint64_t size, uint64_t offset, void *buffer) {
    if (inode == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return -1;
    }

    ustar_node_t *node = (ustar_node_t *)inode->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return -1;
    }
    if (offset >= node->size) { return 0; } // EOF
    
    uint64_t remaining = node->size - offset;
    uint64_t to_read = (size < remaining) ? size : remaining;

    void *data = (void *)(node->offset + offset);
    memcpy(buffer, data, to_read);

    return to_read;
}

int tarfs_read_dir(inode_t *dir, dentry_t *entries, int *num_entries) {
    if (dir == NULL) {
        // k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    ustar_node_t *node = (ustar_node_t *)dir->fs_data;
    if (node == NULL) {
        // k_assert(PROTO_ERR_UNKNOWN);
        return PROTO_ERR_UNKNOWN;
    }
    if (node->type != INODE_FOLDER) {
        return PROTO_ERR_NOT_A_DIRECTORY;
    }

    *num_entries = 0;
    ustar_node_t *entry = node->child;
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

superblock_t *tarfs_init() {
    if (g_lim_modules == NULL || g_lim_modules->module_count == 0) {
        k_error("Could not load limine modules\n");
        return NULL;
    }
    
    initramfs_start                         = g_lim_modules->modules[0]->address;
    initramfs_size                          = g_lim_modules->modules[0]->size;


    initramfs_root                          = k_alloc(sizeof(ustar_node_t));
    initramfs_root->type                    = INODE_FOLDER;
    initramfs_root->inode.type              = INODE_FOLDER;
    initramfs_root->inode.fs_data           = initramfs_root;
    initramfs_root->inode.id                = initramfs_next_id++;

    initramfs_superblock                    = k_alloc(sizeof(superblock_t));
    initramfs_superblock->ops               = k_alloc(sizeof(vfs_ops_t));
    initramfs_superblock->fs_type           = FS_USTAR;
    initramfs_superblock->ops->lookup       = tarfs_lookup;
    initramfs_superblock->ops->read         = tarfs_read;
    initramfs_superblock->ops->stat         = tarfs_stat;
    initramfs_superblock->ops->read_dir     = tarfs_read_dir;
    initramfs_superblock->root              = &initramfs_root->inode;

    initramfs_root->inode.parent_sb         = initramfs_superblock;

    ustar_header_t *header = (ustar_header_t *)initramfs_start;
    unsigned int offset = 0;

    while (offset < initramfs_size) { // For each file in the tar archive
        if (header->file_name[0] == '\0') { break; }

        if (strncmp(header->ustar_magic, USTAR_MAGIC, 5) != 0) {
            k_error("Invalid USTAR header magic\n");
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
