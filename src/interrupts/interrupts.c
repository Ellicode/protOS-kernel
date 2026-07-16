#include <stdarg.h>

#include "debug/logger.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/ps2/mouse.h"
#include "io.h"
#include "globals.h"
#include "string.h"
#include "userspace/scheduler.h"
#include "userspace/syscalls.h"
#include "filesystems/devfs.h"

#include "interrupts/interrupts.h"

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
    [ISR_IRQ_KEYBOARD]              = isr_call_33,
    [ISR_IRQ_MOUSE]                 = isr_call_44,
};

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

    if (is_frame == 1) {
        p_msg = panic_messages[frame->vector];
    } else {
        p_msg = ename;
    }

    uint64_t cr2;
    asm __volatile__ ("movq %%cr2, %0": "=R"(cr2)); 

    set_cursor(0, 0);
    term_clear_buffer();
    fill_screen(PROTO_BLUE);
    set_color(PROTO_WHITE, PROTO_BLUE);

    print_f("\n\n                                                             '^'");

    print_f("\n\n    The system has been halted by a unexpected error. More debug informations can be found below.\n\n");
   
    print_f("    Error Name:    %s\n", p_msg == 0x0 ? "???" : p_msg);
    
    if (is_frame == 1) {
        print_f("    Error Code:    %x\n", frame->error_code);
        print_f("    Address (RIP): %x\n\n", frame->rip);

        print_f("    CR2:           %x\n", cr2);
        print_f("    CS:            %x\n", frame->cs);
        print_f("    RSP:           %x\n", frame->rsp);
        print_f("    RF:            %x\n", frame->rflags);
    }
    print_f("\n    Feel free to file a bug report with the bug tracker link below:\n");
    print_f("    https://github.com/Ellicode/protOS-kernel/issues\n\n");
    print_f("    ProtOS will now stop. Please reboot your computer manually by holding the power button. We apologize for the inconvenience.\n");

    draw_glyph(GLYPH_KPANIC, g_vga_active_framebuffer->width-70, g_vga_active_framebuffer->height-70);

    for (;;) {
        __asm__ ("hlt");
    }
}

void panic(char *err) {
    _panic_stub(err, 0, NULL);
}

void isr_handler(idt_frame_t* frame) {
    uint64_t vec_buffer = frame->vector; // context switches can switch to a different vector

    if (vec_buffer < ISR_EXCEPTION_COUNT) {
        _panic_stub(NULL, 1, frame);
    } else if (vec_buffer == 32) {
        g_pit_ticks++;
        scheduler_tick(frame);
    } else if (vec_buffer == 33) {
        char c = ps2keyboard_read();
        devfs_node_t *stdin = g_stdin->fs_data;
        stdin_data_t *stdin_data = stdin->extra_data;

        if (stdin_data == NULL) { return; } // 3:< i gotchu
        size_t len = strlen(stdin_data->kbd_buf);
        ipc_dispatch("proto.keydown", &c, 1);

        if (c == '\n') {
            if (stdin->waiters.head != NULL) {
                queue_wake_all(&stdin->waiters);
                print_char('\n');
            }
        } else if (c == '\b') {
            if (len > 0 && stdin->waiters.head != NULL) {
                stdin_data->kbd_buf[len - 1] = '\0';
                print_char('\b');
            }
        } else if (c > 0) {
            if (stdin->waiters.head != NULL) {
                stdin_data->kbd_buf[len] = c;
                stdin_data->kbd_buf[len + 1] = '\0';
                print_char(c);
            }
        }
    } else if (vec_buffer == 44) {
        ps2mouse_read();
    } else if (vec_buffer == 0x80) {
        syscall_handler(frame);
    } else {
        k_debug("BEEP! Interrupt received!");
        #if (PROTO_DEBUG == 1)
            print_f(" %d, error=%x, rip=%x\n", vec_buffer, frame->error_code, frame->rip);
        #endif
    }

    if (vec_buffer >= 32 && vec_buffer < 48) {
        eoi((uint8_t)(frame->vector - 32));
    }
}

int interrupts_enabled() {
    uint64_t rflags;
    asm volatile (
        "pushfq;"
        "pop %0;"
        : "=r" (rflags)
    );
    return (rflags >> 9) & 1;
}

void enable_interrupts() {
    asm __volatile__ ("sti");
}

void disable_interrupts() {
    asm __volatile__ ("cli");
}

void restore_interrupts(int irqs) {
    if (irqs) {
        enable_interrupts();
    } else {
        disable_interrupts();
    }
}