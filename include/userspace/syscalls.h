#include "interrupts/interrupts.h"

#ifndef SYSCALLS_H
#define SYSCALLS_H

void syscall_handler(idt_frame_t *frame);

#endif // SYSCALLS_H