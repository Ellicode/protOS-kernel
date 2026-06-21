#ifndef PROTO_H
#define PROTO_H

#include <stdint.h>
#include <stdlib.h>

// SYSCALL ===================================================================================

enum {
    SYS_EXIT,

    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,

    SYS_CREATE_PROCESS,
    SYS_FETCH_FB
};

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3);

// IO ========================================================================================

int read(uint64_t fd, size_t size, void *buffer);
int write(uint64_t fd, size_t size, const void *buffer);
int open(const char *path, const char *flags);
int close(uint64_t fd);

// PROCESS ===================================================================================

int create_process(const char *elf);

// GRAPHICS ==================================================================================

typedef struct __attribute__((packed)) {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
    uint64_t bpp;
} fb_info_t;

int fetch_framebuffer(fb_info_t *fb);

#endif
