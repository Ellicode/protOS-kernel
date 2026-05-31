#include "interrupts/interrupts.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct Thread {
    idt_frame_t     context;
    uint64_t        stack_base;

    struct Thread   *next;
    struct Thread   *prev;
} Thread;
typedef Thread thread_t;

void scheduler_init();

void scheduler_tick(idt_frame_t* ctx);

thread_t* create_kernel_thread(void (*fn)());
void exit_kernel_thread(thread_t* thread);

#endif // SCHEDULER_H