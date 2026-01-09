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
    /*
     * Only call the keyboard handler to read scancode from hardware
     * and queue the event. The shell will consume events from the queue.
     * We do NOT consume events here - that's the shell's job.
     */
    keyboard_handler();
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
