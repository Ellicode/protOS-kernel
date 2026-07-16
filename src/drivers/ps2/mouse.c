#include "drivers/ps2/ps2.h"
#include "interrupts/pic.h"
#include "graphics/console.h"

#include "drivers/ps2/mouse.h"

void ps2mouse_init() {
    ps2_write_cmd(0xD4);
    ps2_write_data(0xFF);
    if (ps2_read_data() != ACK) { return; }
    (void)ps2_read_data();          // BAT result (0xAA)
    (void)ps2_read_data();          // device ID (0x00)

    ps2_write_cmd(0xD4);
    ps2_write_data(0xF4);
    if (ps2_read_data() != ACK) { return; } 

    unmask_irq(12);
}

void ps2mouse_read() {
    uint8_t b = ps2_read_data();
    print_f("%d", b);
}