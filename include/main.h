#ifndef MAIN_H
#define MAIN_H

#define KERNEL_VIRT_BASE 0xffffffff80000000ULL
#define KERNEL_STACK_PAGES 16
#define KERNEL_STACK_VIRT 0xFFFFFF8000000000ULL

static void hcf(void);

#endif // MAIN_H