#include "drivers/ps2/ps2.h"
#include "interrupts/pic.h"
#include "graphics/console.h"

#include "drivers/ps2/mouse.h"

uint8_t mouse_cycle = 0;
uint8_t mouse_packet[3];

void ps2mouse_init() {
    ps2_wait_write(); 
    ps2_write_cmd(0xD4);
    ps2_wait_write(); 
    ps2_write_data(0xFF);
    
    ps2_wait_read();
    if (ps2_read_data() != ACK) { return; } // ACK (0xFA)
    
    ps2_wait_read(); 
    (void)ps2_read_data(); // BAT result (0xAA)
    ps2_wait_read(); 
    (void)ps2_read_data(); // Device ID (0x00)

    ps2_wait_write(); 
    ps2_write_cmd(0xD4);
    ps2_wait_write(); 
    ps2_write_data(0xF4);
    
    ps2_wait_read();
    if (ps2_read_data() != ACK) { return; } 

    unmask_irq(12);
}

void ps2mouse_read() {
    uint8_t current_byte = ps2_read_data();

    mouse_packet[mouse_cycle] = current_byte;

    if (mouse_cycle == 0 && !(current_byte & 0x08)) {
        return; 
    }

    mouse_cycle++;

    if (mouse_cycle == 3) {
        mouse_cycle = 0;

        uint8_t left_click  = mouse_packet[0] & 0x01;
        uint8_t right_click = mouse_packet[0] & 0x02;
        uint8_t middle_click = mouse_packet[0] & 0x04;

        int16_t x_move = mouse_packet[1];
        int16_t y_move = mouse_packet[2];

        if (mouse_packet[0] & 0x10) { x_move |= 0xFF00; }
        if (mouse_packet[0] & 0x20) { y_move |= 0xFF00; }

        y_move = -y_move; 
    }
}