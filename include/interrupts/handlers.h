#include "interrupts/idt.h"

#ifndef HANDLERS_H
#define HANDLERS_H

#define ISR_EXCEPTION_COUNT 32
#define ISR_IRQ_COUNT       16

enum {
    ISR_EXC_DIVISION_ERROR          = 1,
    ISR_EXC_INVALID_OPCODE          = 6,
    ISR_EXC_DEVICE_UNAVAILABLE      = 7,
    ISR_EXC_DOUBLE_FAULT            = 8,
    ISR_EXC_INVALID_TSS             = 10,
    ISR_EXC_SEG_NOT_PRESENT         = 11,
    ISR_EXC_STACK_SEG_FULT          = 12,
    ISR_EXC_GP_FAULT                = 13,
    ISR_EXC_PAGE_FAULT              = 14
};

enum {
    ISR_IRQ_PIT                     = 0,
    ISR_IRQ_KEYBOARD                = 1
};

extern void* isr_exception_handlers[ISR_EXCEPTION_COUNT];
extern void*             isr_irq_handlers[ISR_IRQ_COUNT];

extern void isr_call_0();
extern void isr_call_6();
extern void isr_call_7();
extern void isr_call_8();
extern void isr_call_10();
extern void isr_call_11();
extern void isr_call_12();
extern void isr_call_13();
extern void isr_call_14();

extern void isr_call_32();
extern void isr_call_33();

void isr_handler(idt_frame_t* frame);

#endif