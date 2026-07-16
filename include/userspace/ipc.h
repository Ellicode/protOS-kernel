#ifndef IPC_H
#define IPC_H

#include <stdint.h>

#include "utils/ticket_lock.h"
#include "userspace/scheduler.h"
#include "utils/wait_queue.h"

#define MAX_SUBSCRIPTIONS 255

typedef struct ipc_message_t {
    uint64_t                sender;
    char                    name[255];

    uint64_t                size;
    void                    *data;

    struct ipc_message_t    *next;
    struct ipc_message_t    *prev;
} ipc_message_t;

typedef struct ipc_meta_t {
    uint64_t                sender;
    char                    name[255];
    uint64_t                size;

    ipc_message_t           *msg;
} ipc_meta_t;

typedef struct ipc_sub_t {
    char                    topic[255];
    
    struct ipc_sub_t        *next;
    struct ipc_sub_t        *prev;
} ipc_sub_t;

typedef struct ipc_queue_t {
    ipc_message_t   *messages;
    ipc_sub_t       *subscriptions;

    wait_queue_t    *waiters;

    ticketlock_t    lock;
} ipc_queue_t;

typedef struct ipc_syscall_payload {
    char            *message;
    void            *data;
    size_t          size;
} ipc_syscall_payload;

int ipc_send(uint64_t pid, char *message, void *data, size_t size);
int ipc_recieve(ipc_meta_t *meta, void *data);
int ipc_dispatch(char *message, void *data, size_t size);
int ipc_consume(ipc_meta_t *meta);
int ipc_subscribe(char *topic);
int ipc_unsubscribe(char *topic);

#endif // IPC_H