#include <stdint.h>

#ifndef VFS_H
#define VFS_H

enum {
    INODE_FILE = 0,
    INODE_FOLDER = 1,
};
typedef uint8_t inode_type_t;

enum {
    FS_DEVFS = 0,
    FS_USTAR = 1,
};
typedef uint8_t fs_type_t;

// Forward declarations
typedef struct dentry_t             dentry_t;
typedef struct superblock_t         superblock_t;
typedef struct inode_t              inode_t;
typedef struct vfs_ops_t            vfs_ops_t;
typedef struct file_descriptor_t    file_descriptor_t;
struct inode_t
{
    inode_type_t        type;
    uint64_t            size;
    superblock_t        *parent_sb;
    void                *fs_data;
    uint8_t             mount;
    superblock_t        *child_sb;
};

struct vfs_ops_t
{
    int (*read)(
        inode_t *inode,
        uint64_t size,
        char *buffer
    );

    int (*write)(
        inode_t *fd,
        uint64_t size,
        void *buffer
    );

    int (*lookup)(
        inode_t *dir,
        char *name,
        inode_t **result
    );

    int (*mkdir)(
        inode_t *dir,
        char *name,
        inode_t **result
    );

    int (*create)(
        inode_t *dir,
        char *name,
        inode_t **result
    );
};

typedef union {
    struct {
        uint8_t read               :  1;
        uint8_t write              :  1;
    } __attribute__((packed));
    uint8_t value;
} fd_flags;
typedef uint8_t fd_flags_t;

struct file_descriptor_t {
    inode_t             *inode;
    uint64_t            curr_offset;
    fd_flags_t          flags;
};

struct dentry_t
{
    char                name[256];
    inode_t             *inode;
};

struct superblock_t
{
    fs_type_t           fs_type;
    vfs_ops_t           *ops;
    inode_t             *root;
};


#define FD_READ  0x01
#define FD_WRITE 0x02

#define PATH_DELIM "/\\" // so people can use paths with / AND \ !

extern superblock_t *rootfs;

void vfs_init();
int split_path(const char *path, char segments[][256], int max_segs);

file_descriptor_t *vfs_open(inode_t *cwd, char *path, uint8_t flags);
int vfs_read(file_descriptor_t *fd, size_t size, void *buffer);
int vfs_close(file_descriptor_t *fd);

#endif // VFS_H