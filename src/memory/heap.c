#include "memory/vmm.h"
#include "debug/logger.h"
#include "globals.h"
#include "string.h"

#include "memory/heap.h"

heap_item_t *heap_base = NULL;

uint64_t    heap_size = HEAP_MIN_SIZE;

void heap_dump() {
    #if (PROTO_DEBUG == 1)
    k_debug("Heap dump:\n", "proto.kernel.heap_dump");
    k_debug("heap_size: ", "proto.kernel.heap_dump");
    print_f("%dMB\n", heap_size / (uint64_t)(1024 * 1024));

    heap_item_t *node = heap_base;
    uint64_t heap_used = 0;
    while (node != NULL)
    {
        if (!(node->flags & H_FREE)) {
            heap_used += node->size;
        }
        node = node->next;
    }

    k_debug("heap_used: ", "proto.kernel.heap_dump");
    print_f("%dMB (%d / 100)\n", heap_used / (uint64_t)(1024 * 1024), (heap_used / heap_size)*100);

    k_debug("heap_used (bytes): ", "proto.kernel.heap_dump");
    print_f("%d\n", heap_used);

    #endif
}

void heap_test() {
    char* hello = k_alloc(6);
    memcpy(hello, "Hello", 6);
    k_debug("hello buffer data: ", "proto.kernel.heap_test");
    print_f("%s\n", hello);    
    heap_dump();

    k_free(hello);
}

void heap_init() {
    vmm_map_range(HEAP_VIRTUAL_START, heap_size);
    
    heap_base = (heap_item_t*)HEAP_VIRTUAL_START;
    heap_base->size  = HEAP_MIN_SIZE - sizeof(HeapItem);
    heap_base->flags = H_FREE;
    heap_base->prev  = NULL;
    heap_base->next  = NULL;

    #if (PROTO_DEBUG == 1)
        heap_test();
    #endif

    heap_dump();
}

void *k_alloc(size_t size) {
    if (heap_base == NULL) {
        k_error("k_alloc called before heap_init", "proto.kernel.k_alloc");
        return NULL;
    }

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
        vmm_map_range(HEAP_VIRTUAL_START + heap_size, grow_size);

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

void k_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }

    heap_item_t *block = (heap_item_t*)((uint8_t*)ptr - sizeof(HeapItem));

    if (block->flags & H_FREE) {
        k_error("k_free: double free detected", "proto.kernel.k_free");
        return;
    }

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