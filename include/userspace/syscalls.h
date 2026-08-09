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
    SYS_NB_CREATE_PROCESS,
    SYS_FETCH_FB,
    SYS_CHDIR,
    SYS_GETCWD,
    SYS_GETPID,
    SYS_BRK,
    SYS_SBRK,

    SYS_SEND,
    SYS_RECEIVE,
    SYS_DISPATCH,
    SYS_CONSUME,
    SYS_SUBSCRIBE,
    SYS_UNSUBSCRIBE,
    SYS_SHARE,
    SYS_UNSHARE,

    SYS_SET_CURSOR,
    SYS_CLEAR,

    SYS_PANIC,

    NUM_SYSCALLS // changes automatically
};

typedef int (*syscall_t)(uint64_t, uint64_t, uint64_t, uint64_t, uint64_t, uint64_t);

void syscall_handler(idt_frame_t *frame);
void sys_exit();

#endif // SYSCALLS_H
