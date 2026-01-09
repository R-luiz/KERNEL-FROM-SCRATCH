#include "../include/mouse.h"
#include "../include/pic.h"

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}

#define MOUSE_BUFFER_SIZE   64

static t_mouse_event    g_mouse_buffer[MOUSE_BUFFER_SIZE];
static size_t           g_mouse_read_idx;
static size_t           g_mouse_write_idx;

static uint8_t          g_mouse_cycle;
static uint8_t          g_mouse_packet[4];
static bool_t           g_mouse_has_wheel;

static void mouse_wait_write(void)
{
    uint32_t timeout;

    timeout = 100000;
    while (timeout > 0)
    {
        if ((inb(MOUSE_STATUS_PORT) & 0x02) == 0)
        {
            return;
        }
        timeout--;
    }
}

static void mouse_wait_read(void)
{
    uint32_t timeout;

    timeout = 100000;
    while (timeout > 0)
    {
        if ((inb(MOUSE_STATUS_PORT) & 0x01) != 0)
        {
            return;
        }
        timeout--;
    }
}

static void mouse_write(uint8_t data)
{
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_WRITE_MOUSE);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, data);
}

static uint8_t mouse_read(void)
{
    mouse_wait_read();
    return (inb(MOUSE_DATA_PORT));
}

static bool_t mouse_enable_wheel(void)
{
    uint8_t device_id;


    mouse_write(MOUSE_SET_SAMPLE_RATE);
    mouse_read();
    mouse_write(200);
    mouse_read();

    mouse_write(MOUSE_SET_SAMPLE_RATE);
    mouse_read();
    mouse_write(100);
    mouse_read();

    mouse_write(MOUSE_SET_SAMPLE_RATE);
    mouse_read();
    mouse_write(80);
    mouse_read();


    mouse_write(MOUSE_GET_DEVICE_ID);
    mouse_read();
    device_id = mouse_read();

    return (device_id == 3);
}

void mouse_init(void)
{
    uint8_t status;

    g_mouse_read_idx = 0;
    g_mouse_write_idx = 0;
    g_mouse_cycle = 0;
    g_mouse_has_wheel = FALSE;


    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_ENABLE_AUX);


    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_GET_COMPAQ);
    mouse_wait_read();
    status = inb(MOUSE_DATA_PORT);
    status = (uint8_t)(status | 0x02);
    status = (uint8_t)(status & ~0x20);
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_SET_COMPAQ);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, status);


    mouse_write(MOUSE_SET_DEFAULTS);
    mouse_read();


    g_mouse_has_wheel = mouse_enable_wheel();


    mouse_write(MOUSE_ENABLE_PACKET);
    mouse_read();


    pic_clear_mask(2);
    pic_clear_mask(12);
}

void mouse_handler(void)
{
    uint8_t         data;
    t_mouse_event   event;
    size_t          next_idx;

    data = inb(MOUSE_DATA_PORT);


    if (g_mouse_cycle == 0 && (data & MOUSE_ALWAYS_ONE) == 0)
    {
        pic_send_eoi(12);
        return;
    }

    g_mouse_packet[g_mouse_cycle] = data;
    g_mouse_cycle++;


    if ((g_mouse_has_wheel && g_mouse_cycle == 4) ||
        (!g_mouse_has_wheel && g_mouse_cycle == 3))
    {
        g_mouse_cycle = 0;


        if ((g_mouse_packet[0] & (MOUSE_X_OVERFLOW | MOUSE_Y_OVERFLOW)) != 0)
        {
            pic_send_eoi(12);
            return;
        }


        event.left_btn = (g_mouse_packet[0] & MOUSE_LEFT_BTN) != 0;
        event.right_btn = (g_mouse_packet[0] & MOUSE_RIGHT_BTN) != 0;
        event.middle_btn = (g_mouse_packet[0] & MOUSE_MIDDLE_BTN) != 0;

        event.delta_x = (int8_t)g_mouse_packet[1];
        event.delta_y = (int8_t)g_mouse_packet[2];

        if (g_mouse_has_wheel)
        {
            event.delta_z = (int8_t)(g_mouse_packet[3] & 0x0F);
            if ((g_mouse_packet[3] & 0x08) != 0)
            {
                event.delta_z = (int8_t)(event.delta_z | 0xF0);
            }
        }
        else
        {
            event.delta_z = 0;
        }


        next_idx = (g_mouse_write_idx + 1) % MOUSE_BUFFER_SIZE;
        if (next_idx != g_mouse_read_idx)
        {
            g_mouse_buffer[g_mouse_write_idx] = event;
            g_mouse_write_idx = next_idx;
        }
    }

    pic_send_eoi(12);
}

bool_t mouse_has_event(void)
{
    return (g_mouse_read_idx != g_mouse_write_idx);
}

t_mouse_event mouse_get_event(void)
{
    t_mouse_event event;

    event.delta_x = 0;
    event.delta_y = 0;
    event.delta_z = 0;
    event.left_btn = FALSE;
    event.right_btn = FALSE;
    event.middle_btn = FALSE;

    if (g_mouse_read_idx != g_mouse_write_idx)
    {
        event = g_mouse_buffer[g_mouse_read_idx];
        g_mouse_read_idx = (g_mouse_read_idx + 1) % MOUSE_BUFFER_SIZE;
    }

    return (event);
}
