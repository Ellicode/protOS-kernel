#include <stdint.h>
#include <stddef.h>

#include "debug/errors.h"
#include "userspace/scheduler.h"
#include "userspace/ipc.h"
#include "memory/heap.h"
#include "string.h"
#include "utils/linked_lists.h"

int ipc_send(uint64_t pid, char *message, void *data, size_t size) {
    ipc_message_t *msg = k_alloc(sizeof(ipc_message_t));
    if (msg == NULL) { return PROTO_ERR_OUT_OF_MEMORY; }

    strncpy(msg->name, message, 255);
    msg->data = data;
    msg->size = size;

    if (g_current_thread != NULL && g_current_thread->process != NULL) {
        msg->sender = g_current_thread->process->pid;
    } else {
        msg->sender = 0;
    }

    process_t *dest_process = g_active_processes;
    while (dest_process != NULL) {
        if (dest_process->pid == pid) {
            break;
        }

        dest_process = dest_process->next;
    }
    if (dest_process == NULL) { k_free(msg); return PROTO_ERR_PROCESS_NOT_FOUND; }

    ipc_queue_t *q = &dest_process->msg_queue;

    LL_APPEND(msg, q->messages);
    queue_wake_all(q->waiters);

    return PROTO_OK;
}

int ipc_recieve(ipc_meta_t *meta, void *data) {
    if (meta == NULL || data == NULL) { return PROTO_ERR_INVALID_ARGUMENT; }
    if (!g_current_thread || !g_current_thread->process) { return PROTO_ERR_UNKNOWN; }

    ipc_queue_t *q = &g_current_thread->process->msg_queue;

    while (q->messages == NULL) {
        queue_sleep(q->waiters, g_current_thread);
    }

    ipc_message_t *top_msg = q->messages;
    LL_UNLINK(top_msg, q->messages);

    meta->sender = top_msg->sender;
    meta->size = top_msg->size;
    strcpy(meta->name, top_msg->name);

    memcpy(data, top_msg->data, top_msg->size);

    return PROTO_OK;
}

int ipc_dispatch(char *message, void *data, size_t size) {
    process_t *proc = g_active_processes;
    int err = 0;

    while (proc != NULL)
    {
        int res = ipc_send(proc->pid, message, data, size);
        if (res != PROTO_OK) {
            err = res;
            break;
        }
        proc = proc->next;
    }

    return err;
}