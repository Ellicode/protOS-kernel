#include <stdint.h>
#include <stddef.h>

#ifndef HEAP_H
#define HEAP_H

#define HEAP_VIRTUAL_START 0xFFFFA00000000000

#define HEAP_MIN_SIZE      0x0000000000400000 // 4 MiB
#define HEAP_MAX_SIZE      0x0000100000000000 // A lot of GiB

typedef struct HeapItem {
    size_t size;
    uint64_t flags;
    struct HeapItem* prev;
    struct HeapItem* next;
} __attribute__((packed)) HeapItem;
typedef HeapItem heap_item_t;

extern heap_item_t *heap_base;

void heap_init();
void *k_alloc(size_t size);
void k_free(void *ptr);

#define H_FREE      (1ULL << 0)
#define H_WRITE     (1ULL << 1)
#define H_USER      (1ULL << 2)

#endif // HEAP_H