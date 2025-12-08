/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   vga.c - VGA Text Mode Driver Implementation                              */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - Functions <= 60 lines                                                  */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Variables at narrowest scope                                           */
/*                                                                            */
/* ************************************************************************** */

#include "vga.h"
#include "../lib/string.h"

/*
** ==========================================================================
** Module-level state (single instance terminal)
** ==========================================================================
*/

static t_vga_terminal g_terminal;

/*
** ==========================================================================
** I/O Port Functions (for hardware cursor control)
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
** Internal Helper Functions
** ==========================================================================
*/

/*
** Create a VGA entry (character + color attribute)
** Returns: 16-bit VGA entry
*/
static inline uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)c | ((uint16_t)color << 8));
}

/*
** Calculate buffer index from x,y coordinates
** Returns: Linear index into VGA buffer
** NASA Rule #2: Bounded by VGA_SIZE implicitly
*/
static inline size_t vga_index(size_t x, size_t y)
{
    return (y * VGA_WIDTH + x);
}

/*
** ==========================================================================
** Public Interface Implementation
** ==========================================================================
*/

/*
** Initialize the VGA terminal
** Clears screen and sets default colors (white on black)
*/
void vga_init(void)
{
    g_terminal.cursor_row = 0;
    g_terminal.cursor_col = 0;
    g_terminal.current_color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    g_terminal.buffer = (volatile uint16_t *)VGA_MEMORY_ADDRESS;
    vga_clear();
    vga_enable_cursor(14, 15);
}

/*
** Create a color attribute byte from foreground and background colors
** Returns: Combined color attribute
*/
uint8_t vga_make_color(t_vga_color fg, t_vga_color bg)
{
    return ((uint8_t)(fg | (bg << 4)));
}

/*
** Set the current terminal color
*/
void vga_set_color(uint8_t color)
{
    g_terminal.current_color = color;
}

/*
** Clear the entire screen with current background color
** NASA Rule #2: Loop bounded by VGA_SIZE constant
*/
void vga_clear(void)
{
    size_t      i;
    uint16_t    blank;

    blank = vga_entry(' ', g_terminal.current_color);
    i = 0;
    while (i < VGA_SIZE)
    {
        g_terminal.buffer[i] = blank;
        i++;
    }
    g_terminal.cursor_row = 0;
    g_terminal.cursor_col = 0;
    vga_update_cursor();
}

/*
** Scroll the screen up by one line
** NASA Rule #2: All loops bounded by constants
*/
void vga_scroll(void)
{
    size_t      i;
    uint16_t    blank;
    size_t      last_row_start;

    /* Move all lines up by one */
    i = 0;
    while (i < (VGA_HEIGHT - 1) * VGA_WIDTH)
    {
        g_terminal.buffer[i] = g_terminal.buffer[i + VGA_WIDTH];
        i++;
    }
    /* Clear the last line */
    blank = vga_entry(' ', g_terminal.current_color);
    last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
    i = 0;
    while (i < VGA_WIDTH)
    {
        g_terminal.buffer[last_row_start + i] = blank;
        i++;
    }
}

/*
** Write a character at specific position with specific color
** NASA Rule #7: Bounds checking before write
*/
void vga_putchar_at(char c, uint8_t color, size_t x, size_t y)
{
    size_t index;

    if (x >= VGA_WIDTH || y >= VGA_HEIGHT)
    {
        return;
    }
    index = vga_index(x, y);
    g_terminal.buffer[index] = vga_entry(c, color);
}

/*
** Write a single character at current cursor position
** Handles special characters: newline, carriage return, tab, backspace
*/
void vga_putchar(char c)
{
    if (c == '\n')
    {
        g_terminal.cursor_col = 0;
        g_terminal.cursor_row++;
    }
    else if (c == '\r')
    {
        g_terminal.cursor_col = 0;
    }
    else if (c == '\t')
    {
        g_terminal.cursor_col = (g_terminal.cursor_col + 4) & ~3;
    }
    else if (c == '\b')
    {
        if (g_terminal.cursor_col > 0)
        {
            g_terminal.cursor_col--;
            vga_putchar_at(' ', g_terminal.current_color,
                g_terminal.cursor_col, g_terminal.cursor_row);
        }
    }
    else
    {
        vga_putchar_at(c, g_terminal.current_color,
            g_terminal.cursor_col, g_terminal.cursor_row);
        g_terminal.cursor_col++;
    }
    /* Handle line wrap */
    if (g_terminal.cursor_col >= VGA_WIDTH)
    {
        g_terminal.cursor_col = 0;
        g_terminal.cursor_row++;
    }
    /* Handle scroll */
    if (g_terminal.cursor_row >= VGA_HEIGHT)
    {
        vga_scroll();
        g_terminal.cursor_row = VGA_HEIGHT - 1;
    }
    vga_update_cursor();
}

/*
** Write a null-terminated string to the terminal
** NASA Rule #2: Loop bounded by null terminator (implicit bound)
** NASA Rule #1: No recursion, simple iteration
*/
void vga_putstr(const char *str)
{
    size_t i;

    if (str == NULL)
    {
        return;
    }
    i = 0;
    while (str[i] != '\0')
    {
        vga_putchar(str[i]);
        i++;
    }
}

/*
** ==========================================================================
** Hardware Cursor Control (Bonus)
** ==========================================================================
** Uses VGA I/O ports 0x3D4 (index) and 0x3D5 (data)
*/

#define VGA_CTRL_REGISTER   0x3D4
#define VGA_DATA_REGISTER   0x3D5

/*
** Enable the hardware cursor with specified shape
** cursor_start: top scanline (0-15)
** cursor_end: bottom scanline (0-15)
*/
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xC0) | cursor_start);
    outb(VGA_CTRL_REGISTER, 0x0B);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xE0) | cursor_end);
}

/*
** Disable the hardware cursor
*/
void vga_disable_cursor(void)
{
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 0x20);
}

/*
** Update hardware cursor position to match terminal state
*/
void vga_update_cursor(void)
{
    uint16_t pos;

    pos = (uint16_t)(g_terminal.cursor_row * VGA_WIDTH + g_terminal.cursor_col);
    outb(VGA_CTRL_REGISTER, 0x0F);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, 0x0E);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}

/*
** Set cursor position manually
*/
void vga_set_cursor(size_t x, size_t y)
{
    if (x >= VGA_WIDTH)
    {
        x = VGA_WIDTH - 1;
    }
    if (y >= VGA_HEIGHT)
    {
        y = VGA_HEIGHT - 1;
    }
    g_terminal.cursor_col = x;
    g_terminal.cursor_row = y;
    vga_update_cursor();
}
