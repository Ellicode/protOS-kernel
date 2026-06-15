#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/heap.h"
#include "string.h"
#include "filesystems/tarfs.h"

#include "filesystems/vfs.h"

superblock_t *rootfs;
inode_t *root;

inode_t *vfs_lookup(inode_t *cwd, char *path) {
    inode_t *current = cwd;
    uint8_t starts_with_slash = (path[0] == PATH_DELIM[0] || path[0] == PATH_DELIM[1]);
    
    if (starts_with_slash) {
        if (strlen(path) == 1) {
            return root;
        } else {
            current = root;
        }
    }

    char *token = strtok(path, PATH_DELIM);
    inode_t *next;
    while (token != NULL) {
        if (!current->parent_sb->ops || !current->parent_sb->ops->lookup) {
            return NULL;
        }

        int res = current->parent_sb->ops->lookup(current, token, &next);
        if (res != 0 || !next) {
            k_error("Lookup failed mysteriously...", "proto.kernel.vfs_lookup");
            return NULL;
        }

        current = next;
        token = strtok(NULL, PATH_DELIM);

        if (next->type != INODE_FOLDER && token != NULL) {
            k_error("Path element is not a folder", "proto.kernel.vfs_lookup");
            return NULL;
        }
    }

    return current;
}

void vfs_init() {
    superblock_t *tarfs_sb = tarfs_init();

    file_descriptor_t *fd = k_alloc(sizeof(file_descriptor_t));
    char path[] = "hello.txt";
    tarfs_sb->ops->open(path, 0x00, fd);
    print_f("fd_size=%d\n", fd->inode->size);

    char *buff = k_alloc(50);
    tarfs_sb->ops->read(fd, fd->inode->size, buff);

    print_f("file_data=%s\n",buff);
}