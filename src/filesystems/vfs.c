#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/heap.h"
#include "string.h"

#include "filesystems/vfs.h"

superblock_t *rootfs;
dentry_t *root;

dentry_t *vfs_lookup(char *path) {
    dentry_t *current_dir = root;
    uint8_t starts_with_slash = path[0] == PATH_DELIM[0];
    
    if (starts_with_slash && strlen(path) == 1) {
        return root;
    } else {
        char *token = strtok(path, PATH_DELIM);
        while (token != NULL) {
            dentry_t *entry = current_dir->children;
            dentry_t *found = NULL;
            
            if (strcmp(token, "..") == 0) {
                current_dir = current_dir->parent;
                break;
            }

            while (entry != NULL) {
                if (strcmp(entry->name, token) == 0) {
                    found = entry;
                    break;
                }
                entry = entry->next;
            }
            
            if (found == NULL) {
                k_error("Path component ", "proto.kernel.vfs_mount");
                print_f("\"%s\" not found.\n", token);
                return NULL;
            }
            
            if (!found->inode || !(found->inode->type & INODE_FOLDER)) {
                k_error("Path component\n", "proto.kernel.vfs_mount");
                print_f("\"%s\" is not a directory.\n", token);
                return NULL;
            }
            
            if (found->mount != NULL) {
                current_dir = found->mount->sb->root;
            } else {
                current_dir = found;
            }

            token = strtok(NULL, PATH_DELIM);
        }
    }

    return current_dir;
}

dentry_t *vfs_mkdir(dentry_t *parent, char *name) {
    dentry_t *entry = k_alloc(sizeof(dentry_t));
    
    entry->inode = k_alloc(sizeof(inode_t));
    entry->inode->type = INODE_FOLDER;

    strcpy(entry->name, name);

    entry->parent = parent;
    entry->next = parent->children;
    parent->children = entry;

    return entry;
}

mount_t *vfs_mount(char *path) {
    dentry_t *entry = vfs_lookup(path);    
    if (entry == NULL) {
        k_error("Directory entry not found: ", "proto.kernel.vfs_mount");
        print_f("%s\n", path);
        return NULL;
    } else if (entry->inode->type != INODE_FOLDER) {
        k_error("Directory entry is not a folder: ", "proto.kernel.vfs_mount");
        print_f("%s\n", path);
        return NULL;
    } else {
        mount_t *mount = k_alloc(sizeof(mount_t));
        superblock_t *sb = k_alloc(sizeof(superblock_t));
        dentry_t *fs_root = k_alloc(sizeof(dentry_t));

        fs_root->inode = k_alloc(sizeof(inode_t));
        fs_root->inode->type = INODE_FOLDER;

        sb->root = fs_root;
        mount->mountpoint = entry;
        mount->sb = sb;
        entry->mount = mount;

        k_debug("mounted: ", "proto.kernel.vfs_mount");
        print_f("/.../%s (path=%s)\n", entry->name, path);

        return mount;
    }
}

int vfs_unmount(mount_t *mount) {
    if (mount == NULL || mount->mountpoint == NULL) { return 1; }
    mount->mountpoint->mount = NULL;
    
    k_free(mount->sb->root);
    k_free(mount->sb);
    k_free(mount);

    k_debug("unmounted: ", "proto.kernel.vfs_mount");
    print_f("/.../%s\n", mount->mountpoint->name);

    return 0;
}

void vfs_init() {
    k_debug("Vfs init\n", "proto.kernel.vfs_init");
    rootfs = k_alloc(sizeof(superblock_t));
    memset(rootfs, 0, sizeof(superblock_t));
    root = k_alloc(sizeof(dentry_t));
    if (root == NULL) {
        k_error("Could not allocate root directory entry!\n",
            "proto.kernel.vfs_init");
        return;
    }
    rootfs->root = root;

    dentry_t *home = vfs_mkdir(root, "home");
    mount_t *m = vfs_mount("/home/");

    dentry_t *ellicode = vfs_mkdir(m->sb->root, "ellicode");
    vfs_mkdir(ellicode, "documents");

    dentry_t *elli = vfs_lookup("/home/ellicode/documents/../");
    print_f("%s\n", elli->name);

    vfs_unmount(m);
}