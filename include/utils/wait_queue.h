#ifndef WAIT_QUEUE_H
#define WAIT_QUEUE_H

#include "utils/ticket_lock.h"

typedef struct __thread_queue_inner {
    struct Thread               *thread;

    struct __thread_queue_inner * prev;
    struct __thread_queue_inner * next;
} __thread_queue_inner;

typedef struct wait_queue_t {
    __thread_queue_inner    *head;
    ticketlock_t            *lock;
} wait_queue_t;

#endif // WAIT_QUEUE_H