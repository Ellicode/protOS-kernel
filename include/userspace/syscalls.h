#include "interrupts/interrupts.h"

#ifndef SYSCALLS_H
#define SYSCALLS_H

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

typedef int (*syscall_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

void syscall_handler(idt_frame_t *frame);

#endif // SYSCALLS_H