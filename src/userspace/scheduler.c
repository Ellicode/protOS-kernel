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

thread_t* g_threads         = NULL;
thread_t* g_current_thread  = NULL;
ticketlock_t threads_lock   = {0};
thread_t *idle;
wait_queue_t *run_queue;

void idle_fn() {
    while (1) {
        __asm__ volatile ("hlt");
    }
}

void scheduler_init() {
    g_threads = NULL;
    g_current_thread = NULL;

    run_queue = k_alloc(sizeof(wait_queue_t));

    idle = k_alloc(sizeof(thread_t));
    idle->next          = NULL;
    idle->prev          = idle;
    idle->state         = THREAD_RUNNING;
    idle->stack_base    = 0;
    memset(&idle->context, 0, sizeof(idt_frame_t));
    g_threads           = idle;
    g_current_thread    = idle;
}

void queue_sleep(wait_queue_t *wq, thread_t *thread) {
    __thread_queue_inner *thread_item = k_alloc(sizeof(__thread_queue_inner));
    thread_item->thread = thread;
    
    LL_APPEND(thread_item, wq->head);
    thread->state = THREAD_SLEEPING;

    scheduler_yield();
}   

void queue_wake_all(wait_queue_t *wq) {
    __thread_queue_inner *t = wq->head;
    wq->head = NULL; 

    while (t != NULL) {
        __thread_queue_inner *next = t->next;
        t->thread->state = THREAD_RUNNING;
        k_free(t);
        t = next;
    }
}

void destroy_thread(thread_t *t) {
    int lock1r = ticketlock_lock(&threads_lock);

    process_t *proc = t->process;   // save before free
    LL_UNLINK(t, g_threads);
    k_free(t);

    if (proc) {
        LL_UNLINK(proc, g_active_processes);
        destroy_addr_space(proc->cr3);   // otherwise there would be a huge 5mb memory leak
        k_free(proc->kernel_stack);
        k_free(proc->msg_queue.waiters);
        k_free(proc);
    }

    ticketlock_unlock(&threads_lock, lock1r);
}

void scheduler_tick(idt_frame_t* ctx) {
    memcpy(&g_current_thread->context, ctx, sizeof(idt_frame_t));
        
    LL_UNLINK(g_current_thread, g_threads);
    LL_APPEND(g_current_thread, g_threads);

    thread_t *next = g_threads;

    while (next->state != THREAD_RUNNING) {
        thread_t *temp = next;
        next = next->next;

        if (temp->state == THREAD_STOPPED) {
            destroy_thread(temp);
        }

        if (next == NULL) {
            next = idle;
            break;
        }
    };

    if (next->process != NULL) {
        if (g_current_thread->process == NULL ||
            next->process->cr3 != g_current_thread->process->cr3) {
            tss.rsp0 = (uint64_t)next->process->kernel_stack + KERNEL_STACK_SIZE;
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
    k_debug("Creating thread\n");

    int lock1r = ticketlock_lock(&threads_lock);

    thread_t *thread = k_alloc(sizeof(thread_t));  
    void *stack = k_alloc(81024);                  

    if (stack == NULL) {
        k_error("Could not allocate stack for kernel thread!\n");
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }

    if (thread == NULL) {
        k_error("Could not allocate thread struct!\n");
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

    LL_APPEND(thread, g_threads);

    ticketlock_unlock(&threads_lock, lock1r);
    return thread;
}

thread_t* create_user_thread(process_t *process, uint64_t entry_point) {
    int lock1r = ticketlock_lock(&threads_lock);

    thread_t *thread = k_alloc(sizeof(thread_t));  
    if (thread == NULL) {
        k_error("Could not allocate thread struct!\n");
        ticketlock_unlock(&threads_lock, lock1r);
        return NULL;
    }

    uint64_t stack = (uint64_t)vmm_map_range(process->cr3, USER_STACK_BASE, USER_STACK_SIZE, F_PRESENT | F_USER | F_WRITE);
    if (stack == 0) {
        k_error("Could not allocate stack for user thread!\n");
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

    LL_APPEND(thread, g_threads);

    ticketlock_unlock(&threads_lock, lock1r);

    return thread;
}


void exit_thread(thread_t* thread) {
    int lock1r = ticketlock_lock(&threads_lock);

    LL_UNLINK(thread, g_threads);

    // k_free((void *)thread->stack_base);
    k_free((void *)thread);

    if (thread->process) {
        LL_UNLINK(thread->process, g_active_processes);

        k_free(thread->process->kernel_stack);
        k_free(thread->process);
    }

    ticketlock_unlock(&threads_lock, lock1r);
}