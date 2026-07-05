#include "interrupts/interrupts.h"
#include "userspace/process.h"
#include "utils/ticket_lock.h"

#ifndef SCHEDULER_H
#define SCHEDULER_H

typedef enum {
    THREAD_RUNNING,
    THREAD_SLEEPING,
    THREAD_STOPPED
} thread_state_t;


typedef struct Thread {
    idt_frame_t     context;
    uint64_t        stack_base;
    process_t       *process;
    thread_state_t  state;

    int             waiting_for;

    struct Thread   *next;
    struct Thread   *prev;
} Thread;
typedef Thread thread_t;

typedef struct __thread_queue_inner {
    struct Thread               *thread;

    struct __thread_queue_inner * prev;
    struct __thread_queue_inner * next;
} __thread_queue_inner;

typedef struct wait_queue {
    __thread_queue_inner    *head;
    ticketlock_t            *lock;
} wait_queue_t;

extern thread_t* g_current_thread;
extern thread_t* g_threads;
extern wait_queue_t *run_queue;

void scheduler_init();
void scheduler_tick(idt_frame_t* ctx);
void queue_sleep(wait_queue_t *wq, thread_t *thread);
void queue_wake_all(wait_queue_t *wq);
thread_t* create_kernel_thread(void (*fn)());
thread_t* create_user_thread(process_t *process, uint64_t entry_point);
void exit_thread(thread_t* thread);
void scheduler_yield();

#endif // SCHEDULER_H