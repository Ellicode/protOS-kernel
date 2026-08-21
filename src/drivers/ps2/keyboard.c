#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "utils/utils.h"
#include "globals.h"
#include "graphics/console.h"
#include "interrupts/pic.h"
#include "debug/logger.h"
#include "drivers/ps2/ps2.h"
#include "userspace/ipc.h"

#include "drivers/ps2/keycodes.h"

#include "drivers/ps2/keyboard.h"

static uint8_t kbd_modifiers = 0;
static bool    pending_ext   = false;

static inline void set_mod(uint8_t bit, bool on) {
    if (on) {
        kbd_modifiers |= bit;
    } else {
        kbd_modifiers &= ~bit;
    }
}

static inline void dispatch_key_event(kbd_keycode_t keycode, bool release) {
    keyboard_event_t ev = {
        .keycode   = keycode,
        .character = release ? 0 : kbd_to_char(keycode, kbd_modifiers),
        .modifiers = kbd_modifiers,
    };

    ipc_dispatch(
        release ? "proto.keyboard.keyup" : "proto.keyboard.keydown",
        &ev,
        sizeof(ev)
    );
}

kbd_keycode_t ps2keyboard_read_keycode(void) {
    if (!g_kbd_enable) {
        return KBD_NONE;
    }

    uint8_t scancode = inb(PS2_DATA_PORT);

    /* 0xE0 introduces a two-byte extended scancode. */
    if (scancode == 0xE0) {
        pending_ext = true;
        return KBD_NONE;
    }

    bool release = scancode & 0x80;
    uint8_t code = scancode & 0x7F;
    kbd_keycode_t keycode = pending_ext
        ? ps2_ext_scancode_to_keycode[code]
        : ps2_scancode_to_keycode[code];
    pending_ext = false;

    if (keycode == KBD_NONE) {
        return KBD_NONE;
    }

    if (release) {
        switch (keycode) {
            case KBD_LEFT_SHIFT:
            case KBD_RIGHT_SHIFT: set_mod(KBD_MOD_SHIFT, false); break;
            case KBD_LEFT_CTRL:
            case KBD_RIGHT_CTRL:  set_mod(KBD_MOD_CTRL,  false); break;
            case KBD_LEFT_ALT:    set_mod(KBD_MOD_ALT,   false); break;
            case KBD_RIGHT_ALT:   set_mod(KBD_MOD_ALTGR, false); break;
            case KBD_LEFT_SUPER:
            case KBD_RIGHT_SUPER: set_mod(KBD_MOD_SUPER, false); break;
            default: break;
        }

        dispatch_key_event(keycode, true);
        return KBD_NONE;
    }

    switch (keycode) {
        case KBD_LEFT_SHIFT:
        case KBD_RIGHT_SHIFT: set_mod(KBD_MOD_SHIFT, true); break;
        case KBD_LEFT_CTRL:
        case KBD_RIGHT_CTRL:  set_mod(KBD_MOD_CTRL,  true); break;
        case KBD_LEFT_ALT:    set_mod(KBD_MOD_ALT,   true); break;
        case KBD_RIGHT_ALT:   set_mod(KBD_MOD_ALTGR, true); break;
        case KBD_LEFT_SUPER:
        case KBD_RIGHT_SUPER: set_mod(KBD_MOD_SUPER, true); break;
        case KBD_CAPS_LOCK:   set_mod(KBD_MOD_CAPS,    !(kbd_modifiers & KBD_MOD_CAPS));    break;
        case KBD_NUM_LOCK:    set_mod(KBD_MOD_NUM,     !(kbd_modifiers & KBD_MOD_NUM));     break;
        case KBD_SCROLL_LOCK: set_mod(KBD_MOD_SCROLL,  !(kbd_modifiers & KBD_MOD_SCROLL));  break;
        default: break;
    }

    dispatch_key_event(keycode, false);
    return keycode;
}

char ps2keyboard_read(void) {
    kbd_keycode_t keycode = ps2keyboard_read_keycode();
    if (keycode == KBD_NONE) {
        return '\0';
    }
    return kbd_to_char(keycode, kbd_modifiers);
}

void ps2keyboard_init() {
    /* Reset the keyboard. */
    ps2_write_data(0xFF);
    ps2_wait_read();
    if (ps2_read_data() == ACK) {
        ps2_wait_read();
        unsigned char bat_res = ps2_read_data();
        if (bat_res != 0xAA) {
            /* BAT failed; continue anyway. */
        }
    }

    /* Select Scan Code Set 1. */
    ps2_write_data(0xF0);
    ps2_wait_read();
    if (ps2_read_data() == ACK) {
        ps2_write_data(0x01);
        ps2_wait_read();
        ps2_read_data(); /* ACK */
    }

    /* Enable scanning. */
    ps2_write_data(0xF4);
    ps2_wait_read();
    ps2_read_data(); /* ACK */

    unmask_irq(1);
}

