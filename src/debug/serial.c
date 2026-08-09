#include "debug/serial.h"
#include "debug/errors.h"
#include "utils/utils.h"
#include "interrupts/pic.h"

// SOURCE CODE FROM THE OSDEV WIKI:
// https://osdev.wiki/wiki/Serial_Ports

int serial_init() {
    outb(PORT_COM1 + 1, 0x01);  // Disable all interrupts
    outb(PORT_COM1 + 3, 0x80);  // Enable DLAB (set baud rate divisor)
    outb(PORT_COM1 + 0, 0x03);  // Set divisor to 3 (lo byte) 38400 baud
    outb(PORT_COM1 + 1, 0x00);  // (hi byte)
    outb(PORT_COM1 + 3, 0x03);  // 8 bits, no parity, one stop bit
    outb(PORT_COM1 + 2, 0xC7);  // Enable FIFO, clear them, with 14-byte threshold
    outb(PORT_COM1 + 4, 0x0B);  // IRQs enabled, RTS/DSR set
    outb(PORT_COM1 + 4, 0x1E);  // Set in loopback mode, test the serial chip
    outb(PORT_COM1 + 0, 0xAE);  // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if (inb(PORT_COM1 + 0) != 0xAE) {
        k_assert(PROTO_ERR_INIT_FAILED);
        return PROTO_ERR_INIT_FAILED;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(PORT_COM1 + 4, 0x0F);

    unmask_irq(4);

    return PROTO_OK;
}

static inline int _is_transmit_empty() {
    return inb(PORT_COM1 + 5) & 0x20;
}

void serial_write(char c) {
    while (_is_transmit_empty() == 0) {};
   
    outb(PORT_COM1, c);
}

char serial_read() {
    while (_is_transmit_empty() == 0);

    return inb(PORT_COM1);
}
