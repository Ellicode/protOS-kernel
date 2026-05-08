#include "io.h"
#include "debug/logger.h"
#include "graphics/tty.h"

#include "interrupts/pic.h"

void eoi(uint8_t irq) {
    if(irq >= 8) {
        outb(PIC2_COMMAND,PIC_EOI);
    }
	outb(PIC1_COMMAND,PIC_EOI);
}

void mask_irq(uint8_t irq) {
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

void unmask_irq(uint8_t irq) {
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
}

void _pic_enable(uint64_t offset_1, uint64_t offset_2) {
    __asm__ volatile("cli");

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();

    outb(PIC1_DATA, offset_1);
    outb(PIC2_DATA, offset_2);
    io_wait();

    outb(PIC1_DATA, 0x04);
    outb(PIC2_DATA, 0x02);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);
    io_wait();
}

void _pic_disable(void) {
    outb(PIC1_DATA, 0xff);
    outb(PIC2_DATA, 0xff);
}

void pic_init() {
    _pic_enable(0x20, 0x28);
    
    uint8_t unmasked_irqs[] = { 0, 1 };
    uint8_t length = sizeof(unmasked_irqs) / sizeof(unmasked_irqs[0]); // Calculate array size

    for (uint8_t irq; irq < length; irq++) {
        unmask_irq(unmasked_irqs[irq]);
        k_debug("Unmasked IRQ (vector=", "proto.kernel.pic_init");
        print_f("%d)\n", unmasked_irqs[irq]);
    }
}