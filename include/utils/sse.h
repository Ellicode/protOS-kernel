#ifndef SSE_H
#define SSE_H

static __attribute__((naked)) void enable_x87() {
    __asm__ volatile(
        "movq %cr0, %rax;"
        "orq $0x22, %rax;" // sets the MP and the NE bit
        "andq $0xFFFFFFFFFFFFFFF3, %rax;" // clear the EM and TS bit
        "movq %rax, %cr0;"
        "fninit;"
        "ret;"
    );
}

static __attribute__((naked)) void enable_sse() {
    __asm__ volatile(
        "movq %cr4, %rax;"
        "orq $0x600, %rax;" // sets the OSFXSR and the OSXMMEXCPT bit
        "movq %rax, %cr4;"
        "ret;"
    );
}

#endif // SSE_H
