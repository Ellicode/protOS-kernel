#include <stdint.h>

#ifndef VFS_H
#define VFS_H

enum {
    INODE_FILE = 0,
    INODE_FOLDER = 1,
};
typedef uint8_t inode_type_t;

// Forward declarations
typedef struct dentry_t      dentry_t;
typedef struct superblock_t  superblock_t;
typedef struct mount_t       mount_t;
typedef struct inode_t       inode_t;
typedef struct inode_ops_t   inode_ops_t;

struct dentry_t
{
    char          name[256];
    dentry_t       *parent;
    dentry_t     *children;
    dentry_t         *next;
    dentry_t         *prev;
    inode_t         *inode;
    mount_t         *mount;
};

struct superblock_t
{
    dentry_t        *root;
};

struct mount_t
{
    dentry_t    *mountpoint;
    superblock_t        *sb;
};

struct inode_t
{
    uint64_t          uid;
    inode_type_t     type;
    uint64_t         size;

    inode_ops_t      *ops;
};

struct inode_ops_t
{
    /* ops here */
};

#define PATH_DELIM "/\\" // so people can use paths with / AND \ !

void vfs_init();

#endif // VFS_H