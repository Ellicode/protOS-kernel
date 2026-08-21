#include "string.h"
#include "graphics/console.h"

#include "tty.h"

size_t tty_write(struct tty_data_t *tty, const char *buffer) {
    print(buffer);
    return strlen(buffer);
}

size_t tty_read(struct tty_data_t *tty, char *buffer) {
    queue_sleep(&tty->read_waiters, g_current_thread);
    strcpy(buffer, tty->buffer);
    int len = strlen(tty->buffer);
    memset(tty->buffer, 0, sizeof(tty->buffer)); // clean junk ew
    return len;
}