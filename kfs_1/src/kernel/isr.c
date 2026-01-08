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
** Stack contains: [saved_regs] [ds] [error_code] [irq_num] [eip] [cs] [eflags]
** NASA Rule #4: Function under 60 lines
*/

void irq_handler(uint32_t irq_num)
{
    t_key_event key;
    bool_t      alt_held;
    uint8_t     irq;

    /* Extract actual IRQ number (irq_num is 32-47, IRQ is 0-15) */
    irq = (uint8_t)(irq_num - 32);

    /* Handle keyboard interrupt (IRQ1) */
    if (irq == 1)
    {
        keyboard_handler();

        /* Process keyboard events from buffer */
        while (keyboard_has_key())
        {
            key = keyboard_get_key();
            alt_held = keyboard_alt_pressed();

            /* Check for terminal switch (Alt+F1/F2/F3/F4) */
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
    else
    {
        /* Other IRQs: just acknowledge them for now */
        pic_send_eoi(irq);
    }
}
