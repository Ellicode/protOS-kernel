#include <stddef.h>

#include "debug/logger.h"
#include "graphics/tty.h"
#include "interrupts/interrupts.h"
#include "utils/linked_lists.h"
#include "memory/heap.h"
#include "string.h"
#include "gdt.h"

#include "threads/scheduler.h"

thread_t* threads        = NULL;
thread_t* current_thread = NULL;

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
    print_f("tick: current=%x next=%x threads=%x\n",
        current_thread, current_thread->next, threads);

    LL_UNLINK(current_thread, threads);
    LL_APPEND(current_thread, threads);

    memcpy(&current_thread->context, ctx, sizeof(idt_frame_t));
    memcpy(ctx, &threads->context, sizeof(idt_frame_t));

    current_thread = threads;

    print_f("rflags: %x\n", current_thread->context.rflags);
    print_f("current_thread %x\n", (uint64_t)current_thread);
}

thread_t* create_kernel_thread(void (*fn)()) {
    k_debug("Creating thread\n", "proto.kernel.create_kernel_thread");


    thread_t *thread = k_alloc(sizeof(thread_t));  
    void *stack = k_alloc(81024);                  

    if (stack == NULL) {
        k_error("Could not allocate stack for kernel thread!\n", "proto.kernel.create_kernel_thread");
        return NULL;
    }

    if (thread == NULL) {
        k_error("Could not allocate thread struct!\n", "proto.kernel.create_kernel_thread");
        k_free(stack);
        return NULL;
    }

    thread->stack_base      = (uint64_t)stack;
    thread->context.rip     = (uint64_t)fn;
    thread->context.cs      = (uint64_t)GDT_OFFSET_KERNEL_CODE;
    thread->context.ss      = (uint64_t)GDT_OFFSET_KERNEL_DATA;
    thread->context.rsp = ((uint64_t)stack + 81024) & ~0xFULL;
    thread->context.rflags = 0x202;

    LL_APPEND(thread, threads);

    return thread;
}

void exit_kernel_thread(thread_t* thread) {
    LL_UNLINK(thread, threads);
    k_free((void *)thread->stack_base);
    k_free((void *)thread);
}