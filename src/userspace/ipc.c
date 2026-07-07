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

        dest_process = g_active_processes->next;
    }

    ipc_queue_t q = dest_process->msg_queue;

    LL_APPEND(msg, q.messages);

    return PROTO_OK;
}

int ipc_get(ipc_message_t **msg) {
    if (g_current_thread == NULL || g_current_thread->process == NULL) { return PROTO_ERR_UNKNOWN; }

    ipc_queue_t q = g_current_thread->process->msg_queue;
    ipc_message_t *top_msg = q.messages;
    LL_UNLINK(top_msg, q.messages);

    *msg = top_msg;

    return PROTO_OK;
}