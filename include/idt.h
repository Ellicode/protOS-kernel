#include <stdint.h>

#include "gdt.h"

#ifndef IDT_H
#define IDT_H

enum {
    IDT_GATE_TYPE_INTERRUPT = 0b1110,
    IDT_GATE_TYPE_TRAP = 0b1111
};
typedef uint8_t idt_gate_t;

// IDT Entry attributes

typedef union {
    struct {
        idt_gate_t gate_type       : 4;  // bits 0–3
        uint8_t    zero            : 1;  // bit 4 (must be 0)
        gdt_dpl_t  dpl             : 2;  // bits 5–6
        uint8_t    present         : 1;  // bit 7
    } __attribute__((packed));
    uint8_t value;
} IDTEntryAttributes;
typedef uint8_t idt_attr_t;

// IDT entries

typedef struct {
	uint16_t    isr_low;                 // The lower 16 bits of the ISR's address
	uint16_t    kernel_cs;               // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t	    ist;                     // The IST in the TSS that the CPU will load into RSP; set to zero for now
	idt_attr_t  attributes;              // Type and attributes; see the IDT page
	uint16_t    isr_mid;                 // The higher 16 bits of the lower 32 bits of the ISR's address
	uint32_t    isr_high;                // The higher 32 bits of the ISR's address
	uint32_t    _reserved;               // Set to zero
} __attribute__((packed)) IDTEntry;
typedef IDTEntry idt_entry_t;

// IDTR 

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) IDTR;
typedef IDTR idtr_t;

#define IDT_MAX_DESCRIPTORS 256

// Interrupt frame

typedef struct {
    // Saved registers (in push order, reversed)
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    // Pushed by stub
    uint64_t vector;
    uint64_t error_code;
    // Pushed by CPU automatically
    uint64_t rip, cs, rflags, rsp, ss;
} IDTFrame;
typedef IDTFrame idt_frame_t;

void idt_init();

#endif // IDT_H