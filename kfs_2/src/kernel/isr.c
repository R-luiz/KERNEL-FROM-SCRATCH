#include "kernel.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"
#include "../include/vtty.h"

void isr_handler(void)
{


    __asm__ volatile ("cli; hlt");
    while (1) { __asm__ volatile ("hlt"); }
}

static void handle_keyboard_irq(void)
{

    keyboard_handler();
}

static void handle_mouse_irq(void)
{
    t_mouse_event event;

    mouse_handler();

    while (mouse_has_event())
    {
        event = mouse_get_event();


        if (event.delta_z > 0)
        {

            vtty_scroll_up(3);
        }
        else if (event.delta_z < 0)
        {

            vtty_scroll_down(3);
        }
    }
}

void irq_handler(uint32_t irq_num)
{
    uint8_t irq;


    irq = (uint8_t)(irq_num - 32);

    if (irq == 1)
    {
        handle_keyboard_irq();
    }
    else if (irq == 12)
    {
        handle_mouse_irq();
    }
    else
    {
        pic_send_eoi(irq);
    }
}
