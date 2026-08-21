#ifndef TTY_H
#define TTY_H

#include <stdint.h>
#include <stddef.h>

#include "filesystems/devfs.h"

size_t tty_write(struct tty_data_t *tty, const char *buffer);
size_t tty_read(struct tty_data_t *tty, char *buffer);

#endif // TTY_H