/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   keyboard.c - PS/2 Keyboard Driver Implementation                         */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

#include "../include/keyboard.h"
#include "../include/pic.h"

/*
** ==========================================================================
** I/O Port Functions
** ==========================================================================
*/

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}

/*
** ==========================================================================
** Scancode to ASCII Translation Tables
** ==========================================================================
*/

static const char g_scancode_to_ascii[128] = {
    0,    27,  '1',  '2',  '3',  '4',  '5',  '6',     /* 0x00 - 0x07 */
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',   /* 0x08 - 0x0F */
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',     /* 0x10 - 0x17 */
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',     /* 0x18 - 0x1F */
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',     /* 0x20 - 0x27 */
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',     /* 0x28 - 0x2F */
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',     /* 0x30 - 0x37 */
    0,    ' ',  0,    0,    0,    0,    0,    0,       /* 0x38 - 0x3F */
    0,    0,    0,    0,    0,    0,    0,    '7',     /* 0x40 - 0x47 */
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',     /* 0x48 - 0x4F */
    '2',  '3',  '0',  '.',  0,    0,    0,    0,       /* 0x50 - 0x57 */
    0,    0,    0,    0,    0,    0,    0,    0        /* 0x58 - 0x5F */
};

static const char g_scancode_to_ascii_shift[128] = {
    0,    27,  '!',  '@',  '#',  '$',  '%',  '^',     /* 0x00 - 0x07 */
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',   /* 0x08 - 0x0F */
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',     /* 0x10 - 0x17 */
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',     /* 0x18 - 0x1F */
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',     /* 0x20 - 0x27 */
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',     /* 0x28 - 0x2F */
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',     /* 0x30 - 0x37 */
    0,    ' ',  0,    0,    0,    0,    0,    0,       /* 0x38 - 0x3F */
    0,    0,    0,    0,    0,    0,    0,    '7',     /* 0x40 - 0x47 */
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',     /* 0x48 - 0x4F */
    '2',  '3',  '0',  '.',  0,    0,    0,    0,       /* 0x50 - 0x57 */
    0,    0,    0,    0,    0,    0,    0,    0        /* 0x58 - 0x5F */
};

/*
** ==========================================================================
** Keyboard State and Circular Buffer
** ==========================================================================
*/

static t_keyboard_state g_kb_state;
static t_key_event      g_key_buffer[KEYBOARD_BUFFER_SIZE];
static size_t           g_buffer_read;
static size_t           g_buffer_write;
static size_t           g_buffer_count;

/*
** ==========================================================================
** Keyboard Initialization
** ==========================================================================
*/

void keyboard_init(void)
{
    size_t i;

    g_kb_state.shift_pressed = FALSE;
    g_kb_state.ctrl_pressed = FALSE;
    g_kb_state.alt_pressed = FALSE;
    g_kb_state.caps_lock = FALSE;

    g_buffer_read = 0;
    g_buffer_write = 0;
    g_buffer_count = 0;

    i = 0;
    while (i < KEYBOARD_BUFFER_SIZE)
    {
        g_key_buffer[i].scancode = 0;
        g_key_buffer[i].ascii = 0;
        g_key_buffer[i].pressed = FALSE;
        i++;
    }

    /* Enable keyboard IRQ (IRQ1) */
    pic_clear_mask(1);
}

/*
** ==========================================================================
** Scancode to ASCII Translation
** ==========================================================================
*/

static char scancode_to_ascii(uint8_t scancode)
{
    char    c;
    bool_t  uppercase;

    if (scancode >= 128)
    {
        return (0);
    }

    uppercase = (bool_t)(g_kb_state.shift_pressed ^ g_kb_state.caps_lock);

    if (uppercase)
    {
        c = g_scancode_to_ascii_shift[scancode];
    }
    else
    {
        c = g_scancode_to_ascii[scancode];
    }

    return (c);
}

/*
** ==========================================================================
** Update Keyboard State
** ==========================================================================
*/

static void update_keyboard_state(uint8_t scancode, bool_t pressed)
{
    if (scancode == KEY_LSHIFT || scancode == KEY_RSHIFT)
    {
        g_kb_state.shift_pressed = pressed;
    }
    else if (scancode == KEY_CTRL)
    {
        g_kb_state.ctrl_pressed = pressed;
    }
    else if (scancode == KEY_ALT)
    {
        g_kb_state.alt_pressed = pressed;
    }
    else if (scancode == KEY_CAPS && pressed)
    {
        g_kb_state.caps_lock = (bool_t)(!g_kb_state.caps_lock);
    }
}

/*
** ==========================================================================
** Add Key to Circular Buffer
** ==========================================================================
*/

static void buffer_add_key(t_key_event event)
{
    if (g_buffer_count >= KEYBOARD_BUFFER_SIZE)
    {
        return;
    }

    g_key_buffer[g_buffer_write] = event;
    g_buffer_write = (g_buffer_write + 1) % KEYBOARD_BUFFER_SIZE;
    g_buffer_count++;
}

/*
** ==========================================================================
** Keyboard Interrupt Handler (called from IRQ1)
** ==========================================================================
*/

void keyboard_handler(void)
{
    uint8_t         scancode;
    bool_t          pressed;
    t_key_event     event;

    scancode = inb(KEYBOARD_DATA_PORT);
    pressed = (bool_t)((scancode & KEY_RELEASED_OFFSET) == 0);

    if (!pressed)
    {
        scancode = (uint8_t)(scancode & ~KEY_RELEASED_OFFSET);
    }

    update_keyboard_state(scancode, pressed);

    event.scancode = scancode;
    event.ascii = scancode_to_ascii(scancode);
    event.pressed = pressed;

    if (pressed)
    {
        buffer_add_key(event);
    }

    pic_send_eoi(1);
}

/*
** ==========================================================================
** Check if Key is Available
** ==========================================================================
*/

bool_t keyboard_has_key(void)
{
    return ((bool_t)(g_buffer_count > 0));
}

/*
** ==========================================================================
** Get Key Event from Buffer
** ==========================================================================
*/

t_key_event keyboard_get_key(void)
{
    t_key_event event;

    if (g_buffer_count == 0)
    {
        event.scancode = 0;
        event.ascii = 0;
        event.pressed = FALSE;
        return (event);
    }

    event = g_key_buffer[g_buffer_read];
    g_buffer_read = (g_buffer_read + 1) % KEYBOARD_BUFFER_SIZE;
    g_buffer_count--;

    return (event);
}

/*
** ==========================================================================
** Blocking Get Character (waits for printable key)
** ==========================================================================
*/

char keyboard_getchar(void)
{
    t_key_event event;

    while (1)
    {
        while (!keyboard_has_key())
        {
            __asm__ volatile ("hlt");
        }

        event = keyboard_get_key();

        if (event.ascii != 0)
        {
            return (event.ascii);
        }
    }
}

/*
** ==========================================================================
** Check if Alt Key is Pressed
** ==========================================================================
*/

bool_t keyboard_alt_pressed(void)
{
    return (g_kb_state.alt_pressed);
}
