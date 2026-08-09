#include <stdarg.h>

#include "debug/logger.h"
#include "debug/serial.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "drivers/ps2/keyboard.h"
#include "drivers/ps2/mouse.h"
#include "utils/utils.h"
#include "globals.h"
#include "string.h"
#include "userspace/scheduler.h"
#include "userspace/syscalls.h"
#include "filesystems/devfs.h"
#include "interrupts/panic.h"
#include "userspace/syscalls.h"
#include "pit.h"

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
    [ISR_IRQ_COM1]                  = isr_call_36,
    [ISR_IRQ_MOUSE]                 = isr_call_44,
};

// TODO: MOVE TO ANOTHER FILE
void handle_kbd_event(char c) {
    devfs_node_t *stdin = g_stdin->fs_data;
    stdin_data_t *stdin_data = stdin->extra_data;

    if (stdin_data == NULL) { return; } // 3:< i gotchu
    size_t len = strlen(stdin_data->kbd_buf);
    ipc_dispatch("proto.keyboard.keydown", &c, 1);

    if (c == '\n' || c == 0xD) {
        if (stdin->waiters.head != NULL) {
            queue_wake_all(&stdin->waiters);
            print_char('\n');
        }
    } else if (c == '\b' || c == 0x7F) {
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
}

void isr_handler(idt_frame_t* frame) {
    uint64_t vec_buffer = frame->vector; // context switches can switch to a different vector

    if (vec_buffer < ISR_EXCEPTION_COUNT) {
        if (vec_buffer == ISR_EXC_PAGE_FAULT && (frame->cs & 3) == 3) {
            print_f("[");
            set_color(PROTO_RED, PROTO_BG);
            print_f("FAULT");
            set_color(PROTO_WHITE, PROTO_BG);
            print_f("] Segmentation fault @ %x. (cs=%x)\n", frame->rip, frame->cs);
            sys_exit();
        }
        _panic_stub(NULL, 1, frame);
    } else if (vec_buffer == 32) {
        g_pit_ticks++;
        scheduler_tick(frame);
    } else if (vec_buffer == 33) {
        char c = ps2keyboard_read();
        handle_kbd_event(c);
    } else if (vec_buffer == 36) {
        char c = serial_read();
        handle_kbd_event(c);
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
    __asm__ volatile ("pushfq\npop %0" : "=r" (rflags));
    return (rflags >> 9) & 1;
}

void enable_interrupts() {
    __asm__ volatile ("sti");
}

void disable_interrupts() {
    __asm__ volatile ("cli");
}

void restore_interrupts(int irqs) {
    if (irqs) {
        enable_interrupts();
    } else {
        disable_interrupts();
    }
}
