#include <stdint.h>

#include "io.h"

#include "drivers/ps2.h"

void ps2_init() {
    outb(PS2_COMMAND_PORT, 0xAD);
    io_wait();
    outb(PS2_COMMAND_PORT, 0xA7);
    io_wait();

    inb(PS2_DATA_PORT);
    io_wait();

    outb(PS2_COMMAND_PORT, 0x20);
    io_wait();

    uint8_t status = inb(PS2_DATA_PORT);
    io_wait();
    status |=1;
    status &= ~(1 << 1);

    outb(PS2_COMMAND_PORT, 0x60);
    io_wait();
    outb(PS2_DATA_PORT, status);
    io_wait();

    outb(PS2_COMMAND_PORT, 0xAE);
    io_wait();

    outb(PS2_DATA_PORT, 0xFF);
    io_wait();

    int timeout = 1000;
    uint8_t response;
    while (timeout--) {
        if ((inb(PS2_STATUS_PORT) & 1) != 0) {
            response = inb(PS2_DATA_PORT);
            if (response == 0xFA) { break; }
        }
        // pit_sleep_ms(1);
    }

    outb(PS2_DATA_PORT, 0xF0);
    io_wait();
    outb(PS2_DATA_PORT, 0x02);
    io_wait();

    while ((inb(PS2_STATUS_PORT) & 1) != 0) {
        inb(PS2_DATA_PORT);
    }
}
