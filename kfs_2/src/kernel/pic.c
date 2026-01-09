/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   pic.c - Programmable Interrupt Controller Implementation                 */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

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

static inline void io_wait(void)
{
    outb(0x80, 0);
}

/*
** ==========================================================================
** PIC Initialization
** ==========================================================================
** Remaps PIC interrupts to avoid conflicts with CPU exceptions
** IRQ 0-7  -> INT 32-39
** IRQ 8-15 -> INT 40-47
*/

void pic_init(void)
{
    /* Start initialization sequence (ICW1) */
    outb(PIC1_COMMAND, (uint8_t)(ICW1_INIT | ICW1_ICW4));
    io_wait();
    outb(PIC2_COMMAND, (uint8_t)(ICW1_INIT | ICW1_ICW4));
    io_wait();

    /* Set vector offsets (ICW2) */
    outb(PIC1_DATA, 32);    /* Master PIC vector offset: 32 */
    io_wait();
    outb(PIC2_DATA, 40);    /* Slave PIC vector offset: 40 */
    io_wait();

    /* Set cascade (ICW3) */
    outb(PIC1_DATA, 4);     /* Tell Master PIC slave is at IRQ2 */
    io_wait();
    outb(PIC2_DATA, 2);     /* Tell Slave PIC its cascade identity */
    io_wait();

    /* Set mode (ICW4) */
    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    /* Mask all IRQs initially - will unmask keyboard (IRQ1) later */
    outb(PIC1_DATA, 0xFF);  /* Mask all IRQs on master PIC */
    outb(PIC2_DATA, 0xFF);  /* Mask all IRQs on slave PIC */
}

/*
** ==========================================================================
** Send End of Interrupt
** ==========================================================================
*/

void pic_send_eoi(uint8_t irq)
{
    if (irq >= 8)
    {
        outb(PIC2_COMMAND, PIC_EOI);
    }
    outb(PIC1_COMMAND, PIC_EOI);
}

/*
** ==========================================================================
** Set IRQ Mask (disable IRQ)
** ==========================================================================
*/

void pic_set_mask(uint8_t irq)
{
    uint16_t    port;
    uint8_t     value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq = (uint8_t)(irq - 8);
    }

    value = inb(port);
    value = (uint8_t)(value | (1U << irq));
    outb(port, value);
}

/*
** ==========================================================================
** Clear IRQ Mask (enable IRQ)
** ==========================================================================
*/

void pic_clear_mask(uint8_t irq)
{
    uint16_t    port;
    uint8_t     value;

    if (irq < 8)
    {
        port = PIC1_DATA;
    }
    else
    {
        port = PIC2_DATA;
        irq = (uint8_t)(irq - 8);
    }

    value = inb(port);
    value = (uint8_t)(value & ~(1U << irq));
    outb(port, value);
}
