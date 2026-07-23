#ifndef PROTO_EVENTS_H
#define PROTO_EVENTS_H

#include <stddef.h>
#include <stdint.h>

typedef struct ipc_syscall_payload {
    char            *message;
    void            *data;
    size_t          size;
} ipc_syscall_payload;

typedef struct ev_meta_t {
    uint64_t                sender;
    char                    name[255];
    uint64_t                size;

    void                    *msg;
} ev_meta_t;

int send(uint64_t pid, char *message, void *data, size_t size);
int recieve(ev_meta_t *meta, void *data);
int consume(ev_meta_t *meta);
int dispatch(char *message, void *data, size_t size);
int subscribe(char *topic);
int unsubscribe(char *topic);

/*****************************************************************************
 * Special packet types
 *****************************************************************************/

typedef struct mouse_move_packet_t {
    int x;
    int y;

    int vel_x;
    int vel_y;
} mouse_move_packet_t;

#endif