#include <stdint.h>

#include "io.h"
#include "globals.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "debug/logger.h"

#include "drivers/ps2_csa.h"

#include "drivers/ps2.h"

uint8_t shift_pressed  = 0;
uint8_t alt_gr_pressed = 0;

void ps2_isr() {
    if (g_kbd_enable == 1) {
        uint8_t scancode = inb(PS2_DATA_PORT);
        uint8_t key = scancode & 0x7F;

        if (scancode == 0x2A || scancode == 0x36) {
            shift_pressed = 1;
        } else if (scancode == 0xAA || scancode == 0xB6) {
            shift_pressed = 0;
        }

        if (scancode == 0x38) {
            alt_gr_pressed = 1;
        } else if (scancode == 0xB8) {
            alt_gr_pressed = 0;
        }
        
        if (scancode & 0x80) {
            // Handle release
        } else {
            unsigned char ascii;
            if (shift_pressed) {
                ascii = ascii_shift[key];
            } else {
                ascii = ascii_no_shift[key];
            }

            char str[2];
            str[0] = (char)ascii; // Cast to signed char
            str[1] = '\0';     // Null terminator

            print(str);
        }
    }
}

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

    unmask_irq(1);
}

