#include <stddef.h>

#include "utils/ticket_lock.h"
#include "debug/logger.h"
#include "graphics/console.h"
#include "interrupts/interrupts.h"
#include "utils/linked_lists.h"
#include "memory/heap.h"
#include "string.h"
#include "gdt.h"

#include "userspace/scheduler.h"

thread_t* threads         = NULL;
thread_t* current_thread  = NULL;
ticketlock_t threads_lock = {0};

void scheduler_init() {
    threads = NULL;
    current_thread = NULL;

    thread_t *idle = k_alloc(sizeof(thread_t));
    idle->next       = NULL;
    idle->prev       = idle;
    idle->stack_base = 0;
    memset(&idle->context, 0, sizeof(idt_frame_t));
    threads        = idle;
    current_thread = idle;
}

void scheduler_tick(idt_frame_t* ctx) {
    LL_UNLINK(current_thread, threads);
    LL_APPEND(current_thread, threads);

    memcpy(&current_thread->context, ctx, sizeof(idt_frame_t));
    memcpy(ctx, &threads->context, sizeof(idt_frame_t));

    current_thread = threads;
}

thread_t* create_kernel_thread(void (*fn)()) {
    k_debug("Creating thread\n", "proto.kernel.create_kernel_thread");

    int lock1r = ticketlock_lock(&threads_lock);

    thread_t *thread = k_alloc(sizeof(thread_t));  
    void *stack = k_alloc(81024);                  

    if (stack == NULL) {
        k_error("Could not allocate stack for kernel thread!\n", "proto.kernel.create_kernel_thread");
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }

    if (thread == NULL) {
        k_error("Could not allocate thread struct!\n", "proto.kernel.create_kernel_thread");
        k_free(stack);
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }

    thread->stack_base      = (uint64_t)stack;
    thread->context.rip     = (uint64_t)fn;
    thread->context.cs      = (uint64_t)GDT_OFFSET_KERNEL_CODE;
    thread->context.ss      = (uint64_t)GDT_OFFSET_KERNEL_DATA;
    thread->context.rsp = ((uint64_t)stack + 81024) & ~0xFULL;
    thread->context.vector  =
    thread->context.rflags  = 0x202;

    LL_APPEND(thread, threads);

    ticketlock_unlock(&threads_lock, lock1r);
    return thread;
}

void exit_kernel_thread(thread_t* thread) {
    int lock1r = ticketlock_lock(&threads_lock);
    LL_UNLINK(thread, threads);
    k_free((void *)thread->stack_base);
    k_free((void *)thread);
    ticketlock_unlock(&threads_lock, lock1r);
}