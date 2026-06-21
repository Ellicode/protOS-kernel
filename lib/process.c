#include "proto.h"

int create_process(const char *elf) {
    return syscall(SYS_CREATE_PROCESS, (uint64_t)elf, 0, 0);
}