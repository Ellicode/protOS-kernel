#include "memory/vmm.h"

#ifndef UTILS_H
#define UTILS_H

#define ALIGN_UP(x, align) ((((uint64_t) (x)) + ((align) - 1)) & ~((uint64_t) ((align) - 1)))
#define ALIGN_DOWN(x, align) (((uint64_t) (x)) & ~((uint64_t) ((align) - 1)))

#define ROUND_UP(x, round)(((((uint64_t)(x) + (uint64_t)(round) - 1) / (uint64_t)(round) ) * (uint64_t)(round)))

#define PAGE_ALIGN(x) ALIGN_UP(x, PAGE_SIZE)
#define PAGE_ROUND(x) ROUND_UP(x, PAGE_SIZE)

static inline void hcf(void) {
    for (;;) {
        __asm__ ("hlt");
    }
}

// static void panic_test(void) {
//     __asm__ volatile ("ud2"); // Trigger invalid opcode error
// }

#endif // UTILS_H