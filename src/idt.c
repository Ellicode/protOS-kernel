#include "graphics/tty.h"
#include "debug/logger.h"

#include "idt.h"

__attribute__((aligned(0x10))) 
idt_entry_t idt[IDT_MAX_DESCRIPTORS] = { 0 }; // Create an array of IDT entries; aligned for performance
idtr_t idtr;

static idt_entry_t _idt_generate_descriptor(
    void* isr,
    IDTEntryAttributes attributes
) {
    idt_entry_t descriptor = {};
    
    descriptor.isr_low        = (uint64_t)isr & 0xFFFF;
    descriptor.kernel_cs      = GDT_OFFSET_KERNEL_CODE;
    descriptor.ist            = 0;
    descriptor.attributes     = attributes.value;
    descriptor.isr_mid        = ((uint64_t)isr >> 16) & 0xFFFF;
    descriptor.isr_high       = ((uint64_t)isr >> 32) & 0xFFFFFFFF;
    descriptor._reserved      = 0;

    k_debug("idt_entry", "proto.kernel.idt_init");
    print_f(": attr=%x\n", attributes.value);

    return descriptor;
}

void interrupt_handle(idt_frame_t* frame) {
    print_f("Interrupt %d, error=%x, rip=%x\n",
        frame->vector, frame->error_code, frame->rip);
    // __asm__ ("cli");
    // for (;;) {
    //     __asm__ ("hlt");
    // }
}

__attribute__((naked)) void interrupt_stub() {
    __asm__ volatile (
        ".intel_syntax noprefix\n"
        "cld\n"
        "push rax\n"
        "push rbx\n"
        "push rcx\n"
        "push rdx\n"
        "push rsi\n"
        "push rdi\n"
        "push rbp\n"
        "push r8\n"
        "push r9\n"
        "push r10\n"
        "push r11\n"
        "push r12\n"
        "push r13\n"
        "push r14\n"
        "push r15\n"
        "mov rdi, rsp\n"
        "and rsp, -16\n"       // align to 16 bytes before call
        "sub rsp, 8\n"         // keep alignment after call pushes return addr
        "call interrupt_handle\n"
        "add rsp, 8\n"         // undo the sub
        "mov rsp, rdi\n"       // restore unaligned rsp from rdi
        "pop r15\n"
        "pop r14\n"
        "pop r13\n"
        "pop r12\n"
        "pop r11\n"
        "pop r10\n"
        "pop r9\n"
        "pop r8\n"
        "pop rbp\n"
        "pop rdi\n"
        "pop rsi\n"
        "pop rdx\n"
        "pop rcx\n"
        "pop rbx\n"
        "pop rax\n"
        "iretq\n"
        ".att_syntax prefix\n"
    );
}

void idt_init() {
    idtr.base = (uintptr_t)idt;
    idtr.limit = (uint16_t)sizeof(IDTEntry) * IDT_MAX_DESCRIPTORS - 1;

    for (int i = 0; i < 32; i++) {
        idt[i] = _idt_generate_descriptor(interrupt_stub, ((IDTEntryAttributes) {
            IDT_GATE_TYPE_INTERRUPT,
            0,
            GDT_ENTRY_DPL_KERNEL,
            GDT_ENTRY_PRESENT
        })); 
    }

    k_debug("idt base: ", "proto.kernel.idt_init");
    print_f("%x, limit: %x\n", idtr.base, idtr.limit);

    __asm__ volatile ("lidt %0" ::"m"(idtr));
    __asm__ volatile ("sti"); // Enable interrupts
}