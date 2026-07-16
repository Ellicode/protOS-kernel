#include <stdint.h>
#include <stddef.h>

#include "io.h"
#include "globals.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "debug/logger.h"
#include "drivers/ps2/ps2.h"

#include "drivers/ps2/scansets/csa.h"

#include "drivers/ps2/keyboard.h"

uint8_t shift_pressed  = 0;
uint8_t alt_gr_pressed = 0;

char ps2keyboard_read() {
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
            }  else if (alt_gr_pressed) {
                ascii = ascii_altgr[key];
            } else {
                ascii = ascii_no_shift[key];
            }

            return ascii;
        }
    }
    return '\0';
}

void ps2keyboard_init() {
    ps2_write_data(0xFF); 
    
    if (ps2_read_data() == ACK) {
        unsigned char bat_res = ps2_read_data();
        if (bat_res != 0xAA) {
        }
    }

    ps2_write_data(0xF0);
    if (ps2_read_data() == ACK) {
        ps2_write_data(0x02);  
        ps2_read_data();
    }

    ps2_write_data(0xF4);

    unmask_irq(1);
}

