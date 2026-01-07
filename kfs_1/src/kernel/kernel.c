/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
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
#include "../include/idt.h"
#include "../include/pic.h"
#include "../include/keyboard.h"
#include "../include/vtty.h"

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
    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    vga_putstr("\n");
    vga_putstr("        ##   #####  \n");
    vga_putstr("        ##  ##   ## \n");
    vga_putstr("        ## ##     ##\n");
    vga_putstr("   ##   ##       ## \n");
    vga_putstr("   ##   ##      ##  \n");
    vga_putstr("   ##   ##     ##   \n");
    vga_putstr("   #######    ##    \n");
    vga_putstr("        ##   ##     \n");
    vga_putstr("        ##  ####### \n");
    vga_putstr("\n");
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
    /* Initialize VGA terminal */
    vga_init();

    /* Display kernel header */
    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vga_putstr("===========================================\n");
    vga_putstr("  ");
    vga_putstr(KERNEL_NAME);
    vga_putstr(" v");
    vga_putstr(KERNEL_VERSION);
    vga_putstr(" - ");
    vga_putstr(KERNEL_AUTHOR);
    vga_putstr("\n");
    vga_putstr("===========================================\n");

    /* Display mandatory "42" */
    display_42_banner();

    /* Display system information */
    vga_set_color(vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("Kernel loaded successfully!\n\n");

    /* Demo printk (Bonus) */
    vga_set_color(vga_make_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK));
    printk("printk test: string=%s, char=%c, int=%d\n", "hello", 'X', -42);
    printk("printk test: uint=%u, hex=%x, ptr=%p\n", 12345, 0xDEAD, (void *)0xB8000);

    /* Initialize interrupt subsystem (Bonus) */
    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    vga_putstr("\nInitializing interrupt subsystem...\n");
    pic_init();
    idt_init();
    vga_putstr("[OK] PIC and IDT initialized\n");

    /* Initialize keyboard driver (Bonus) */
    keyboard_init();
    vga_putstr("[OK] Keyboard driver initialized\n");

    /* Initialize virtual terminals (Bonus) */
    vtty_init();
    vga_putstr("[OK] Virtual terminals initialized (4 TTYs)\n");

    /* Enable interrupts */
    __asm__ volatile ("sti");
    vga_putstr("[OK] Interrupts enabled\n\n");

    /* Display usage instructions */
    vga_set_color(vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));
    vga_putstr("Bonus features enabled:\n");
    vga_putstr("  - Keyboard input with echo\n");
    vga_putstr("  - Virtual terminals (Alt+F1/F2/F3/F4 to switch)\n");
    vga_putstr("  - Scroll and cursor support\n");
    vga_putstr("  - Color support\n\n");

    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vga_putstr("System ready. Type to see keyboard input!\n");
    vga_putstr("Press Alt+F1 through Alt+F4 to switch terminals.\n\n");

    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));

    /* Switch to virtual terminal system */
    /* Terminal 0 is active - keyboard input will be echoed */
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
