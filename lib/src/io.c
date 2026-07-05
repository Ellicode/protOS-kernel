#include <stdarg.h>

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
    return syscall(SYS_CLOSE, fd, 0, 0);
};

int chdir(const char *path) {
    return syscall(SYS_CHDIR, (uint64_t)path, 0, 0);
};

int getcwd(char *buf, size_t size) {
    return syscall(SYS_GETCWD, (uint64_t)buf, size, 0);
};

int stat(uint64_t fd, dentry_t *buffer) {
    return syscall(SYS_GETCWD, fd, (uint64_t)buffer, 0);
};

int read_dir(uint64_t fd, dentry_t *entries, int *num_entries) {
    return syscall(SYS_READ_DIR, fd, (uint64_t)entries, (uint64_t)num_entries);
};

int input(char *buffer) {
    return read(0, 0, buffer);
}

void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    write(STDOUT, 0, buffer);
}

void fprintf(uint64_t fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    write(fd, 0, buffer);
}