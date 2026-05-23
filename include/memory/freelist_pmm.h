/*
    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/

#include <stdint.h>

#include "interrupts/interrupts.h"
#include "utils/utils.h"
#include "memory/vmm.h"

#ifndef FREELIST_PMM_H
#define FREELIST_PMM_H

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

typedef struct freelist_pmm_node {
    struct freelist_pmm_node* next;
} freelist_pmm_node_t;

extern freelist_pmm_node_t* freelist_pmm_head;
extern uint64_t freelist_pmm_fill_entry;
extern uint64_t freelist_pmm_fill_offset;

void* fpmm_allocate_page();
void  fpmm_free_page(void* ptr);
int   fpmm_init();

#endif // FREELIST_PMM_H