#include "debug/logger.h"
#include "debug/errors.h"
#include "graphics/console.h"
#include "memory/heap.h"
#include "string.h"
#include "filesystems/tarfs.h"
#include "filesystems/devfs.h"

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
            k_error("No parent superblock\n");
            return NULL;
        }
        if (!current->parent_sb->ops || !current->parent_sb->ops->lookup) {
            k_error("Operation \"lookup\" not supported on fs\n");
            return NULL;
        }

        int res = current->parent_sb->ops->lookup(current, segments[i], &next);
        if (res != PROTO_OK || !next) {
            k_error("Lookup failed with status=");
            print_f("%d\n", res);
            return NULL;
        }

        if (next->child_sb != NULL) {
            next = next->child_sb->root;
        }

        current = next;
        if (next->type != INODE_FOLDER && i < seg_count - 1) {
            k_error("Path element is not a folder\n");
            return NULL;
        }
    }

    return current;
}

file_descriptor_t *vfs_open(inode_t *cwd, char *path, uint8_t flags) {
    inode_t *inode = vfs_lookup(cwd, path);
    if (inode == NULL) {
        k_error("Cannot open: lookup failed\n");
        return NULL;
    }

    file_descriptor_t *fd = k_alloc(sizeof(file_descriptor_t));
    if (fd == NULL) {
        k_error("Cannot allocate file descriptor\n");
        return NULL;
    }

    fd->flags = flags;
    fd->inode = inode;

    return fd;
}

int vfs_read(file_descriptor_t *fd, size_t size, void *buffer) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    if (!inode->parent_sb->ops || !inode->parent_sb->ops->read) {
        k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
        return PROTO_ERR_FILE_UNSUPPORTED_OP;
    }

    if (inode->type != INODE_FILE) {
        k_assert(PROTO_ERR_IS_A_DIRECTORY);
        return PROTO_ERR_IS_A_DIRECTORY;
    }

    if (!(fd->flags & FD_READ)) {
        k_assert(PROTO_ERR_FILE_UNAUTHORIZED_OP);
        return PROTO_ERR_FILE_UNAUTHORIZED_OP;
    }

    return inode->parent_sb->ops->read(fd->inode, size, buffer);
}

int vfs_write(file_descriptor_t *fd, size_t size, const void *buffer) {
    inode_t *inode = fd->inode;
    if (inode == NULL) {
        k_assert(PROTO_ERR_INVALID_ARGUMENT);
        return PROTO_ERR_INVALID_ARGUMENT;
    }

    if (!inode->parent_sb->ops || !inode->parent_sb->ops->write) {
        k_assert(PROTO_ERR_FILE_UNSUPPORTED_OP);
        return PROTO_ERR_FILE_UNSUPPORTED_OP;
    }

    if (inode->type != INODE_FILE) {
        k_assert(PROTO_ERR_IS_A_DIRECTORY);
        return PROTO_ERR_IS_A_DIRECTORY;
    }

    if (!(fd->flags & FD_WRITE)) {
        k_assert(PROTO_ERR_FILE_UNAUTHORIZED_OP);
        return PROTO_ERR_FILE_UNAUTHORIZED_OP;
    }

    return inode->parent_sb->ops->write(fd->inode, size, buffer);
}

int vfs_close(file_descriptor_t *fd) {
    k_free(fd);
    return PROTO_OK;
}

int vfs_mount(superblock_t *sb, char *path) {
    inode_t *mountpoint = vfs_lookup(rootfs->root, path);
    if (mountpoint == NULL || mountpoint->type != INODE_FOLDER) {
        k_assert(PROTO_ERR_FILE_NOT_FOUND);
        return PROTO_ERR_FILE_NOT_FOUND;
    }

    mountpoint->child_sb = sb;

    k_debug("Mounted type ");
    print_f("%d filesystem @ %s\n", sb->fs_type, path);

    return PROTO_OK;
}

void vfs_init() {
    rootfs = tarfs_init();
    superblock_t *devfs = devfs_init();
    vfs_mount(devfs, "/Devices");
}