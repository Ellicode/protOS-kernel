#include "io.h"
#include "debug/logger.h"
#include "graphics/tty.h"
#include "interrupts/interrupts.h"

#include "interrupts/pic.h"

void eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND,PIC_EOI);
    }
	outb(PIC1_COMMAND,PIC_EOI);

    __asm__ volatile ("sti");
}

void mask_irq(irq_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    value = inb(port) | (1 << irq);
    outb(port, value);        
}

void unmask_irq(irq_t irq) {
    uint16_t port;
    uint8_t value;

    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }

    value = inb(port) & ~(1 << irq);
    outb(port, value);
    io_wait();

    k_debug("Unmasked IRQ (vector=", "proto.kernel.pic_init");
    print_f("%d)\n", irq);
}

void _pic_enable(uint8_t offset_1, uint8_t offset_2) {
    __asm__ volatile("cli");

    // Save masks
    uint8_t mask1 = inb(PIC1_DATA);
    uint8_t mask2 = inb(PIC2_DATA);

    // Initialize PIC1
    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset_1);
    io_wait();
    outb(PIC1_DATA, 0x04);  // PIC2 at IRQ2
    io_wait();
    outb(PIC1_DATA, ICW4_8086);
    io_wait();

    // Initialize PIC2
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_DATA, offset_2);
    io_wait();
    outb(PIC2_DATA, 0x02);  // cascade identity
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    // Restore masks
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}
void _pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void pic_init() {
    // Disable APIC - set APIC enable bit (bit 11) to 0 in IA32_APIC_BASE MSR
    uint32_t lo;
    uint32_t hi;
    __asm__ volatile("rdmsr" : "=a"(lo), "=d"(hi) : "c"(0x1B));
    lo &= ~(1 << 11); 
    __asm__ volatile("wrmsr" :: "c"(0x1B), "a"(lo), "d"(hi));

    // Switch to PIC mode via IMCR
    outb(0x22, 0x70);
    outb(0x23, 0x01);

    _pic_enable(0x20, 0x28);
}