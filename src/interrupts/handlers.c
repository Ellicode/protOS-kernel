#include "debug/logger.h"
#include "graphics/tty.h"
#include "interrupts/pic.h"

#include "interrupts/handlers.h"

void _panic_print(idt_frame_t* frame) {
    __asm__ ("cli");

    print_f("[");
    set_color(PROTO_RED);
    print_f("PANIC");
    set_color(PROTO_WHITE);
    print_f("] (vec%d) %x", frame->vector, frame->error_code);

    for (;;) {
        __asm__ ("hlt");
    }   
}

void isr_handler(idt_frame_t* frame) {
    if (frame->vector < ISR_EXCEPTION_COUNT) {
        _panic_print(frame);
    } else {
        print_f("Interrupt %d, error=%x, rip=%x\n", frame->vector, frame->error_code, frame->rip);
    }

    if (frame->vector >= 32 && frame->vector < 48) {
        eoi((uint8_t)(frame->vector - 32));
    }
}

void* isr_exception_handlers[ISR_EXCEPTION_COUNT] = {
    [ISR_EXC_DIVISION_ERROR]        = isr_call_0,
    [ISR_EXC_INVALID_OPCODE]        = isr_call_6,
    [ISR_EXC_DEVICE_UNAVAILABLE]    = isr_call_7,
    [ISR_EXC_DOUBLE_FAULT]          = isr_call_8,
    [ISR_EXC_INVALID_TSS]           = isr_call_10,
    [ISR_EXC_SEG_NOT_PRESENT]       = isr_call_11,
    [ISR_EXC_STACK_SEG_FULT]        = isr_call_12,
    [ISR_EXC_GP_FAULT]              = isr_call_13,
    [ISR_EXC_PAGE_FAULT]            = isr_call_14
};


void* isr_irq_handlers[ISR_IRQ_COUNT] = {
    [ISR_IRQ_PIT]                   = isr_call_32,
    [ISR_IRQ_KEYBOARD]              = isr_call_33
};