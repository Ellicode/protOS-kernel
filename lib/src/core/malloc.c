#include <proto/core.h>

heap_item_t *heap_base = NULL;
uint64_t     heap_size = HEAP_MIN_SIZE;

void heap_init() {    
    heap_base = (heap_item_t*)HEAP_VIRTUAL_START;
    heap_base->size  = HEAP_MIN_SIZE - sizeof(HeapItem);
    heap_base->flags = H_FREE;
    heap_base->prev  = NULL;
    heap_base->next  = NULL;
}

void *malloc(size_t size) {
    if (heap_base == NULL) { return NULL; }

    heap_item_t *node = heap_base;
    
    while (node != NULL) {
        if ((node->size >= size) && (node->flags & H_FREE)) {
            uint64_t remainder = node->size - size;

            if (remainder > sizeof(HeapItem)) {
                heap_item_t *new_node = (heap_item_t*)((uint8_t*)node + sizeof(HeapItem) + size);
                new_node->size  = remainder - sizeof(HeapItem);
                new_node->flags = H_FREE;
                new_node->prev  = node;
                new_node->next  = node->next;

                if (node->next != NULL) {
                    node->next->prev = new_node;
                }

                node->next = new_node;
            }

            node->size   = size;
            node->flags &= ~H_FREE;

            void *ptr = (void *)((uint8_t*)node + sizeof(HeapItem));

            memset(ptr, 0, size);

            return ptr;
        }

        node = node->next;
    }
    
    if (heap_size < HEAP_MAX_SIZE) {
        heap_item_t *last = heap_base;
        while (last->next != NULL) {
            last = last->next;
        }

        uint64_t grow_size = size + sizeof(HeapItem);

        heap_item_t *new_block = (heap_item_t*)(HEAP_VIRTUAL_START + heap_size);
        new_block->size  = grow_size - sizeof(HeapItem);
        new_block->flags = H_FREE;
        new_block->prev  = last;
        new_block->next  = NULL;
        last->next = new_block;

        heap_size += grow_size;
    }

    return NULL;
}

int free(void *ptr) {
    if (ptr == NULL) { return PROTO_ERR_INVALID_ARGUMENT; }

    heap_item_t *block = (heap_item_t*)((uint8_t*)ptr - sizeof(HeapItem));

    if (block->flags & H_FREE) { return PROTO_ERR_UNKNOWN; }

    block->flags |= H_FREE;

    // Coalesce forward
    if (block->next != NULL && (block->next->flags & H_FREE)) {
        block->size += sizeof(HeapItem) + block->next->size;
        block->next  = block->next->next;

        if (block->next != NULL) {
            block->next->prev = block;
        }
    }

    // Coalesce backward
    if (block->prev != NULL && (block->prev->flags & H_FREE)) {
        block->prev->size += sizeof(HeapItem) + block->size;
        block->prev->next  = block->next;

        if (block->next != NULL) {
            block->next->prev = block->prev;
        }
    }
}