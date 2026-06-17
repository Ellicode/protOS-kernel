#include "interrupts/interrupts.h"

#ifndef SYSCALLS_H
#define SYSCALLS_H

#define NUM_SYSCALLS    7

enum {
    SYS_EXIT,

    SYS_READ,
    SYS_WRITE,
    SYS_OPEN,
    SYS_CLOSE,

    SYS_CREATE_PROCESS,
    SYS_FETCH_FB
};

typedef uint64_t (*syscall_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

void syscall_handler(idt_frame_t *frame);

#endif // SYSCALLS_H