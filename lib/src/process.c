#include "proto.h"

int create_process(const char *elf, char argv[16][64]) {
    return syscall(SYS_CREATE_PROCESS, (uint64_t)elf, (uint64_t)argv, 0);
}

void exit() {
    syscall(SYS_EXIT, 0, 0, 0);
}

void wait_for_process(int pid) {
    syscall(SYS_WAIT_FOR_PROCESS, pid, 0, 0);
}