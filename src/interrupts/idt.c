#include "graphics/console.h"
#include "debug/logger.h"
#include "interrupts/interrupts.h"
#include "userspace/syscalls.h"

#include "interrupts/idt.h"

__attribute__((aligned(0x10))) 
idt_entry_t idt[IDT_MAX_DESCRIPTORS] = { 0 }; // Create an array of IDT entries; aligned for performance
idtr_t idtr;

static idt_entry_t _idt_generate_descriptor(
    void* isr,
    IDTEntryAttributes attributes,
    uint8_t vector
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
    #if (PROTO_DEBUG == 1)
        print_f(": vec=%d, attr=%x\n", vector, attributes.value);
    #endif

    return descriptor;
}

void idt_init() {
    idtr.base = (uintptr_t)idt;
    idtr.limit = sizeof(idt) - 1;

    IDTEntryAttributes attr = (IDTEntryAttributes){{
        IDT_GATE_TYPE_INTERRUPT,
        0,
        GDT_ENTRY_DPL_KERNEL,
        GDT_ENTRY_PRESENT
    }};

    for (uint64_t i = 0; i < ISR_EXCEPTION_COUNT; i++) {
        if (isr_exception_handlers[i] == 0) { continue; }
        idt[i] = _idt_generate_descriptor(isr_exception_handlers[i], attr, i);
    }

    for (uint64_t i = 0; i < ISR_IRQ_COUNT; i++) {
        if (isr_irq_handlers[i] == 0) { continue; }
        uint8_t vec = ISR_EXCEPTION_COUNT + i;
        idt[vec] = _idt_generate_descriptor(isr_irq_handlers[i], attr, vec);
    }

    IDTEntryAttributes attr_user = (IDTEntryAttributes){{
        IDT_GATE_TYPE_INTERRUPT,
        0,
        GDT_ENTRY_DPL_USER,
        GDT_ENTRY_PRESENT
    }};

    idt[128] = _idt_generate_descriptor(syscall_handler, attr_user, 128);

    __asm__ volatile ("lidt %0" : : "m"(idtr));
}