#include "debug/logger.h"
#include "graphics/tty.h"
#include "interrupts/pic.h"

#include "interrupts/handlers.h"

void isr_handler(idt_frame_t* frame) {
    print_f("Interrupt %d, error=%x, rip=%x\n",
        frame->vector, frame->error_code, frame->rip);

    if (frame->vector >= 32 && frame->vector < 48) {
        eoi((uint8_t)(frame->vector - 32));
    }
}
