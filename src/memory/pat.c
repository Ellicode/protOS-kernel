#include <stdint.h>

#include "utils/utils.h"

#include "memory/pat.h"

void pat_init(void) {
    uint64_t pat = rdmsr(IA32_PAT_MSR);

    // WB
    pat &= ~(0xFFULL << (0 * 8));
    pat |=  ((uint64_t)PAT_WB << (0 * 8));

    // WC (write combine)
    pat &= ~(0xFFULL << (1 * 8));
    pat |=  ((uint64_t)PAT_WC << (1 * 8));

    wrmsr(IA32_PAT_MSR, pat);
}