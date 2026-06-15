#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/heap.h"
#include "string.h"
#include "filesystems/tarfs.h"

#include "filesystems/vfs.h"

superblock_t *rootfs;

int split_path(const char *path, char segments[][256], int max_segs) {
    int count = 0;
    const char *p = path;
    while (*p && count < max_segs) {
        if (*p == PATH_DELIM[0] || *p == PATH_DELIM[1]) { p++; continue; }
        int len = 0;
        while (p[len] && p[len] != PATH_DELIM[0] && p[len] != PATH_DELIM[1]) { len++; }
        if (len > 0) {
            strncpy(segments[count++], p, len);
            segments[count-1][len] = '\0';
        }
        p += len;
    }
    return count;
}

inode_t *vfs_lookup(inode_t *cwd, char *path) {
    inode_t *current = cwd;
    uint8_t starts_with_slash = (path[0] == PATH_DELIM[0] || path[0] == PATH_DELIM[1]);
    
    if (starts_with_slash) {
        if (strlen(path) == 1) {
            return rootfs->root;
        } else {
            current = rootfs->root;
        }
    }

    char segments[32][256];
    int seg_count = split_path(path, segments, 32);

    for (int i = 0; i < seg_count; i++) {
        inode_t *next;

        if (!current->parent_sb) {
            k_error("No parent superblock\n", "proto.kernel.vfs_lookup");
            return NULL;
        }
        if (!current->parent_sb->ops || !current->parent_sb->ops->lookup) {
            k_error("Operation \"lookup\" not supported on fs\n", "proto.kernel.vfs_lookup");
            return NULL;
        }

        int res = current->parent_sb->ops->lookup(current, segments[i], &next);
        if (res != 0 || !next) {
            k_error("Lookup failed with status=", "proto.kernel.vfs_lookup");
            print_f("%d\n", res);
            return NULL;
        }

        current = next;
        if (next->type != INODE_FOLDER && i < seg_count - 1) {
            k_error("Path element is not a folder\n", "proto.kernel.vfs_lookup");
            return NULL;
        }
    }

    return current;
}

file_descriptor_t *vfs_open(inode_t *cwd, char *path, uint8_t flags) {
    inode_t *inode = vfs_lookup(cwd, path);
    if (inode == NULL) {
        k_error("Cannot open: lookup failed\n", "proto.kernel.vfs_open");
        return NULL;
    }

    file_descriptor_t *fd = k_alloc(sizeof(file_descriptor_t));
    if (fd == NULL) {
        k_error("Cannot allocate file descriptor\n", "proto.kernel.vfs_open");
        return NULL;
    }

    fd->flags = flags;
    fd->inode = inode;

    return fd;
}

int vfs_read(file_descriptor_t *fd, size_t size, void *buffer) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_error("Cannot read: file descriptor is empty\n", "proto.kernel.vfs_read");
        return 1;
    }

    if (!inode->parent_sb->ops || !inode->parent_sb->ops->read) {
        k_error("Operation \"read\" not supported on fs\n", "proto.kernel.vfs_read");
        return 1;
    }

    if (!(fd->flags & FD_READ)) {
        k_error("Cannot read: operation unauthorized.\n", "proto.kernel.vfs_read");
        return 1;
    }

    return inode->parent_sb->ops->read(fd->inode, size, buffer);
}

int vfs_close(file_descriptor_t *fd) {
    k_free(fd);
    return 1;
}

void vfs_init() {
    rootfs = tarfs_init();
}