#include "../include/keyboard.h"
#include "../include/pic.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}

static const char g_scancode_to_ascii[128] = {
    0,    27,  '1',  '2',  '3',  '4',  '5',  '6',
    '7',  '8',  '9',  '0',  '-',  '=',  '\b', '\t',
    'q',  'w',  'e',  'r',  't',  'y',  'u',  'i',
    'o',  'p',  '[',  ']',  '\n', 0,    'a',  's',
    'd',  'f',  'g',  'h',  'j',  'k',  'l',  ';',
    '\'', '`',  0,    '\\', 'z',  'x',  'c',  'v',
    'b',  'n',  'm',  ',',  '.',  '/',  0,    '*',
    0,    ' ',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    '7',
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2',  '3',  '0',  '.',  0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0
};

static const char g_scancode_to_ascii_shift[128] = {
    0,    27,  '!',  '@',  '#',  '$',  '%',  '^',
    '&',  '*',  '(',  ')',  '_',  '+',  '\b', '\t',
    'Q',  'W',  'E',  'R',  'T',  'Y',  'U',  'I',
    'O',  'P',  '{',  '}',  '\n', 0,    'A',  'S',
    'D',  'F',  'G',  'H',  'J',  'K',  'L',  ':',
    '"',  '~',  0,    '|',  'Z',  'X',  'C',  'V',
    'B',  'N',  'M',  '<',  '>',  '?',  0,    '*',
    0,    ' ',  0,    0,    0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    '7',
    '8',  '9',  '-',  '4',  '5',  '6',  '+',  '1',
    '2',  '3',  '0',  '.',  0,    0,    0,    0,
    0,    0,    0,    0,    0,    0,    0,    0
};

static t_keyboard_state g_kb_state;
static t_key_event      g_key_buffer[KEYBOARD_BUFFER_SIZE];
static size_t           g_buffer_read;
static size_t           g_buffer_write;
static size_t           g_buffer_count;
static uint8_t          g_last_scancode;
static uint32_t         g_debounce_counter;

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
    g_last_scancode = 0;
    g_debounce_counter = 0;

    i = 0;
    while (i < KEYBOARD_BUFFER_SIZE)
    {
        g_key_buffer[i].scancode = 0;
        g_key_buffer[i].ascii = 0;
        g_key_buffer[i].pressed = FALSE;
        i++;
    }


    pic_clear_mask(1);
}

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

void keyboard_handler(void)
{
    uint8_t         scancode;
    bool_t          pressed;
    t_key_event     event;
    bool_t          is_duplicate;

    scancode = inb(KEYBOARD_DATA_PORT);
    pressed = (bool_t)((scancode & KEY_RELEASED_OFFSET) == 0);

    if (!pressed)
    {
        scancode = (uint8_t)(scancode & ~KEY_RELEASED_OFFSET);
        g_last_scancode = 0;
        g_debounce_counter = 0;
    }

    update_keyboard_state(scancode, pressed);


    is_duplicate = (bool_t)(pressed && scancode == g_last_scancode &&
                            g_debounce_counter < 5);

    if (is_duplicate)
    {
        g_debounce_counter++;
        pic_send_eoi(1);
        return;
    }

    event.scancode = scancode;
    event.ascii = scancode_to_ascii(scancode);
    event.pressed = pressed;

    if (pressed)
    {
        buffer_add_key(event);
        g_last_scancode = scancode;
        g_debounce_counter = 0;
    }

    pic_send_eoi(1);
}

bool_t keyboard_has_key(void)
{
    return ((bool_t)(g_buffer_count > 0));
}

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

bool_t keyboard_alt_pressed(void)
{
    return (g_kb_state.alt_pressed);
}
