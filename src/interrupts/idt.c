#include "graphics/tty.h"
#include "debug/logger.h"
#include "interrupts/handlers.h"

#include "interrupts/idt.h"

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

void idt_init() {
    idtr.base = (uintptr_t)idt;
    idtr.limit = sizeof(idt) - 1;

    IDTEntryAttributes attr = (IDTEntryAttributes){
        IDT_GATE_TYPE_INTERRUPT,
        0,
        GDT_ENTRY_DPL_KERNEL,
        GDT_ENTRY_PRESENT
    };

    idt[0]  = _idt_generate_descriptor(isr_call_0, attr);
    idt[6]  = _idt_generate_descriptor(isr_call_6, attr);
    idt[7]  = _idt_generate_descriptor(isr_call_7, attr);
    idt[8]  = _idt_generate_descriptor(isr_call_8, attr);
    idt[10] = _idt_generate_descriptor(isr_call_10, attr);
    idt[11] = _idt_generate_descriptor(isr_call_11, attr);
    idt[12] = _idt_generate_descriptor(isr_call_12, attr);
    idt[13] = _idt_generate_descriptor(isr_call_13, attr);
    idt[14] = _idt_generate_descriptor(isr_call_14, attr);

    idt[32] = _idt_generate_descriptor(isr_call_32, attr);
    idt[33] = _idt_generate_descriptor(isr_call_33, attr);

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}