#include <stdint.h>
#include <stddef.h>

#include "debug/errors.h"
#include "userspace/scheduler.h"
#include "userspace/ipc.h"
#include "memory/heap.h"
#include "string.h"
#include "utils/linked_lists.h"

int ipc_send(uint64_t pid, char *message, void *data, size_t size) {
    if (message == NULL || data == NULL) { return PROTO_ERR_INVALID_ARGUMENT; }

    ipc_message_t *msg = k_alloc(sizeof(ipc_message_t));
    if (msg == NULL) { return PROTO_ERR_OUT_OF_MEMORY; }

    strncpy(msg->name, message, 255);
    msg->data           = data;
    msg->size           = size;

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

    meta->sender    = top_msg->sender;
    meta->size      = top_msg->size;
    meta->msg       = top_msg;

    strcpy(meta->name, top_msg->name);

    memcpy(data, top_msg->data, top_msg->size);

    return PROTO_OK;
}

int ipc_consume(ipc_meta_t *meta) {
    k_free(meta->msg);

    return PROTO_OK;
}

int ipc_dispatch(char *message, void *data, size_t size) {
    if (message == NULL || data == NULL) { return PROTO_ERR_INVALID_ARGUMENT; }

    process_t *proc = g_active_processes;
    int err = PROTO_OK;

    while (proc != NULL)
    {
        int is_subscribed = 0;
        ipc_sub_t *sub = proc->msg_queue.subscriptions;

        while (sub != NULL) {
            if (strncmp(sub->topic, message, 255) == 0) {
                int res = ipc_send(proc->pid, message, data, size);
                if (res != PROTO_OK) {
                    k_assert(res);
                }
            }
            sub = sub->next;
        }
        proc = proc->next;
    }

    return err;
}

int ipc_subscribe(char *topic) {
    if (g_current_thread == NULL || g_current_thread->process == NULL) {
        return PROTO_ERR_UNKNOWN;
    }

    ipc_sub_t *curr = g_current_thread->process->msg_queue.subscriptions;
    while (curr != NULL) {
        if (strncmp(curr->topic, topic, 255) == 0) {
            return PROTO_OK;
        }
        curr = curr->next;
    }

    ipc_sub_t *sub = k_alloc(sizeof(ipc_sub_t));
    if (sub == NULL) { return PROTO_ERR_UNKNOWN; }

    strncpy(sub->topic, topic, 255);

    LL_APPEND(sub, g_current_thread->process->msg_queue.subscriptions);

    return PROTO_OK;
}

int ipc_unsubscribe(char *topic) {
    if (g_current_thread == NULL || g_current_thread->process == NULL) {
        return PROTO_ERR_UNKNOWN;
    }

    ipc_sub_t *sub = g_current_thread->process->msg_queue.subscriptions;

    while (sub != NULL) {
        if (strncmp(sub->topic, topic, 255) == 0) {
            LL_UNLINK(sub, g_current_thread->process->msg_queue.subscriptions);
            k_free(sub);
            break;
        }
        sub = sub->next;
    }

    return PROTO_OK;
}