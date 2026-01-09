#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"

#define KEYBOARD_DATA_PORT      0x60
#define KEYBOARD_STATUS_PORT    0x64
#define KEYBOARD_COMMAND_PORT   0x64

#define KEYBOARD_BUFFER_SIZE    256

#define KEY_ESC         0x01
#define KEY_BACKSPACE   0x0E
#define KEY_TAB         0x0F
#define KEY_ENTER       0x1C
#define KEY_CTRL        0x1D
#define KEY_LSHIFT      0x2A
#define KEY_RSHIFT      0x36
#define KEY_ALT         0x38
#define KEY_CAPS        0x3A
#define KEY_F1          0x3B
#define KEY_F2          0x3C
#define KEY_F3          0x3D
#define KEY_F4          0x3E
#define KEY_F5          0x3F
#define KEY_F6          0x40
#define KEY_F7          0x41
#define KEY_F8          0x42
#define KEY_F9          0x43
#define KEY_F10         0x44
#define KEY_F11         0x57
#define KEY_F12         0x58

#define KEY_RELEASED_OFFSET     0x80

typedef struct s_keyboard_state
{
    bool_t      shift_pressed;
    bool_t      ctrl_pressed;
    bool_t      alt_pressed;
    bool_t      caps_lock;
}   t_keyboard_state;

typedef struct s_key_event
{
    uint8_t     scancode;
    char        ascii;
    bool_t      pressed;
}   t_key_event;

void        keyboard_init(void);
void        keyboard_handler(void);
bool_t      keyboard_has_key(void);
t_key_event keyboard_get_key(void);
char        keyboard_getchar(void);
bool_t      keyboard_alt_pressed(void);

#endif
