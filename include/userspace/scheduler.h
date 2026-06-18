#include "interrupts/interrupts.h"
#include "userspace/process.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {
    THREAD_RUNNING,
    THREAD_SLEEPING
} thread_state_t;


typedef struct Thread {
    idt_frame_t     context;
    uint64_t        stack_base;
    process_t       *process;
    thread_state_t  state;

    struct Thread   *next;
    struct Thread   *prev;
} Thread;
typedef Thread thread_t;

typedef struct wait_queue {
    thread_t        *head;
} wait_queue_t;

extern thread_t* g_current_thread;

void scheduler_init();
void scheduler_tick(idt_frame_t* ctx);
void queue_sleep(wait_queue_t *wq, thread_t *thread);
void queue_wake_all(wait_queue_t *wq);
thread_t* create_kernel_thread(void (*fn)());
thread_t* create_user_thread(process_t *process, uint64_t entry_point);
void exit_thread(thread_t* thread);
void scheduler_yield();

#endif // SCHEDULER_H