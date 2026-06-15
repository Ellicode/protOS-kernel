#ifndef TARFS_H
#define TARFS_H

#include "filesystems/vfs.h"

#define USTAR_MAGIC "ustar" // should be this

typedef struct ustar_node_t ustar_node_t;

enum ustar_file_types {
    USTAR_NORMAL_ALT = 0,
    USTAR_NORMAL = '0',
    USTAR_HARD_LINK = '1',
    USTAR_SYMBOLIC_LINK = '2',
    USTAR_CHAR_DEV = '3',
    USTAR_BLOCK_DEV = '4',
    USTAR_DIRECTORY = '5',
    USTAR_PIPE = '6'
};

struct {
    char file_name[100];
    char modes[8];
    char owner_id[8];
    char group_id[8];
    char file_size[12]; // ascii octal representation
    char last_modified_time[12];
    char checksum[8];
    char type;
    char linked_file_name[100];
    char ustar_magic[6];
    char ustar_version[2]; // 00
    char owner_name[32];
    char group_name[32];
    char dev_major[8];
    char dev_minor[8];
    char filename_prefix[155];
    char __pad[12]; // the structure is 512 bytes
} __attribute__((packed)) typedef ustar_header_t;

struct ustar_node_t {
    char                    name[256];
    size_t                  size;        // extracted size
    int64_t                 offset; // offset to the header of this record
    inode_type_t            type;
    inode_t                 inode;
    struct ustar_node_t*    parent;
    struct ustar_node_t*    child; // one level deeper, applies to directories
    struct ustar_node_t*    next; // next on this level
};

#define USTAR_PATH_PREFIX       "./"
#define USTAR_PATH_PREFIX_SIZE  (sizeof(USTAR_PATH_PREFIX) - 1)

superblock_t *tarfs_init();

#endif