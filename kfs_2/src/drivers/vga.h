#ifndef VGA_H
#define VGA_H

#include "../include/types.h"

#define VGA_MEMORY_ADDRESS  0xB8000
#define VGA_WIDTH           80
#define VGA_HEIGHT          25
#define VGA_SIZE            (VGA_WIDTH * VGA_HEIGHT)

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

typedef struct s_vga_terminal
{
    size_t          cursor_row;
    size_t          cursor_col;
    uint8_t         current_color;
    volatile uint16_t   *buffer;
}   t_vga_terminal;

void    vga_init(void);

uint8_t vga_make_color(t_vga_color fg, t_vga_color bg);
void    vga_set_color(uint8_t color);

void    vga_putchar(char c);
void    vga_putstr(const char *str);
void    vga_putchar_at(char c, uint8_t color, size_t x, size_t y);

void    vga_clear(void);
void    vga_scroll(void);

void    vga_set_cursor(size_t x, size_t y);
void    vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void    vga_disable_cursor(void);
void    vga_update_cursor(void);

#endif
