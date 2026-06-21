/*
    Tysm Evalyn for the absolutely beautiful freelist PMM integration <3

    MIT License

    Copyright (c) 2026 Evalyn Goemer & EvalynOS Contributors

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    Get the code here ----+
                          |
                          v

    https://git.evalyngoemer.com/evalynOS/evalynOS/src/branch/main/kernel/src/mem/freelist_pmm.c
*/

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "utils/ticket_lock.h"
#include "debug/errors.h"
#include "debug/logger.h"
#include "limine/limine.h"
#include "memory/vmm.h"
#include "globals.h"

#include "memory/freelist_pmm.h"

ticketlock_t freelist_pmm_lock = {0};
freelist_pmm_node_t* freelist_pmm_head = NULL;
uint64_t freelist_pmm_fill_entry = 0;
uint64_t freelist_pmm_fill_offset = 0;

void fpmm_fill(uint64_t pages) {
    uint64_t pages_added = 0;

    while (freelist_pmm_fill_entry < g_lim_memmap->entry_count && pages_added < pages) {
        struct limine_memmap_entry *entry = g_lim_memmap->entries[freelist_pmm_fill_entry];

        if (entry->type != LIMINE_MEMMAP_USABLE) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
            continue;
        }

        uint64_t aligned_start = ALIGN_UP(entry->base, PAGE_SIZE);
        uint64_t aligned_end   = ALIGN_DOWN(entry->base + entry->length, PAGE_SIZE);

        // dont allocate memory in the first 1MB
        if (aligned_start < 0x100000ULL) {
            aligned_start = 0x100000ULL;
        }

        if (aligned_end <= aligned_start) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
            continue;
        }

        if (freelist_pmm_fill_offset == 0) {
            freelist_pmm_fill_offset = aligned_start - entry->base;
        }

        while (entry->base + freelist_pmm_fill_offset + PAGE_SIZE <= aligned_end && pages_added < pages) {
            uint64_t page_addr = entry->base + freelist_pmm_fill_offset;
            freelist_pmm_node_t *node = (freelist_pmm_node_t*)(page_addr + g_lim_hhdm->offset);
            node->next = freelist_pmm_head;
            freelist_pmm_head = node;
            pages_added++;
            freelist_pmm_fill_offset += PAGE_SIZE;
        }

        if (entry->base + freelist_pmm_fill_offset + PAGE_SIZE > aligned_end) {
            freelist_pmm_fill_entry++;
            freelist_pmm_fill_offset = 0;
        }
    }
}

void* fpmm_allocate_page() {
    int lock1r = ticketlock_lock(&freelist_pmm_lock);

    if (freelist_pmm_head == NULL) {
        fpmm_fill(512);
    }
    
    if (freelist_pmm_head == NULL) {
        k_error("out of memory!\n");
    }

    freelist_pmm_node_t* node = freelist_pmm_head;
    freelist_pmm_head = node->next;
    uint64_t phys = (uint64_t)((uintptr_t)node - g_lim_hhdm->offset);

    ticketlock_unlock(&freelist_pmm_lock, lock1r);

    memset(node, 0, PAGE_SIZE);
    return (void *)phys;
}

void fpmm_free_page(void* ptr) {
    int lock1r = ticketlock_lock(&freelist_pmm_lock);

    freelist_pmm_node_t* node = (freelist_pmm_node_t*)(ptr + g_lim_hhdm->offset);
    node->next = freelist_pmm_head;
    freelist_pmm_head = node;

    ticketlock_unlock(&freelist_pmm_lock, lock1r);
}

int fpmm_init() {
    fpmm_fill(512);

    m_pmm_alloc_p = fpmm_allocate_page;
    m_pmm_free_p = fpmm_free_page;

    return PROTO_OK;
}
