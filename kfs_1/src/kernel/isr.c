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
#include "../include/keyboard.h"
#include "../include/vtty.h"

/*
** ==========================================================================
** ISR Handler - CPU Exceptions
** ==========================================================================
** Called from assembly stub when CPU exception occurs
*/

void isr_handler(void)
{
    /* For now, just halt on any CPU exception */
    /* In future, could parse stack to get exception number and info */
    KERNEL_PANIC("CPU Exception occurred");
}

/*
** ==========================================================================
** IRQ Handler - Hardware Interrupts
** ==========================================================================
** Called from assembly stub when hardware IRQ occurs
** NASA Rule #4: Function under 60 lines
*/

void irq_handler(void)
{
    t_key_event key;
    bool_t      alt_held;

    /* For simplicity, we handle keyboard directly since it's IRQ1 */
    /* The keyboard handler will send EOI */
    keyboard_handler();

    /* Process keyboard events from buffer */
    while (keyboard_has_key())
    {
        key = keyboard_get_key();
        alt_held = keyboard_alt_pressed();

        /* Check for terminal switch (Alt+F1/F2/F3/F4) */
        if (alt_held && key.scancode >= KEY_F1 && key.scancode <= KEY_F4)
        {
            /* Alt+F1 through Alt+F4 for terminals 0-3 */
            vtty_switch((uint8_t)(key.scancode - KEY_F1));
        }
        else if (key.ascii != 0)
        {
            /* Echo printable character to current terminal */
            vtty_putchar(key.ascii);
        }
    }
}
