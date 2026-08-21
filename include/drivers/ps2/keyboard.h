#ifndef PS2_KEYBOARD_H
#define PS2_KEYBOARD_H

#include "drivers/ps2/keycodes.h"

typedef struct keyboard_event {
    kbd_keycode_t keycode;
    unsigned char character;
    uint8_t       modifiers;
} keyboard_event_t;

char ps2keyboard_read();
kbd_keycode_t ps2keyboard_read_keycode();

void ps2keyboard_init();

#endif // PS2_KEYBOARD_H