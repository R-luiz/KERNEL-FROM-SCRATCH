/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   kernel.c - Kernel Main Implementation                                    */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Functions <= 60 lines                                                  */
/*   - Variables at narrowest scope                                           */
/*                                                                            */
/* ************************************************************************** */

#include "kernel.h"
#include "../include/gdt.h"
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/keyboard.h"
#include "../include/mouse.h"
#include "../include/vtty.h"
#include "../include/shell.h"
#include "../include/stack.h"

/*
** ==========================================================================
** Variadic Arguments Support (minimal implementation)
** ==========================================================================
** We implement our own since we can't use stdarg.h
*/

typedef __builtin_va_list   va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)

/*
** ==========================================================================
** Kernel Panic Implementation
** ==========================================================================
*/

void NORETURN kernel_panic(const char *file, int line, const char *msg)
{
    char line_str[12];

    /* Set error colors: white on red */
    vga_set_color(vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    vga_clear();

    vga_putstr("\n\n");
    vga_putstr("  =============================================\n");
    vga_putstr("              KERNEL PANIC\n");
    vga_putstr("  =============================================\n\n");
    vga_putstr("  File: ");
    vga_putstr(file);
    vga_putstr("\n");
    vga_putstr("  Line: ");
    k_itoa(line, line_str, 10);
    vga_putstr(line_str);
    vga_putstr("\n\n");
    vga_putstr("  Message: ");
    vga_putstr(msg);
    vga_putstr("\n\n");
    vga_putstr("  System halted.\n");

    /* Halt the CPU forever */
    while (1)
    {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
}

/*
** ==========================================================================
** Printk Implementation (Bonus)
** ==========================================================================
** Simplified printf for kernel debugging
** Supports: %s, %c, %d, %i, %u, %x, %X, %p, %%
*/

static void printk_putnum(uint32_t num, int base, int is_signed, int uppercase)
{
    char buffer[33];

    if (is_signed)
    {
        k_itoa((int32_t)num, buffer, base);
    }
    else
    {
        k_utoa(num, buffer, base);
    }
    if (!uppercase && base == 16)
    {
        size_t i = 0;
        while (buffer[i] != '\0')
        {
            if (buffer[i] >= 'A' && buffer[i] <= 'F')
            {
                buffer[i] = buffer[i] + ('a' - 'A');
            }
            i++;
        }
    }
    vga_putstr(buffer);
}

void printk(const char *format, ...)
{
    va_list args;
    size_t  i;
    char    c;

    if (format == NULL)
    {
        return;
    }
    va_start(args, format);
    i = 0;
    while (format[i] != '\0')
    {
        if (format[i] == '%' && format[i + 1] != '\0')
        {
            i++;
            c = format[i];
            if (c == 's')
            {
                const char *s = va_arg(args, const char *);
                vga_putstr(s != NULL ? s : "(null)");
            }
            else if (c == 'c')
            {
                char ch = (char)va_arg(args, int);
                vga_putchar(ch);
            }
            else if (c == 'd' || c == 'i')
            {
                int32_t num = va_arg(args, int32_t);
                printk_putnum((uint32_t)num, 10, 1, 0);
            }
            else if (c == 'u')
            {
                uint32_t num = va_arg(args, uint32_t);
                printk_putnum(num, 10, 0, 0);
            }
            else if (c == 'x')
            {
                uint32_t num = va_arg(args, uint32_t);
                printk_putnum(num, 16, 0, 0);
            }
            else if (c == 'X')
            {
                uint32_t num = va_arg(args, uint32_t);
                printk_putnum(num, 16, 0, 1);
            }
            else if (c == 'p')
            {
                uint32_t ptr = (uint32_t)va_arg(args, void *);
                vga_putstr("0x");
                printk_putnum(ptr, 16, 0, 0);
            }
            else if (c == '%')
            {
                vga_putchar('%');
            }
            else
            {
                vga_putchar('%');
                vga_putchar(c);
            }
        }
        else
        {
            vga_putchar(format[i]);
        }
        i++;
    }
    va_end(args);
}

/*
** ==========================================================================
** Display 42 in Style (Mandatory Requirement)
** ==========================================================================
*/

static void display_42_banner(void)
{
    vtty_set_color(vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    vtty_putstr("\n");
    vtty_putstr("        ##   #####  \n");
    vtty_putstr("        ##  ##   ## \n");
    vtty_putstr("        ## ##     ##\n");
    vtty_putstr("   ##   ##       ## \n");
    vtty_putstr("   ##   ##      ##  \n");
    vtty_putstr("   ##   ##     ##   \n");
    vtty_putstr("   #######    ##    \n");
    vtty_putstr("        ##   ##     \n");
    vtty_putstr("        ##  ####### \n");
    vtty_putstr("\n");
}

/*
** ==========================================================================
** Kernel Main Entry Point
** ==========================================================================
** Called by boot.asm after stack initialization
** NASA Rule #4: Function under 60 lines
*/

void kernel_main(void)
{
    /* Initialize VGA terminal first (for early error messages) */
    vga_init();

    /*
     * Initialize GDT at address 0x800
     * This is the MAIN requirement for KFS-2
     * Must be done before interrupts
     */
    gdt_init();

    /* Initialize interrupt subsystem */
    pic_init();
    idt_init();

    /* Initialize keyboard driver (interrupts still disabled) */
    keyboard_init();

    /* Initialize mouse driver */
    mouse_init();

    /* Initialize virtual terminals */
    vtty_init();

    /* Display kernel banner */
    vtty_set_color(vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vtty_putstr("===========================================\n");
    vtty_putstr("  ");
    vtty_putstr(KERNEL_NAME);
    vtty_putstr(" v");
    vtty_putstr(KERNEL_VERSION);
    vtty_putstr(" - ");
    vtty_putstr(KERNEL_AUTHOR);
    vtty_putstr("\n");
    vtty_putstr("===========================================\n");

    /* Display mandatory "42" */
    display_42_banner();

    /* Show GDT info */
    vtty_set_color(vga_make_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    vtty_putstr("GDT initialized at 0x800 with 7 segments\n");
    vtty_putstr("  [Kernel: Code/Data/Stack | User: Code/Data/Stack]\n\n");

    /* Enable interrupts */
    __asm__ volatile ("sti");

    vtty_set_color(vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    /* Initialize and run the shell (Bonus) */
    shell_init();
    shell_run();  /* Never returns */

    /* Should never reach here */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
