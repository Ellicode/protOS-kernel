#include "userspace/scheduler.h"
#include "debug/errors.h"
#include "memory/heap.h"
#include "utils/linked_lists.h"

#include "filesystems/aio.h"

aio_op_t *aio_op_queue = NULL;

int aio_enqueue(file_descriptor_t *fd, int mode) {
    int pid = g_current_thread->process->pid;

    // Don't enqueue duplicate ops for the same fd/mode
    aio_op_t *op = aio_op_queue;
    while (op != NULL) {
        if (op->pid == pid && op->fd == fd && op->mode == mode) {
            return PROTO_OK;
        }
        op = op->next;
    }

    op = k_alloc(sizeof(aio_op_t));

    op->pid = pid;
    op->fd = fd;
    op->mode = mode;

    LL_APPEND(op, aio_op_queue);
    return PROTO_OK;
}

int aio_notify(file_descriptor_t *fd, int mode) {
    int notified = 0;

    aio_op_t *op = aio_op_queue;
    while (op != NULL) {
        aio_op_t *next = op->next;
        if (op->fd == fd && op->mode == mode) {
            ipc_send(op->pid, "proto.aio.readable", NULL, 0);
            LL_UNLINK(op, aio_op_queue);
            k_free(op);
            notified = 1;
        }
        op = next;
    }

    return notified ? PROTO_OK : PROTO_ERR_UNKNOWN;
}

void aio_remove(int pid, file_descriptor_t *fd) {
    aio_op_t *op = aio_op_queue;
    while (op != NULL) {
        aio_op_t *next = op->next;
        if (op->pid == pid && op->fd == fd) {
            LL_UNLINK(op, aio_op_queue);
            k_free(op);
        }
        op = next;
    }
}