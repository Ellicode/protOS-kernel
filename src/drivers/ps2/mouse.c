#include "userspace/process.h"
#include "drivers/ps2/ps2.h"
#include "interrupts/pic.h"
#include "graphics/console.h"
#include "globals.h"
#include "utils/utils.h"

#include "drivers/ps2/mouse.h"

uint8_t mouse_cycle     = 0;
uint8_t mouse_packet[3];
int abs_x               = 0;
int abs_y               = 0;
uint8_t prev_left       = 0;
uint8_t prev_right      = 0;
uint8_t prev_middle     = 0;

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

        int8_t dx = (int8_t)mouse_packet[1];
        int8_t dy = (int8_t)mouse_packet[2];

        int rel_x = (int)dx;
        int rel_y = (int)dy;

        int screen_width = g_vga_active_framebuffer->width;
        int screen_height = g_vga_active_framebuffer->height;

        abs_x = CLAMP(abs_x + rel_x, 0, screen_width - 1);
        abs_y = CLAMP(abs_y - rel_y, 0, screen_height - 1);

        int prev_buttons[3] = {prev_left, prev_right, prev_middle};
        int curr_buttons[3] = {left_click, right_click, middle_click};
        int button_ids[3]   = {1, 2, 3}; 

        for (int i = 0; i < 3; i++) {
            if (prev_buttons[i] != curr_buttons[i]) {
                char* event = (curr_buttons[i] == 1) ? "proto.mouse.down" : "proto.mouse.up";
                ipc_dispatch(event, &button_ids[i], 1);
            }
        }

        if (rel_x != 0 || rel_y != 0) {
            mouse_move_packet_t pkt = (mouse_move_packet_t) {
                .x = abs_x,
                .y = abs_y,
                .vel_x = rel_x,
                .vel_y = rel_y
            };

            ipc_dispatch("proto.mouse.move", &pkt, sizeof(mouse_move_packet_t));
        }

        prev_left = left_click;
        prev_right = right_click;
        prev_middle = middle_click;
    }
}