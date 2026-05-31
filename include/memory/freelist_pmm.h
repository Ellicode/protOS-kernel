/*
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
*/

#include <stdint.h>

#include "utils/utils.h"
#include "memory/vmm.h"

#ifndef FREELIST_PMM_H
#define FREELIST_PMM_H

typedef struct freelist_pmm_node {
    struct freelist_pmm_node* next;
} freelist_pmm_node_t;

extern freelist_pmm_node_t* freelist_pmm_head;
extern uint64_t freelist_pmm_fill_entry;
extern uint64_t freelist_pmm_fill_offset;

void* fpmm_allocate_page();
void  fpmm_free_page(void* ptr);
int   fpmm_init();

#endif // FREELIST_PMM_H