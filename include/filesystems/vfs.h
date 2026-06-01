#include <stdint.h>

#ifndef VFS_H
#define VFS_H

enum {
    INODE_FILE = 0,
    INODE_FOLDER = 1,
};
typedef uint8_t inode_type_t;

typedef struct DirectoryEntry
{
    char                    name[256];
    struct DirectoryEntry     *parent;
    struct DirectoryEntry   *children;
    struct DirectoryEntry       *next;
    struct Inode               *inode;
    uint8_t             is_mountpoint;
    struct Mount               *mount; // NULL if not a mountpoint
} DirectoryEntry;
typedef DirectoryEntry dentry_t;

typedef struct SuperBlock
{
    struct DirectoryEntry       *root;
} SuperBlock;
typedef SuperBlock superblock_t;

typedef struct Mount {
    struct dentry         *mountpoint; 
    struct superblock             *sb;     
} Mount;
typedef Mount mount_t;

typedef struct Inode
{
    uint32_t                      uid;
    inode_type_t                 type;
    uint64_t                     size;
    uint32_t                    nlink;

    struct InodeOps              *ops;
    void                     *fs_data;
} Inode;
typedef Inode inode_t;

typedef struct InodeOps
{
    /* ops here */
} InodeOps;
typedef InodeOps inode_ops_t;

#endif // VFS_H