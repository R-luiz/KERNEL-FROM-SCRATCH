#ifndef VTTY_H
#define VTTY_H

#include "types.h"
#include "../drivers/vga.h"

#define VTTY_COUNT              8
#define VTTY_SCROLLBACK_LINES   200
#define VTTY_BUFFER_SIZE        (VGA_WIDTH * VTTY_SCROLLBACK_LINES)
#define VTTY_VISIBLE_SIZE       (VGA_WIDTH * VGA_HEIGHT)

typedef struct s_vtty
{
    uint16_t    buffer[VTTY_BUFFER_SIZE];
    size_t      cursor_row;
    size_t      cursor_col;
    size_t      scroll_offset;
    size_t      total_lines;
    uint8_t     color;
}   t_vtty;

void    vtty_init(void);
void    vtty_switch(uint8_t terminal);
uint8_t vtty_get_current(void);
void    vtty_putchar(char c);
void    vtty_putstr(const char *str);
void    vtty_set_color(uint8_t color);
void    vtty_clear(void);
void    vtty_scroll_up(size_t lines);
void    vtty_scroll_down(size_t lines);

#endif
