#include "debug/logger.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "drivers/ps2.h"
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
    [ISR_IRQ_KEYBOARD]              = isr_call_33
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

void _panic_print(idt_frame_t* frame) {
    char* p_msg = panic_messages[frame->vector];
    uint64_t cr2;
    asm __volatile__ ("movq %%cr2, %0": "=R"(cr2)); 

    set_color(PROTO_BLACK, PROTO_RED);
    print_f("[");
    print_f("PANIC");
    print_f("]");
    set_color(PROTO_WHITE, PROTO_BG);
    print_f(" VEC: %18d, ERR: %18x, %s.\n", frame->vector, frame->error_code, p_msg == 0x0 ? "???" : p_msg);
    print_f("        R8 : %18x, R9 : %18x, R10: %18x, R11: %18x\n", frame->r8, frame->r9, frame->r10, frame->r11);
    print_f("        R12: %18x, R13: %18x, R14: %18x, R15: %18x\n", frame->r12, frame->r13, frame->r14, frame->r15);
    print_f("        RBP: %18x, RDI: %18x, RSI: %18x, RDX: %18x\n", frame->rbp, frame->rdi, frame->rsi, frame->rdx);
    print_f("        RCX: %18x, RBX: %18x, RAX: %18x, CR2: %18x\n", frame->rcx, frame->rbx, frame->rax, cr2);
    print_f("        RIP: %18x, CS : %18x, RF : %18x, RSP: %18x\n", frame->rip, frame->cs, frame->rflags, frame->rsp);
    print_f("        Something really bad has happened 3:");
    for (;;) {
        __asm__ ("hlt");
    }
}

void isr_handler(idt_frame_t* frame) {
    uint64_t vec_buffer = frame->vector; // context switches can switch to a different vector

    if (vec_buffer < ISR_EXCEPTION_COUNT) {
        _panic_print(frame);
    } else if (vec_buffer == 32) {
        g_pit_ticks++;
        scheduler_tick(frame);
    } else if (vec_buffer == 33) {
        char c = get_ps2_scancode();
        devfs_node_t *stdin = g_stdin->fs_data;
        stdin_data_t *stdin_data = stdin->extra_data;

        if (stdin_data == NULL) { return; } // 3:< i gotchu
        size_t len = strlen(stdin_data->kbd_buf);
        if (c == '\n') {
            queue_wake_all(&stdin->waiters);
            print_char('\n');
        } else if (c == '\b') {
            if (len > 0) {
                stdin_data->kbd_buf[len - 1] = '\0';
                print_char('\b');
            }
        } else if (c > 0) {
            stdin_data->kbd_buf[len] = c;
            stdin_data->kbd_buf[len + 1] = '\0';
            ipc_dispatch("proto.keydown", NULL, 0);
            print_char(c);
        }
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