#include "proto.h"

int read(uint64_t fd, size_t size, void *buffer) {
    return syscall(SYS_READ, fd, (uint64_t)size, (uint64_t)buffer);
};

int write(uint64_t fd, size_t size, const void *buffer) {
    return syscall(SYS_WRITE, fd, (uint64_t)size, (uint64_t)buffer);
}

int open(const char *path, const char *flags) {
    return syscall(SYS_OPEN, (uint64_t)path, (uint64_t)flags, 0);
}

int close(uint64_t fd) {
    return syscall(SYS_OPEN, fd, 0, 0);
};