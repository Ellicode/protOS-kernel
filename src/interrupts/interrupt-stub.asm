; Your exception handler here
extern isr_handler

; no error code (push a 0)
%macro ISR_NOERR 2
global isr_call_%1
isr_call_%1:
    cli ; disable interrupts
    push 0
    push %2
    jmp isr_common_stub
%endmacro

; error code
%macro ISR_ERR 2
global isr_call_%1
isr_call_%1:
    cli ;disable interrupts
    push %2
    jmp isr_common_stub
%endmacro

global isr_common_stub
isr_common_stub:
    ; This is the main stub everyone
    ; jumps to.

    ; Push registers
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; move the registers_t struct on 
    ; the stack to arguments which
    ; our ISR handler will use
    mov rdi, rsp

    ; call the C handler
    call isr_handler

    ; pop registers from the stack
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    ; remove error code and num
    add rsp, 16
    iretq

; Map ISR macros to functions
; Each ISR can be referenced from C code as:
;  --------------------------------------------------------------
;  |   C Symbol     | Vector | Description                      |
;  |------------------------------------------------------------|
;  | do_isr0()      |   0     | Divide Error                    |
;  | do_isr1()      |   1     | Debug Exception                 |
;  | do_isr2()      |   2     | Non-Maskable Interrupt          |
;  | do_isr3()      |   3     | Breakpoint                      |
;  | do_isr4()      |   4     | Overflow                        |
;  | do_isr5()      |   5     | Bound Range Exceeded            |
;  | do_isr6()      |   6     | Invalid Opcode                  |
;  | do_isr7()      |   7     | Device Not Available            |
;  | do_isr8()      |   8     | Double Fault                    |
;  | do_isr9()      |   9     | Coprocessor Segment Overrun     |
;  | do_isr10()     |  10     | Invalid TSS                     |
;  | do_isr11()     |  11     | Segment Not Present             |
;  | do_isr12()     |  12     | Stack Fault                     |
;  | do_isr13()     |  13     | General Protection Fault        |
;  | do_isr14()     |  14     | Page Fault                      |
;  | do_isr15()     |  15     | Reserved                        |
;  | do_isr16()     |  16     | x87 Floating-Point Exception    |
;  | do_isr17()     |  17     | Alignment Check                 |
;  | do_isr18()     |  18     | Machine Check                   |
;  |------------------------------------------------------------|

; add more as needed for other ISRs
ISR_NOERR 0, 0
ISR_NOERR 1, 1
ISR_NOERR 2, 2
ISR_NOERR 3, 3
ISR_NOERR 4, 4
ISR_NOERR 5, 5
ISR_NOERR 6, 6
ISR_NOERR 7, 7
ISR_ERR   8, 8
ISR_NOERR 9, 9
ISR_ERR   10, 10
ISR_ERR   11, 11
ISR_ERR   12, 12
ISR_ERR   13, 13 
ISR_ERR   14, 14
ISR_NOERR 15, 15
ISR_NOERR 16, 16
ISR_ERR   17, 17
ISR_NOERR 18, 18

ISR_NOERR 32, 32 ; PIT
ISR_NOERR 33, 33 ; PS2 Keyboard