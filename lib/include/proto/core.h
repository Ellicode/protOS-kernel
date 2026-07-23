#ifndef PROTO_CORE_H
#define PROTO_CORE_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

/*****************************************************************************
 * Error codes
 *****************************************************************************/

enum status_code_t {
    PROTO_OK                            = 0,
    PROTO_ERR_UNKNOWN                   = 1,

    PROTO_ERR_FILE_NOT_FOUND            = 11,
    PROTO_ERR_NOT_A_DIRECTORY           = 12,
    PROTO_ERR_IS_A_DIRECTORY            = 13,
    PROTO_ERR_ALREADY_EXISTS            = 14,
    PROTO_ERR_FILE_UNSUPPORTED_OP       = 15,
    PROTO_ERR_FILE_UNAUTHORIZED_OP      = 16,
    PROTO_ERR_MAX_FD_REACHED            = 17,
    PROTO_ERR_INVALID_FD                = 18,

    PROTO_ERR_INIT_FAILED               = 21,
    PROTO_ERR_OUT_OF_MEMORY             = 22,
    PROTO_ERR_INVALID_ARGUMENT          = 23,
    PROTO_ERR_SYSCALL_OUT_OF_BOUNDS     = 24,
    PROTO_ERR_INVALID_CONTEXT           = 25,

    PROTO_ERR_ELF_INVALID_HDR           = 31,
    PROTO_ERR_ELF_UNSUPPORTED           = 32,
    PROTO_ERR_ELF_CORRUPTED             = 33,
    PROTO_ERR_ELF_CANNOT_LOAD           = 34,

    PROTO_ERR_PROCESS_NOT_FOUND         = 41,
};

/*****************************************************************************
 * Syscall numbers
 *****************************************************************************/

enum {
    SYS_EXIT,

    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,
    SYS_STAT,
    SYS_READ_DIR,

    SYS_CREATE_PROCESS,
    SYS_FETCH_FB,
    SYS_CHDIR,
    SYS_GETCWD,
    SYS_WAIT_FOR_PROCESS,
    SYS_GETPID,

    SYS_SEND,
    SYS_RECIEVE,
    SYS_DISPATCH,
    SYS_CONSUME,
    SYS_SUBSCRIBE,
    SYS_UNSUBSCRIBE,

    SYS_PANIC,

    NUM_SYSCALLS // changes automatically
};

enum {
    STDIN,
    STDOUT,
    STDERR
};

typedef struct about_data_t {
    char os_name[256];
    char os_version[64];
    char os_arch[64];

    int mem_size;
    int mem_used;
} about_data_t;

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

/*****************************************************************************
 * Input-Output operations
 *****************************************************************************/

typedef struct dentry_t dentry_t;

enum {
    INODE_FILE = 0,
    INODE_FOLDER = 1,
};
typedef uint8_t inode_type_t;

struct dentry_t {
    char                name[256];
    uint64_t            size;
    inode_type_t        type;
    void                *k_inode;
};


int read(uint64_t fd, size_t size, void *buffer);
int write(uint64_t fd, size_t size, const void *buffer);
int open(const char *path, const char *flags);
int close(uint64_t fd);
int chdir(const char *path);
int getcwd(char *buf, size_t size);
int stat(uint64_t fd, dentry_t *buffer);
int read_dir(uint64_t fd, dentry_t *entries, int *num_entries);
int panic(char *ename);

int input(char *buffer);
void printf(const char *format, ...);
void fprintf(uint64_t fd, const char *format, ...);

/*****************************************************************************
 * Process management
 *****************************************************************************/

int create_process(const char *elf, char argv[16][64], int argc);
void exit();
void wait_for_process(int pid);
int getpid();

/*****************************************************************************
 * String operations
 *****************************************************************************/

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
size_t strlen(const char*);
void strcat(char *dest, const char *src);
char *strtok(char *src, const char *delim);
char strcmp(const char * s1, const char * s2);
char* strcpy(char* dst, const char* src);
long strtol(const char *nptr, char **endptr, int base);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *dst, const char *src, size_t n);

char *int_to_string(int64_t num);
char *hex_to_string(uint64_t value);

int vsnprintf(char *str, size_t size, const char *format, va_list args);
int snprintf(char *str, size_t size, const char *format, ...);

/*****************************************************************************
 * Malloc and other heap-related operations
 *****************************************************************************/

typedef struct HeapItem {
    size_t size;
    uint64_t flags;
    struct HeapItem* prev;
    struct HeapItem* next;
} __attribute__((packed)) HeapItem;
typedef HeapItem heap_item_t;

#define HEAP_MIN_SIZE       0x0000000000400000 // 4 MiB
#define HEAP_MAX_SIZE       0x00000FFFFFFFFFFF // A lot of GiB
#define HEAP_VIRTUAL_START  0x0000100000000000

#define H_FREE      (1ULL << 0)
#define H_WRITE     (1ULL << 1)
#define H_USER      (1ULL << 2)

void heap_init();
void *malloc(size_t size);
int free(void *ptr);

#endif // PROTO_CORE_H