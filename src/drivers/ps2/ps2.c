#include "io.h"
#include "interrupts/pic.h"

#include "drivers/ps2/ps2.h"

void ps2_wait_write() {
    while (inb(PS2_STATUS_PORT) & (1 << 1)) { ;; }
}

void ps2_wait_read() {
    while (!(inb(PS2_STATUS_PORT) & (1 << 0))) { ;; }
}

void ps2_write_cmd(unsigned char cmd) {
    ps2_wait_write();
    outb(PS2_COMMAND_PORT, cmd);
}

void ps2_write_data(unsigned char data) {
    ps2_wait_write();
    outb(PS2_DATA_PORT, data);
}

unsigned char ps2_read_data() {
    ps2_wait_read();
    return inb(PS2_DATA_PORT);
}

void ps2_init() {
    ps2_write_cmd(0xAD); // Disable Port 1 (Keyboard)
    ps2_write_cmd(0xA7); // Disable Port 2 (Mouse)

    while (inb(PS2_STATUS_PORT) & (1 << 0)) {
        inb(PS2_DATA_PORT);
    }

    ps2_write_cmd(0x20);
    unsigned char status = ps2_read_data();

    status |= (1 << 0);   // Enable Keyboard Interrupts
    status |= (1 << 1);  // Enable Mouse Interrupts

    ps2_write_cmd(0x60);
    ps2_write_data(status);

    ps2_write_cmd(0xAE); // Re-enable keyboard
    ps2_write_cmd(0xA8); // Re-enable mouse

    unmask_irq(2);
}
