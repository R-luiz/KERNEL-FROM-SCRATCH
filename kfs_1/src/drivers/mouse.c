/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   mouse.c - PS/2 Mouse Driver Implementation                               */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

#include "../include/mouse.h"
#include "../include/pic.h"

/*
** ==========================================================================
** I/O Port Functions
** ==========================================================================
*/

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

/*
** ==========================================================================
** Mouse State
** ==========================================================================
*/

#define MOUSE_BUFFER_SIZE   64

static t_mouse_event    g_mouse_buffer[MOUSE_BUFFER_SIZE];
static size_t           g_mouse_read_idx;
static size_t           g_mouse_write_idx;

static uint8_t          g_mouse_cycle;
static uint8_t          g_mouse_packet[4];
static bool_t           g_mouse_has_wheel;

/*
** ==========================================================================
** Wait for Mouse Controller
** ==========================================================================
*/

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

/*
** ==========================================================================
** Send Command to Mouse
** ==========================================================================
*/

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

/*
** ==========================================================================
** Enable Scroll Wheel (IntelliMouse)
** ==========================================================================
*/

static bool_t mouse_enable_wheel(void)
{
    uint8_t device_id;

    /* Magic sequence to enable scroll wheel */
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

    /* Get device ID - should be 3 if wheel is enabled */
    mouse_write(MOUSE_GET_DEVICE_ID);
    mouse_read();  /* ACK */
    device_id = mouse_read();

    return (device_id == 3);
}

/*
** ==========================================================================
** Mouse Initialization
** ==========================================================================
*/

void mouse_init(void)
{
    uint8_t status;

    g_mouse_read_idx = 0;
    g_mouse_write_idx = 0;
    g_mouse_cycle = 0;
    g_mouse_has_wheel = FALSE;

    /* Enable auxiliary device (mouse) */
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_ENABLE_AUX);

    /* Enable interrupts */
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_GET_COMPAQ);
    mouse_wait_read();
    status = inb(MOUSE_DATA_PORT);
    status = (uint8_t)(status | 0x02);   /* Enable IRQ12 */
    status = (uint8_t)(status & ~0x20);  /* Enable mouse clock */
    mouse_wait_write();
    outb(MOUSE_COMMAND_PORT, MOUSE_CMD_SET_COMPAQ);
    mouse_wait_write();
    outb(MOUSE_DATA_PORT, status);

    /* Set defaults */
    mouse_write(MOUSE_SET_DEFAULTS);
    mouse_read();

    /* Try to enable scroll wheel */
    g_mouse_has_wheel = mouse_enable_wheel();

    /* Enable packet streaming */
    mouse_write(MOUSE_ENABLE_PACKET);
    mouse_read();

    /* Enable IRQ12 (mouse) and IRQ2 (cascade from slave PIC) */
    pic_clear_mask(2);   /* Cascade */
    pic_clear_mask(12);  /* Mouse */
}

/*
** ==========================================================================
** Mouse Interrupt Handler
** ==========================================================================
*/

void mouse_handler(void)
{
    uint8_t         data;
    t_mouse_event   event;
    size_t          next_idx;

    data = inb(MOUSE_DATA_PORT);

    /* Synchronize: byte 0 must have bit 3 set */
    if (g_mouse_cycle == 0 && (data & MOUSE_ALWAYS_ONE) == 0)
    {
        pic_send_eoi(12);
        return;
    }

    g_mouse_packet[g_mouse_cycle] = data;
    g_mouse_cycle++;

    /* Check if packet is complete */
    if ((g_mouse_has_wheel && g_mouse_cycle == 4) ||
        (!g_mouse_has_wheel && g_mouse_cycle == 3))
    {
        g_mouse_cycle = 0;

        /* Skip if overflow */
        if ((g_mouse_packet[0] & (MOUSE_X_OVERFLOW | MOUSE_Y_OVERFLOW)) != 0)
        {
            pic_send_eoi(12);
            return;
        }

        /* Build event */
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

        /* Add to buffer */
        next_idx = (g_mouse_write_idx + 1) % MOUSE_BUFFER_SIZE;
        if (next_idx != g_mouse_read_idx)
        {
            g_mouse_buffer[g_mouse_write_idx] = event;
            g_mouse_write_idx = next_idx;
        }
    }

    pic_send_eoi(12);
}

/*
** ==========================================================================
** Check if Mouse Event Available
** ==========================================================================
*/

bool_t mouse_has_event(void)
{
    return (g_mouse_read_idx != g_mouse_write_idx);
}

/*
** ==========================================================================
** Get Mouse Event from Buffer
** ==========================================================================
*/

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
