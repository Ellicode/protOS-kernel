#include <proto/core.h>
#include <stdint.h>

int create_process(const char *elf, char argv[16][64], int argc) {
    return syscall(SYS_CREATE_PROCESS, (uint64_t)elf, (uint64_t)argv, argc);
}

int create_nonblocking_process(const char *elf, char argv[16][64], int argc) {
    return syscall(SYS_NB_CREATE_PROCESS, (uint64_t)elf, (uint64_t)argv, argc);
}

void exit() {
    syscall(SYS_EXIT, 0, 0, 0);
}

int getpid() {
    return syscall(SYS_GETPID, 0, 0, 0);
}

int share(int pid, void *mem, size_t size) {
    return syscall(SYS_SHARE, pid, (uint64_t)mem, size);
}
