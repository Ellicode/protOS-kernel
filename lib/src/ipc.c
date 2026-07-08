#include "proto.h"

int ipc_send(uint64_t pid, char *message, void *data, size_t size) {
    ipc_syscall_payload *payload = malloc(sizeof(ipc_syscall_payload));
    payload->message    = message;
    payload->data       = data;
    payload->size       = size;

    return syscall(SYS_SEND, pid, (uint64_t)payload, 0);
}

int ipc_recieve(ipc_message_t **buf) {
    return syscall(SYS_RECIEVE, (uint64_t)buf, 0, 0);
}
