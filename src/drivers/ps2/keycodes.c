#include "drivers/ps2/keycodes.h"

/* PS/2 Scan Code Set 1, non-extended make codes. */
const kbd_keycode_t ps2_scancode_to_keycode[256] = {
    [0x01] = KBD_ESCAPE,

    [0x02] = KBD_1, [0x03] = KBD_2, [0x04] = KBD_3, [0x05] = KBD_4,
    [0x06] = KBD_5, [0x07] = KBD_6, [0x08] = KBD_7, [0x09] = KBD_8,
    [0x0A] = KBD_9, [0x0B] = KBD_0,
    [0x0C] = KBD_MINUS,
    [0x0D] = KBD_EQUALS,
    [0x0E] = KBD_BACKSPACE,
    [0x0F] = KBD_TAB,

    [0x10] = KBD_Q, [0x11] = KBD_W, [0x12] = KBD_E, [0x13] = KBD_R,
    [0x14] = KBD_T, [0x15] = KBD_Y, [0x16] = KBD_U, [0x17] = KBD_I,
    [0x18] = KBD_O, [0x19] = KBD_P,
    [0x1A] = KBD_LEFT_BRACKET,
    [0x1B] = KBD_RIGHT_BRACKET,
    [0x1C] = KBD_ENTER,
    [0x1D] = KBD_LEFT_CTRL,

    [0x1E] = KBD_A, [0x1F] = KBD_S, [0x20] = KBD_D, [0x21] = KBD_F,
    [0x22] = KBD_G, [0x23] = KBD_H, [0x24] = KBD_J, [0x25] = KBD_K,
    [0x26] = KBD_L,
    [0x27] = KBD_SEMICOLON,
    [0x28] = KBD_APOSTROPHE,
    [0x29] = KBD_GRAVE,
    [0x2A] = KBD_LEFT_SHIFT,
    [0x2B] = KBD_BACKSLASH,

    [0x2C] = KBD_Z, [0x2D] = KBD_X, [0x2E] = KBD_C, [0x2F] = KBD_V,
    [0x30] = KBD_B, [0x31] = KBD_N, [0x32] = KBD_M,
    [0x33] = KBD_COMMA,
    [0x34] = KBD_PERIOD,
    [0x35] = KBD_SLASH,
    [0x36] = KBD_RIGHT_SHIFT,
    [0x37] = KBD_KP_MULTIPLY,
    [0x38] = KBD_LEFT_ALT,
    [0x39] = KBD_SPACE,
    [0x3A] = KBD_CAPS_LOCK,

    [0x3B] = KBD_F1,  [0x3C] = KBD_F2,  [0x3D] = KBD_F3,  [0x3E] = KBD_F4,
    [0x3F] = KBD_F5,  [0x40] = KBD_F6,  [0x41] = KBD_F7,  [0x42] = KBD_F8,
    [0x43] = KBD_F9,  [0x44] = KBD_F10,

    [0x45] = KBD_NUM_LOCK,
    [0x46] = KBD_SCROLL_LOCK,

    [0x47] = KBD_KP_7, [0x48] = KBD_KP_8, [0x49] = KBD_KP_9,
    [0x4A] = KBD_KP_MINUS,
    [0x4B] = KBD_KP_4, [0x4C] = KBD_KP_5, [0x4D] = KBD_KP_6,
    [0x4E] = KBD_KP_PLUS,
    [0x4F] = KBD_KP_1, [0x50] = KBD_KP_2, [0x51] = KBD_KP_3,
    [0x52] = KBD_KP_0,
    [0x53] = KBD_KP_PERIOD,

    [0x57] = KBD_F11,
    [0x58] = KBD_F12,
};

/* PS/2 Scan Code Set 1, 0xE0-prefixed extended make codes. */
const kbd_keycode_t ps2_ext_scancode_to_keycode[256] = {
    [0x1C] = KBD_KP_ENTER,
    [0x1D] = KBD_RIGHT_CTRL,
    [0x35] = KBD_KP_DIVIDE,
    [0x37] = KBD_PRINT_SCREEN,
    [0x38] = KBD_RIGHT_ALT,
    [0x47] = KBD_HOME,
    [0x48] = KBD_UP,
    [0x49] = KBD_PAGE_UP,
    [0x4B] = KBD_LEFT,
    [0x4D] = KBD_RIGHT,
    [0x4F] = KBD_END,
    [0x50] = KBD_DOWN,
    [0x51] = KBD_PAGE_DOWN,
    [0x52] = KBD_INSERT,
    [0x53] = KBD_DELETE,
    [0x5B] = KBD_LEFT_SUPER,
    [0x5C] = KBD_RIGHT_SUPER,
    [0x5D] = KBD_MENU,
};

/* Best-effort reverse map for printable ASCII -> keycode. */
const kbd_keycode_t ascii_to_kbd_keycode[128] = {
    [8]   = KBD_BACKSPACE,
    [9]   = KBD_TAB,
    [10]  = KBD_ENTER,
    [13]  = KBD_ENTER,
    [27]  = KBD_ESCAPE,
    [32]  = KBD_SPACE,

    [33]  = KBD_1,           /* ! */
    [34]  = KBD_PERIOD,      /* " */
    [35]  = KBD_2,           /* # */
    [36]  = KBD_3,           /* $ */
    [37]  = KBD_4,           /* % */
    [38]  = KBD_7,           /* & */
    [39]  = KBD_COMMA,       /* ' */
    [40]  = KBD_9,           /* ( */
    [41]  = KBD_0,           /* ) */
    [42]  = KBD_8,           /* * */
    [43]  = KBD_EQUALS,      /* + */
    [44]  = KBD_COMMA,       /* , */
    [45]  = KBD_MINUS,       /* - */
    [46]  = KBD_PERIOD,      /* . */
    [47]  = KBD_GRAVE,       /* / */

    [48]  = KBD_0, [49]  = KBD_1, [50]  = KBD_2, [51]  = KBD_3,
    [52]  = KBD_4, [53]  = KBD_5, [54]  = KBD_6, [55]  = KBD_7,
    [56]  = KBD_8, [57]  = KBD_9,

    [58]  = KBD_SEMICOLON,   /* : */
    [59]  = KBD_SEMICOLON,   /* ; */
    [60]  = KBD_COMMA,       /* < */
    [61]  = KBD_EQUALS,      /* = */
    [62]  = KBD_PERIOD,      /* > */
    [63]  = KBD_6,           /* ? */
    [64]  = KBD_2,           /* @ */

    [65]  = KBD_A, [66]  = KBD_B, [67]  = KBD_C, [68]  = KBD_D,
    [69]  = KBD_E, [70]  = KBD_F, [71]  = KBD_G, [72]  = KBD_H,
    [73]  = KBD_I, [74]  = KBD_J, [75]  = KBD_K, [76]  = KBD_L,
    [77]  = KBD_M, [78]  = KBD_N, [79]  = KBD_O, [80]  = KBD_P,
    [81]  = KBD_Q, [82]  = KBD_R, [83]  = KBD_S, [84]  = KBD_T,
    [85]  = KBD_U, [86]  = KBD_V, [87]  = KBD_W, [88]  = KBD_X,
    [89]  = KBD_Y, [90]  = KBD_Z,

    [91]  = KBD_9,           /* [ */
    [92]  = KBD_GRAVE,       /* \ */
    [93]  = KBD_0,           /* ] */
    [94]  = KBD_LEFT_BRACKET,/* ^ */
    [95]  = KBD_MINUS,       /* _ */
    [96]  = KBD_LEFT_BRACKET,/* ` */

    [97]  = KBD_A, [98]  = KBD_B, [99]  = KBD_C, [100] = KBD_D,
    [101] = KBD_E, [102] = KBD_F, [103] = KBD_G, [104] = KBD_H,
    [105] = KBD_I, [106] = KBD_J, [107] = KBD_K, [108] = KBD_L,
    [109] = KBD_M, [110] = KBD_N, [111] = KBD_O, [112] = KBD_P,
    [113] = KBD_Q, [114] = KBD_R, [115] = KBD_S, [116] = KBD_T,
    [117] = KBD_U, [118] = KBD_V, [119] = KBD_W, [120] = KBD_X,
    [121] = KBD_Y, [122] = KBD_Z,

    [123] = KBD_7,           /* { */
    [124] = KBD_GRAVE,       /* | */
    [125] = KBD_8,           /* } */
    [126] = KBD_RIGHT_BRACKET/* ~ */
};

static const unsigned char csa_unshifted[KBD_KEYCODE_COUNT] = {
    [KBD_ESCAPE]    = 0x1B,
    [KBD_GRAVE]     = '/',

    [KBD_1] = '1', [KBD_2] = '2', [KBD_3] = '3', [KBD_4] = '4',
    [KBD_5] = '5', [KBD_6] = '6', [KBD_7] = '7', [KBD_8] = '8',
    [KBD_9] = '9', [KBD_0] = '0',

    [KBD_MINUS]     = '-',
    [KBD_EQUALS]    = '=',
    [KBD_BACKSPACE] = '\b',
    [KBD_TAB]       = '\t',

    [KBD_Q] = 'q', [KBD_W] = 'w', [KBD_E] = 'e', [KBD_R] = 'r',
    [KBD_T] = 't', [KBD_Y] = 'y', [KBD_U] = 'u', [KBD_I] = 'i',
    [KBD_O] = 'o', [KBD_P] = 'p',

    [KBD_LEFT_BRACKET]  = '^',
    [KBD_RIGHT_BRACKET] = 'c',      /* ç fallback */
    [KBD_BACKSLASH]     = 0,

    [KBD_A] = 'a', [KBD_S] = 's', [KBD_D] = 'd', [KBD_F] = 'f',
    [KBD_G] = 'g', [KBD_H] = 'h', [KBD_J] = 'j', [KBD_K] = 'k',
    [KBD_L] = 'l',

    [KBD_SEMICOLON]     = ';',
    [KBD_APOSTROPHE]    = 'e',      /* è fallback */
    [KBD_ENTER]         = '\n',

    [KBD_Z] = 'z', [KBD_X] = 'x', [KBD_C] = 'c', [KBD_V] = 'v',
    [KBD_B] = 'b', [KBD_N] = 'n', [KBD_M] = 'm',

    [KBD_COMMA]     = ',',
    [KBD_PERIOD]    = '.',
    [KBD_SLASH]     = 'e',          /* é fallback */

    [KBD_SPACE]     = ' ',

    [KBD_KP_0] = '0', [KBD_KP_1] = '1', [KBD_KP_2] = '2', [KBD_KP_3] = '3',
    [KBD_KP_4] = '4', [KBD_KP_5] = '5', [KBD_KP_6] = '6', [KBD_KP_7] = '7',
    [KBD_KP_8] = '8', [KBD_KP_9] = '9',
    [KBD_KP_PERIOD]  = '.',
    [KBD_KP_MINUS]   = '-',
    [KBD_KP_PLUS]    = '+',
    [KBD_KP_MULTIPLY]= '*',
    [KBD_KP_DIVIDE]  = '/',
    [KBD_KP_ENTER]   = '\n',
};

static const unsigned char csa_shifted[KBD_KEYCODE_COUNT] = {
    [KBD_GRAVE]     = '\\',

    [KBD_1] = '!', [KBD_2] = '@', [KBD_3] = '#', [KBD_4] = '$',
    [KBD_5] = '%', [KBD_6] = '?', [KBD_7] = '&', [KBD_8] = '*',
    [KBD_9] = '(', [KBD_0] = ')',

    [KBD_MINUS]     = '_',
    [KBD_EQUALS]    = '+',
    [KBD_BACKSPACE] = '\b',
    [KBD_TAB]       = '\t',

    [KBD_Q] = 'Q', [KBD_W] = 'W', [KBD_E] = 'E', [KBD_R] = 'R',
    [KBD_T] = 'T', [KBD_Y] = 'Y', [KBD_U] = 'U', [KBD_I] = 'I',
    [KBD_O] = 'O', [KBD_P] = 'P',

    [KBD_LEFT_BRACKET]  = '{',
    [KBD_RIGHT_BRACKET] = 'C',      /* Ç fallback */

    [KBD_A] = 'A', [KBD_S] = 'S', [KBD_D] = 'D', [KBD_F] = 'F',
    [KBD_G] = 'G', [KBD_H] = 'H', [KBD_J] = 'J', [KBD_K] = 'K',
    [KBD_L] = 'L',

    [KBD_SEMICOLON]     = ':',
    [KBD_APOSTROPHE]    = 'E',      /* È fallback */
    [KBD_ENTER]         = '\n',

    [KBD_Z] = 'Z', [KBD_X] = 'X', [KBD_C] = 'C', [KBD_V] = 'V',
    [KBD_B] = 'B', [KBD_N] = 'N', [KBD_M] = 'M',

    [KBD_COMMA]     = '\'',
    [KBD_PERIOD]    = '"',
    [KBD_SLASH]     = 'E',          /* É fallback */

    [KBD_SPACE]     = ' ',

    [KBD_KP_0] = '0', [KBD_KP_1] = '1', [KBD_KP_2] = '2', [KBD_KP_3] = '3',
    [KBD_KP_4] = '4', [KBD_KP_5] = '5', [KBD_KP_6] = '6', [KBD_KP_7] = '7',
    [KBD_KP_8] = '8', [KBD_KP_9] = '9',
    [KBD_KP_PERIOD]  = '.',
    [KBD_KP_MINUS]   = '-',
    [KBD_KP_PLUS]    = '+',
    [KBD_KP_MULTIPLY]= '*',
    [KBD_KP_DIVIDE]  = '/',
    [KBD_KP_ENTER]   = '\n',
};

static const unsigned char csa_altgr[KBD_KEYCODE_COUNT] = {
    [KBD_GRAVE]     = '|',

    [KBD_7] = '{',
    [KBD_8] = '}',
    [KBD_9] = '[',
    [KBD_0] = ']',

    [KBD_LEFT_BRACKET]  = '`',
    [KBD_RIGHT_BRACKET] = '~',

    [KBD_APOSTROPHE]    = '{',

    [KBD_COMMA]     = '<',
    [KBD_PERIOD]    = '>',

    [KBD_SPACE]     = ' ',
    [KBD_ENTER]     = '\n',
    [KBD_BACKSPACE] = '\b',
    [KBD_TAB]       = '\t',
};

const kbd_keyset_t kbd_keyset_csa = {
    .name      = "csa",
    .unshifted = csa_unshifted,
    .shifted   = csa_shifted,
    .altgr     = csa_altgr,
};

const kbd_keyset_t *kbd_active_keyset = &kbd_keyset_csa;

void kbd_set_keyset(const kbd_keyset_t *keyset) {
    kbd_active_keyset = keyset ? keyset : &kbd_keyset_csa;
}

const kbd_keyset_t *kbd_get_keyset(void) {
    return kbd_active_keyset;
}
