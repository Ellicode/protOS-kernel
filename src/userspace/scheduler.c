#include <stddef.h>

#include "utils/ticket_lock.h"
#include "debug/logger.h"
#include "graphics/console.h"
#include "memory/vmm.h"
#include "interrupts/interrupts.h"
#include "utils/linked_lists.h"
#include "memory/heap.h"
#include "string.h"
#include "globals.h"
#include "gdt.h"

#include "userspace/scheduler.h"

thread_t* threads         = NULL;
thread_t* g_current_thread  = NULL;
ticketlock_t threads_lock = {0};
thread_t *idle;

void idle_fn() {
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void scheduler_init() {
    threads = NULL;
    g_current_thread = NULL;

    idle = k_alloc(sizeof(thread_t));
    idle->next          = NULL;
    idle->prev          = idle;
    idle->state         = THREAD_RUNNING;
    idle->stack_base    = 0;
    memset(&idle->context, 0, sizeof(idt_frame_t));
    threads             = idle;
    g_current_thread    = idle;
}

void scheduler_yield() {
    __asm__ volatile (
        "pushq %rbp;" // for backtracing
        "movq %rsp, %rbp;"
        "pushf;"
        "sti;"
        "int $0x20;"
        "popf;"
        "popq %rbp;"
        "ret;"
    );
}

void queue_sleep(wait_queue_t *wq, thread_t *thread) {
    int lock = ticketlock_lock(wq->lock);
    thread->state = THREAD_SLEEPING;
    __thread_queue_inner *thread_item = k_alloc(sizeof(__thread_queue_inner));
    thread_item->thread = thread;
    LL_APPEND(thread_item, wq->head);
    ticketlock_unlock(wq->lock, lock);
    scheduler_yield();
}   

void queue_wake_all(wait_queue_t *wq) {
    if (wq->lock == NULL) { return; };
    int lock = ticketlock_lock(wq->lock);
    __thread_queue_inner *t = wq->head;
    while (t != NULL) {
        __thread_queue_inner *next = t->next;
        t->thread->state = THREAD_RUNNING;
        LL_UNLINK(t, wq->head);
        k_free(t);
        t = next;
    }
    ticketlock_unlock(wq->lock, lock);
}

void scheduler_tick(idt_frame_t* ctx) {
    memcpy(&g_current_thread->context, ctx, sizeof(idt_frame_t));
        
    LL_UNLINK(g_current_thread, threads);
    LL_APPEND(g_current_thread, threads);

    thread_t *next = threads;

    while (next->state == THREAD_SLEEPING) {
        next = next->next;
        if (next == NULL) {
            next = idle;
            break;
        }
    };

    if (next->process != NULL) {
        if (g_current_thread->process == NULL ||
            next->process->cr3 != g_current_thread->process->cr3) {
            tss.rsp0 = (uint64_t)next->process->kernel_stack + 4096;
            _load_cr3(next->process->cr3);
        }
    } else {
        uint64_t kernel_cr3 = (uint64_t)kernel_pml4 - g_lim_hhdm->offset;
        _load_cr3(kernel_cr3);
    }

    memcpy(ctx, &next->context, sizeof(idt_frame_t));

    g_current_thread = next;
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
    thread->context.rsp     = ((uint64_t)stack + 81024) & ~0xFULL;
    thread->context.rflags  = 0x202;

    LL_APPEND(thread, threads);

    ticketlock_unlock(&threads_lock, lock1r);
    return thread;
}

thread_t* create_user_thread(process_t *process, uint64_t entry_point) {
    int lock1r = ticketlock_lock(&threads_lock);

    thread_t *thread = k_alloc(sizeof(thread_t));  
    if (thread == NULL) {
        k_error("Could not allocate thread struct!\n", "proto.kernel.create_user_thread");
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }

    uint64_t stack = (uint64_t)vmm_map_range(process->cr3, USER_STACK_BASE, USER_STACK_SIZE, F_PRESENT | F_USER | F_WRITE);
    if (stack == 0) {
        k_error("Could not allocate stack for user thread!\n", "proto.kernel.create_user_thread");
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }


    thread->stack_base      = stack;
    thread->context.rip     = entry_point;
    thread->context.cs      = (uint64_t)GDT_OFFSET_USER_CODE | 3;
    thread->context.ss      = (uint64_t)GDT_OFFSET_USER_DATA | 3;
    thread->context.rsp     = (stack + USER_STACK_SIZE) & ~0xFULL;
    thread->context.rflags  = 0x202;
    thread->process         = process;
    thread->state           = THREAD_RUNNING;

    LL_APPEND(thread, threads);

    ticketlock_unlock(&threads_lock, lock1r);

    return thread;
}


void exit_thread(thread_t* thread) {
    int lock1r = ticketlock_lock(&threads_lock);

    LL_UNLINK(thread, threads);

    k_free((void *)thread->stack_base);
    k_free((void *)thread);

    if (thread->process) {
        LL_UNLINK(thread->process, g_active_processes);

        k_free(thread->process->kernel_stack);
        k_free(thread->process);
    }

    ticketlock_unlock(&threads_lock, lock1r);
}