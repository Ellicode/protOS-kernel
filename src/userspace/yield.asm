bits 64                     ; Target 64-bit mode
section .text               ; Code section

global scheduler_yield      ; Make the function visible to the linker

scheduler_yield:
    push    rbp             ; Establish stack frame
    mov     rbp, rsp
    
    pushf                   ; Push flags to stack
    sti                     ; Enable interrupts
    int     0x20            ; Trigger yield interrupt
    
    popf                    ; Restore flags
    pop     rbp             ; Restore base pointer
    ret                     ; Return to caller