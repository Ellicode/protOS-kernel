#include <stddef.h>

#include "interrupts/interrupts.h"
#include "graphics/console.h"
#include "utils/utils.h"
#include "debug/backtrace.h"
char* panic_messages[ISR_EXCEPTION_COUNT] = {
    [ISR_EXC_DIVISION_ERROR]        = "Division Error",
    [ISR_EXC_INVALID_OPCODE]        = "Invalid Opcode",
    [ISR_EXC_DEVICE_UNAVAILABLE]    = "Device Unavailable",
    [ISR_EXC_DOUBLE_FAULT]          = "Double Fault",
    [ISR_EXC_INVALID_TSS]           = "Invalid TSS",
    [ISR_EXC_SEG_NOT_PRESENT]       = "Segment not Present",
    [ISR_EXC_STACK_SEG_FULT]        = "Stack-Segment Fault",
    [ISR_EXC_GP_FAULT]              = "General Protection Fault",
    [ISR_EXC_PAGE_FAULT]            = "Page fault"
};

void _panic_stub(char *ename, int is_frame, idt_frame_t *frame) {
    char* p_msg; 

    if (is_frame == 1 && frame != NULL) {
        p_msg = panic_messages[frame->vector];
    } else {
        p_msg = ename;
    }

    uint64_t cr2;
    asm __volatile__ ("movq %%cr2, %0": "=R"(cr2)); 

    set_cursor(0, 0);
    term_clear_buffer();
    fill_screen(0x0000FF);
    set_color(PROTO_WHITE, 0x0000FF);

    print_f("\n\n");
    for (int i = 0; i < (g_term_cols-3)/2; i++) { print_f(" "); }
    print_f("'^'");

    print_f("\n\n    The system has been halted by a unexpected error. More debug information can be found below.\n\n");
   
    print_f("    Error Name:    %s\n", p_msg == NULL ? "???" : p_msg);
    
    if (is_frame == 1) {
        print_f("    Error Code:    %x\n", frame->error_code);
        print_f("    Address (RIP): %x\n\n", frame->rip);

        print_f("    CR2:           %x\n", cr2);
        print_f("    CS:            %x\n", frame->cs);
        print_f("    RSP:           %x\n", frame->rsp);
        print_f("    RF:            %x\n\n", frame->rflags);

        print_f("    Backtrace:\n\n");

        unwind_stack(__builtin_frame_address(0));
    }
    print_f("\n    Feel free to file a bug report with the bug tracker link below:\n");
    print_f("    https://github.com/Ellicode/protOS-kernel/issues\n\n");
    print_f("    ProtOS will now stop. Please reboot your computer manually by holding the power button.");

    draw_glyph(GLYPH_KPANIC, g_framebuffer->width-70, g_framebuffer->height-70);

    hcf();
}

void panic(char *err) {
    _panic_stub(err, 0, NULL);
}
