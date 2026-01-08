/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   vga.h - VGA Text Mode Driver Interface                                   */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef VGA_H
#define VGA_H

#include "../include/types.h"

/*
** ==========================================================================
** VGA Text Mode Constants
** ==========================================================================
**
** Memory Layout at 0xB8000:
** Each character cell = 2 bytes
** Byte 0: ASCII character code
** Byte 1: Attribute byte (colors)
**
** Attribute byte format:
** Bits 0-3: Foreground color (0-15)
** Bits 4-6: Background color (0-7)
** Bit 7:    Blink enable
*/

#define VGA_MEMORY_ADDRESS  0xB8000
#define VGA_WIDTH           80
#define VGA_HEIGHT          25
#define VGA_SIZE            (VGA_WIDTH * VGA_HEIGHT)

/*
** ==========================================================================
** VGA Color Enumeration
** ==========================================================================
*/

typedef enum e_vga_color
{
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15
}   t_vga_color;

/*
** ==========================================================================
** VGA Terminal State Structure
** ==========================================================================
*/

typedef struct s_vga_terminal
{
    size_t          cursor_row;
    size_t          cursor_col;
    uint8_t         current_color;
    volatile uint16_t   *buffer;
}   t_vga_terminal;

/*
** ==========================================================================
** Public Interface - Function Prototypes
** ==========================================================================
** NASA Rule #7: All return values must be checked or functions return void
*/

/* Initialization */
void    vga_init(void);

/* Color management */
uint8_t vga_make_color(t_vga_color fg, t_vga_color bg);
void    vga_set_color(uint8_t color);

/* Output functions */
void    vga_putchar(char c);
void    vga_putstr(const char *str);
void    vga_putchar_at(char c, uint8_t color, size_t x, size_t y);

/* Screen management */
void    vga_clear(void);
void    vga_scroll(void);

/* Cursor management (Bonus) */
void    vga_set_cursor(size_t x, size_t y);
void    vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void    vga_disable_cursor(void);
void    vga_update_cursor(void);

#endif /* VGA_H */
