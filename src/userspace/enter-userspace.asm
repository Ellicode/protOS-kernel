global enter_userspace
enter_userspace:  ; enter_userspace(uint64_t entry, uint64_t user_rsp)
    ; rdi = entry point, rsi = user stack pointer
    
    push 0x23        ; SS  — user data segment (GDT index 4 | RPL 3)
    push rsi         ; RSP
    push 0x202       ; RFLAGS
    push 0x1B        ; CS  — user code segment (GDT index 3 | RPL 3)
    push rdi         ; RIP
    iretq