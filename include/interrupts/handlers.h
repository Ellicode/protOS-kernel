#include "interrupts/idt.h"

#ifndef HANDLERS_H
#define HANDLERS_H

#define ISR_EXCEPTION_COUNT 32

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