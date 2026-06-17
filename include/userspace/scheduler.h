#include "interrupts/interrupts.h"
#include "userspace/process.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef struct Thread {
    idt_frame_t     context;
    uint64_t        stack_base;
    process_t       *process;

    struct Thread   *next;
    struct Thread   *prev;
} Thread;
typedef Thread thread_t;

extern thread_t* g_current_thread;

void scheduler_init();
void scheduler_tick(idt_frame_t* ctx);
thread_t* create_kernel_thread(void (*fn)());
thread_t* create_user_thread(process_t *process, uint64_t entry_point);
void exit_thread(thread_t* thread);

#endif // SCHEDULER_H