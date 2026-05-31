#include "interrupts/interrupts.h"

#ifndef TICKET_LOCK_H
#define TICKET_LOCK_H

typedef struct {
    uint32_t next;
    uint32_t owner;
} ticketlock_t;

static inline void ticketlock_init(ticketlock_t* lock) {
    __atomic_store_n(&lock->next,  0, __ATOMIC_RELAXED);
    __atomic_store_n(&lock->owner, 0, __ATOMIC_RELAXED);
}

[[nodiscard]]
static inline int ticketlock_lock(ticketlock_t* lock) {
    int irqs = interrupts_enabled();
    disable_interrupts();

    uint32_t my = __atomic_fetch_add(&lock->next, 1, __ATOMIC_ACQ_REL);
    while (__atomic_load_n(&lock->owner, __ATOMIC_ACQUIRE) != my) {
        asm ("pause");
    }

    return irqs;
}

static inline void ticketlock_unlock(ticketlock_t* lock, int irqs) {
    __atomic_fetch_add(&lock->owner, 1, __ATOMIC_RELEASE);
    restore_interrupts(irqs);
}

#endif