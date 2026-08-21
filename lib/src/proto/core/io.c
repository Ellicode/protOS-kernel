#include <stdarg.h>
#include <proto/core.h>

int read(uint64_t fd, void *buffer, size_t size) {
    return syscall(SYS_READ, fd, (uint64_t)size, (uint64_t)buffer);
};

int write(uint64_t fd, const void *buffer, size_t size) {
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
    return syscall(SYS_STAT, fd, (uint64_t)buffer, 0);
};

int read_dir(uint64_t fd, dentry_t *entries, int *num_entries) {
    return syscall(SYS_READ_DIR, fd, (uint64_t)entries, (uint64_t)num_entries);
};

int panic(char *ename) {
    return syscall(SYS_PANIC, (uint64_t)ename, 0, 0);
}

int input(char *buffer) {
    size_t read_bytes = read(0, buffer, 1024);
    buffer[read_bytes] = 0;
    return read_bytes;
}
void printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    write(STDOUT, buffer, strlen(buffer));
}

void fprintf(uint64_t fd, const char *format, ...) {
    va_list args;
    va_start(args, format);
    char buffer[1024];
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    write(fd, buffer, strlen(buffer));
}

void set_cursor(int row, int col) {
    syscall(SYS_SET_CURSOR, row, col, 0);
}

void clear() {
    syscall(SYS_CLEAR, 0, 0, 0);
}

int brk(void *addr) {
    return syscall(SYS_BRK, (uint64_t)addr, 0, 0);
}

int sbrk(size_t size) {
    return syscall(SYS_SBRK, size, 0, 0);
}

int setiofd(char *path) {
    close(STDIN);
    close(STDOUT);
    int stdin = open(path, "r");
    if (stdin < PROTO_OK) { return PROTO_ERR_UNKNOWN; }
    int stdout = open(path, "w");
    if (stdout < PROTO_OK) { return PROTO_ERR_UNKNOWN; }
    return PROTO_OK;
}