#include "proto.h"

int send(uint64_t pid, char *message, void *data, size_t size) {
    ipc_syscall_payload payload = (ipc_syscall_payload) {
        .message    = message,
        .data       = data,
        .size       = size
    }; 

    return syscall(SYS_SEND, pid, (uint64_t)&payload, 0);
}

int recieve(ipc_meta_t *meta, void *data) {
    return syscall(SYS_RECIEVE, (uint64_t)meta, (uint64_t)data, 0);
}

int consume(ipc_meta_t *meta) {
    return syscall(SYS_CONSUME, (uint64_t)meta, 0, 0);
}

int dispatch(char *message, void *data, size_t size) {
    return syscall(SYS_DISPATCH, (uint64_t)message, (uint64_t)data, size);
}

int subscribe(char *topic) {
    return syscall(SYS_SUBSCRIBE, (uint64_t)topic, 0, 0);
}

int unsubscribe(char *topic) {
    return syscall(SYS_UNSUBSCRIBE, (uint64_t)topic, 0, 0);
}
