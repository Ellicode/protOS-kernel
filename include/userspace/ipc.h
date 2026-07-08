#ifndef IPC_H
#define IPC_H

#include <stdint.h>

#include "utils/ticket_lock.h"
#include "userspace/scheduler.h"
#include "utils/wait_queue.h"

typedef struct ipc_message_t {
    uint64_t                sender;
    char                    name[255];

    uint64_t                size;
    void                    *data;

    struct ipc_message_t    *next;
    struct ipc_message_t    *prev;
} ipc_message_t;

typedef struct ipc_queue_t {
    ipc_message_t   *messages;
    wait_queue_t    *waiters;

    ticketlock_t    lock;
} ipc_queue_t;

typedef struct ipc_syscall_payload {
    char            *message;
    void            *data;
    size_t          size;
} ipc_syscall_payload;

int ipc_send(uint64_t pid, char *message, void *data, size_t size);
int ipc_recieve(ipc_message_t **buf);
int ipc_dispatch(char *message, void *data, size_t size);

#endif // IPC_H