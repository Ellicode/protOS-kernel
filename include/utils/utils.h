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

typedef uint16_t port_t;
typedef uint32_t msr_id_t;

void outb(port_t port, uint8_t value);
uint8_t inb(port_t port);
void outl(port_t port, uint32_t value); 
uint32_t inl(port_t port);
void io_wait();

void hcf();
uint64_t rdmsr(msr_id_t id);
void wrmsr(msr_id_t id, uint64_t value);

#endif // UTILS_H
