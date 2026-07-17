#ifndef UTILS_H
#define UTILS_H

#include "memory/vmm.h"

#define ALIGN_UP(x, align) ((((uint64_t) (x)) + ((align) - 1)) & ~((uint64_t) ((align) - 1)))
#define ALIGN_DOWN(x, align) (((uint64_t) (x)) & ~((uint64_t) ((align) - 1)))

#define ROUND_UP(x, round)(((((uint64_t)(x) + (uint64_t)(round) - 1) / (uint64_t)(round) ) * (uint64_t)(round)))

#define PAGE_ALIGN(x) ALIGN_UP(x, PAGE_SIZE)
#define PAGE_ROUND(x) ROUND_UP(x, PAGE_SIZE)

#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

static inline void hcf(void) {
    for (;;) {
        __asm__ ("hlt");
    }
}

static inline uint64_t rdmsr(uint32_t msr_id) {
    uint32_t low;
    uint32_t high;
    // Execute rdmsr instruction, passing msr_id via ECX
    // Output the low 32 bits to 'low' and high 32 bits to 'high'
    asm volatile ("rdmsr" : "=a" (low), "=d" (high) : "c" (msr_id));
    
    return ((uint64_t)high << 32) | low;
}

static inline void wrmsr(uint32_t msr_id, uint64_t msr_value) {
    uint32_t low = (uint32_t)(msr_value & 0xFFFFFFFF);
    uint32_t high = (uint32_t)(msr_value >> 32);
    
    // Inline assembly for wrmsr
    // "c" maps to ECX, "a" maps to EAX, "d" maps to EDX
    __asm__ volatile (
        "wrmsr"
        : 
        : "c" (msr_id), "a" (low), "d" (high)
    );
}


// static void panic_test(void) {
//     __asm__ volatile ("ud2"); // Trigger invalid opcode error
// }

#endif // UTILS_H