#include "proto.h"

int ipc_send(uint64_t pid, char *message, void *data, size_t size) {
    ipc_syscall_payload payload = (ipc_syscall_payload) {
        .message    = message,
        .data       = data,
        .size       = size
    }; 

    return syscall(SYS_SEND, pid, (uint64_t)&payload, 0);
}

int ipc_recieve(ipc_meta_t *meta, void *data) {
    return syscall(SYS_RECIEVE, (uint64_t)meta, (uint64_t)data, 0);
}
