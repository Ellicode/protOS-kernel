#include <stdint.h>

uint64_t syscall(uint64_t num, uint64_t arg1, uint64_t arg2, uint64_t arg3) {
    uint64_t ret;
    
    __asm__ __volatile__(
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    
    return ret;
}

int main()
{
    syscall(1, 0, 0, 0);
}