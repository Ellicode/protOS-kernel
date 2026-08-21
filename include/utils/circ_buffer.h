#ifndef CIRC_BUFFER_H
#define CIRC_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Generic byte-oriented circular buffer.
typedef struct {
    char *data;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
} circ_buffer_t;

static inline void circ_buffer_init(circ_buffer_t *cb, char *buffer, size_t capacity) {
    cb->data = buffer;
    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

static inline void circ_buffer_reset(circ_buffer_t *cb) {
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
}

static inline bool circ_buffer_is_empty(const circ_buffer_t *cb) {
    return cb->count == 0;
}

static inline bool circ_buffer_is_full(const circ_buffer_t *cb) {
    return cb->count == cb->capacity;
}

static inline size_t circ_buffer_count(const circ_buffer_t *cb) {
    return cb->count;
}

static inline size_t circ_buffer_space(const circ_buffer_t *cb) {
    return cb->capacity - cb->count;
}

// Push a single byte. Returns true on success, false if the buffer is full.
static inline bool circ_buffer_push(circ_buffer_t *cb, char byte) {
    if (circ_buffer_is_full(cb)) {
        return false;
    }

    cb->data[cb->head] = byte;
    cb->head = (cb->head + 1) % cb->capacity;
    cb->count++;
    return true;
}

// Pop a single byte. Returns true on success, false if the buffer is empty.
static inline bool circ_buffer_pop(circ_buffer_t *cb, char *out) {
    if (circ_buffer_is_empty(cb)) {
        return false;
    }

    *out = cb->data[cb->tail];
    cb->tail = (cb->tail + 1) % cb->capacity;
    cb->count--;
    return true;
}

// Peek at the oldest byte without removing it.
// Returns true on success, false if the buffer is empty.
static inline bool circ_buffer_peek(const circ_buffer_t *cb, char *out) {
    if (circ_buffer_is_empty(cb)) {
        return false;
    }

    *out = cb->data[cb->tail];
    return true;
}

// Push multiple bytes. Returns the number of bytes actually written.
static inline size_t circ_buffer_write(circ_buffer_t *cb, const char *src, size_t len) {
    size_t written = 0;

    while (written < len && circ_buffer_push(cb, src[written])) {
        written++;
    }

    return written;
}

// Pop up to `len` bytes into `dst`. Returns the number of bytes actually read.
static inline size_t circ_buffer_read(circ_buffer_t *cb, char *dst, size_t len) {
    size_t read = 0;

    while (read < len && circ_buffer_pop(cb, &dst[read])) {
        read++;
    }

    return read;
}

#endif // CIRC_BUFFER_H