/*
 * libkb
 *
 * Handles translations of raw scancodes into usable characters
 */

#include "kb.h"

unsigned int us_map[128] = {
    [0x01] = KEY_ESCAPE,
    [0x02] = '1',
    [0x03] = '2',
    [0x04] = '3',
    [0x05] = '4',
    [0x06] = '5',
    [0x07] = '6',
    [0x08] = '7',
    [0x09] = '8',
    [0x0a] = '9',
    [0x0b] = '0',
    [0x0c] = '-',
    [0x0d] = '=',
    [0x0e] = '\b',
    [0x0f] = '\t',
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1a] = '[',
    [0x1b] = ']',
    [0x1c] = '\n',

    [0x1e] = 'a',
    [0x1f] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ';',
    [0x28] = '\'',
    [0x29] = '`',

    [0x2b] = '\\',
    [0x2c] = 'z',
    [0x2d] = 'x',
    [0x2e] = 'c',
    [0x2f] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = ',',
    [0x34] = '.',
    [0x35] = '/',

    [0x39] = ' ',

    [0x3b] = KEY_F1,
    [0x3c] = KEY_F2,
    [0x3d] = KEY_F3,
    [0x3e] = KEY_F4,
    [0x3f] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,

    [0x49] = KEY_PAGE_UP,
    [0x51] = KEY_PAGE_DOWN,

    [0x52] = KEY_INSERT,
    [0x47] = KEY_HOME,
    [0x53] = KEY_DEL,
    [0x4f] = KEY_END,

    [0x48] = KEY_ARROW_UP,
    [0x50] = KEY_ARROW_DOWN,
    [0x4b] = KEY_ARROW_LEFT,
    [0x4d] = KEY_ARROW_RIGHT,

};

unsigned int us_map_shift[128] = {
    [0x01] = KEY_ESCAPE,
    [0x02] = '!',
    [0x03] = '@',
    [0x04] = '#',
    [0x05] = '$',
    [0x06] = '%',
    [0x07] = '^',
    [0x08] = '&',
    [0x09] = '*',
    [0x0a] = '(',
    [0x0b] = ')',
    [0x0c] = '_',
    [0x0d] = '+',
    [0x0e] = '\b',
    [0x0f] = KEY_BACKTAB,
    [0x10] = 'q',
    [0x11] = 'w',
    [0x12] = 'e',
    [0x13] = 'r',
    [0x14] = 't',
    [0x15] = 'y',
    [0x16] = 'u',
    [0x17] = 'i',
    [0x18] = 'o',
    [0x19] = 'p',
    [0x1a] = '{',
    [0x1b] = '}',
    [0x1c] = '\n',

    [0x1e] = 'a',
    [0x1f] = 's',
    [0x20] = 'd',
    [0x21] = 'f',
    [0x22] = 'g',
    [0x23] = 'h',
    [0x24] = 'j',
    [0x25] = 'k',
    [0x26] = 'l',
    [0x27] = ':',
    [0x28] = '"',
    [0x29] = '~',

    [0x2b] = '|',
    [0x2c] = 'z',
    [0x2d] = 'x',
    [0x2e] = 'c',
    [0x2f] = 'v',
    [0x30] = 'b',
    [0x31] = 'n',
    [0x32] = 'm',
    [0x33] = '<',
    [0x34] = '>',
    [0x35] = '?',

    [0x3b] = KEY_F1,
    [0x3c] = KEY_F2,
    [0x3d] = KEY_F3,
    [0x3e] = KEY_F4,
    [0x3f] = KEY_F5,
    [0x40] = KEY_F6,
    [0x41] = KEY_F7,
    [0x42] = KEY_F8,
    [0x43] = KEY_F9,
    [0x44] = KEY_F10,
    [0x52] = KEY_INSERT,
    [0x47] = KEY_HOME,
    [0x53] = KEY_DEL,
    [0x4f] = KEY_END,

    [0x39] = ' ',
    [0x49] = KEY_PAGE_UP,
    [0x51] = KEY_PAGE_DOWN,

    [0x48] = KEY_SHIFT_ARROW_UP,
    [0x50] = KEY_SHIFT_ARROW_DOWN,
    [0x4b] = KEY_SHIFT_ARROW_LEFT,
    [0x4d] = KEY_SHIFT_ARROW_RIGHT,

};

int kb_parse(struct kb_state* state, struct kb_result* result,
             unsigned char scancode)
{
    int is_release = 0;
    if (!state || !result) {
        return 0;
    }
    if (scancode & 0x80) {
        is_release = 1;
        scancode &= ~0x80;
    }
    switch (scancode) {
        case 0xE0:
            // Umm, probably handle this, this is special
            break;
        case 0x1D:
            state->ctrl = !state->ctrl;
            break;
        case 0x2A:
            state->shift = !state->shift;
            break;
        default:
            if (!is_release) {
                if (state->shift) {
                    result->result = us_map_shift[scancode];
                } else {
                    result->result = us_map[scancode];
                }
                return 1;
            }
    }
    return 0;
}