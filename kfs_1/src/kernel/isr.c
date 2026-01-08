/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   isr.c - Interrupt Service Routine Handlers                               */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

#include "kernel.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"
#include "../include/vtty.h"

/*
** ==========================================================================
** ISR Handler - CPU Exceptions
** ==========================================================================
** Called from assembly stub when CPU exception occurs
** DO NOT use KERNEL_PANIC here - it might cause another exception!
*/

void isr_handler(void)
{
    /* Halt immediately without trying to print anything */
    /* Printing could cause another exception -> double fault -> triple fault */
    __asm__ volatile ("cli; hlt");
    while (1) { __asm__ volatile ("hlt"); }
}

/*
** ==========================================================================
** Handle Keyboard IRQ
** ==========================================================================
*/

static void handle_keyboard_irq(void)
{
    t_key_event key;
    bool_t      alt_held;

    keyboard_handler();

    while (keyboard_has_key())
    {
        key = keyboard_get_key();
        alt_held = keyboard_alt_pressed();

        if (alt_held && key.scancode >= KEY_F1 && key.scancode <= KEY_F4)
        {
            vtty_switch((uint8_t)(key.scancode - KEY_F1));
        }
        else if (key.ascii != 0)
        {
            vtty_putchar(key.ascii);
        }
    }
}

/*
** ==========================================================================
** Handle Mouse IRQ
** ==========================================================================
*/

static void handle_mouse_irq(void)
{
    t_mouse_event event;

    mouse_handler();

    while (mouse_has_event())
    {
        event = mouse_get_event();

        /* Handle scroll wheel */
        if (event.delta_z > 0)
        {
            /* Scroll up (wheel moved toward user) */
            vtty_scroll_up(3);
        }
        else if (event.delta_z < 0)
        {
            /* Scroll down (wheel moved away from user) */
            vtty_scroll_down(3);
        }
    }
}

/*
** ==========================================================================
** IRQ Handler - Hardware Interrupts
** ==========================================================================
** Called from assembly stub when hardware IRQ occurs
** NASA Rule #4: Function under 60 lines
*/

void irq_handler(uint32_t irq_num)
{
    uint8_t irq;

    /* Extract actual IRQ number (irq_num is 32-47, IRQ is 0-15) */
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
