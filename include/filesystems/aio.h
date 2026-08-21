#ifndef ASYNC_IO_H
#define ASYNC_IO_H

#include "filesystems/vfs.h"

typedef struct aio_op_t {
    int                 pid;
    file_descriptor_t   *fd;
    int                 mode;

    struct aio_op_t     *next;
    struct aio_op_t     *prev;
} aio_op_t;

enum {
    AIO_OP_READ = 0,
    AIO_OP_WRITE = 1,
};

int aio_enqueue(file_descriptor_t *fd, int mode);
int aio_notify(file_descriptor_t *fd, int mode);
void aio_remove(int pid, file_descriptor_t *fd);

#endif // ASYNC_IO