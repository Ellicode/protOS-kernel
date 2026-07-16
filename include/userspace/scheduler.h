#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "userspace/process.h"
#include "interrupts/interrupts.h"
#include "utils/ticket_lock.h"
#include "utils/wait_queue.h"

typedef struct process_t process_t;

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

extern void scheduler_yield(void);

#endif // SCHEDULER_H