#ifndef PROTO_KEYBOARD_H
#define PROTO_KEYBOARD_H

#include <stdint.h>

typedef enum {
    KBD_NONE = 0,

    KBD_ESCAPE,
    KBD_F1,  KBD_F2,  KBD_F3,  KBD_F4,
    KBD_F5,  KBD_F6,  KBD_F7,  KBD_F8,
    KBD_F9,  KBD_F10, KBD_F11, KBD_F12,
    KBD_PRINT_SCREEN,
    KBD_SCROLL_LOCK,
    KBD_PAUSE,

    KBD_GRAVE,
    KBD_1, KBD_2, KBD_3, KBD_4, KBD_5,
    KBD_6, KBD_7, KBD_8, KBD_9, KBD_0,
    KBD_MINUS,
    KBD_EQUALS,
    KBD_BACKSPACE,
    KBD_TAB,

    KBD_Q, KBD_W, KBD_E, KBD_R, KBD_T,
    KBD_Y, KBD_U, KBD_I, KBD_O, KBD_P,
    KBD_LEFT_BRACKET,
    KBD_RIGHT_BRACKET,
    KBD_BACKSLASH,

    KBD_CAPS_LOCK,
    KBD_A, KBD_S, KBD_D, KBD_F, KBD_G,
    KBD_H, KBD_J, KBD_K, KBD_L,
    KBD_SEMICOLON,
    KBD_APOSTROPHE,
    KBD_ENTER,

    KBD_LEFT_SHIFT,
    KBD_Z, KBD_X, KBD_C, KBD_V, KBD_B,
    KBD_N, KBD_M,
    KBD_COMMA,
    KBD_PERIOD,
    KBD_SLASH,
    KBD_RIGHT_SHIFT,

    KBD_LEFT_CTRL,
    KBD_LEFT_SUPER,
    KBD_LEFT_ALT,
    KBD_SPACE,
    KBD_RIGHT_ALT,
    KBD_RIGHT_SUPER,
    KBD_MENU,
    KBD_RIGHT_CTRL,

    KBD_INSERT,
    KBD_HOME,
    KBD_PAGE_UP,
    KBD_DELETE,
    KBD_END,
    KBD_PAGE_DOWN,

    KBD_UP,
    KBD_LEFT,
    KBD_DOWN,
    KBD_RIGHT,

    KBD_NUM_LOCK,
    KBD_KP_DIVIDE,
    KBD_KP_MULTIPLY,
    KBD_KP_MINUS,
    KBD_KP_PLUS,
    KBD_KP_ENTER,
    KBD_KP_PERIOD,
    KBD_KP_0, KBD_KP_1, KBD_KP_2, KBD_KP_3, KBD_KP_4,
    KBD_KP_5, KBD_KP_6, KBD_KP_7, KBD_KP_8, KBD_KP_9,

    KBD_KEYCODE_COUNT
} kbd_keycode_t;

#define KBD_MOD_SHIFT   (1 << 0)
#define KBD_MOD_CTRL    (1 << 1)
#define KBD_MOD_ALT     (1 << 2)
#define KBD_MOD_ALTGR   (1 << 3)
#define KBD_MOD_SUPER   (1 << 4)
#define KBD_MOD_CAPS    (1 << 5)
#define KBD_MOD_NUM     (1 << 6)
#define KBD_MOD_SCROLL  (1 << 7)

typedef struct keyboard_event {
    kbd_keycode_t keycode;
    unsigned char character;
    char          modifiers;
} keyboard_event_t;

#endif // PROTO_KEYBOARD_H
