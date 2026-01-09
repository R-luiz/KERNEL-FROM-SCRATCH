/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   vtty.h - Virtual Terminal (TTY) Interface                                */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef VTTY_H
#define VTTY_H

#include "types.h"
#include "../drivers/vga.h"

/*
** ==========================================================================
** Virtual Terminal Constants
** ==========================================================================
*/

#define VTTY_COUNT              8
#define VTTY_SCROLLBACK_LINES   200     /* Total lines in scrollback buffer */
#define VTTY_BUFFER_SIZE        (VGA_WIDTH * VTTY_SCROLLBACK_LINES)
#define VTTY_VISIBLE_SIZE       (VGA_WIDTH * VGA_HEIGHT)

/*
** ==========================================================================
** Virtual Terminal Structure
** ==========================================================================
*/

typedef struct s_vtty
{
    uint16_t    buffer[VTTY_BUFFER_SIZE];   /* Scrollback buffer */
    size_t      cursor_row;                  /* Cursor row in buffer */
    size_t      cursor_col;                  /* Cursor column */
    size_t      scroll_offset;               /* Lines scrolled back (0 = bottom) */
    size_t      total_lines;                 /* Total lines written */
    uint8_t     color;                       /* Current color */
}   t_vtty;

/*
** ==========================================================================
** Function Declarations
** ==========================================================================
*/

void    vtty_init(void);
void    vtty_switch(uint8_t terminal);
uint8_t vtty_get_current(void);
void    vtty_putchar(char c);
void    vtty_putstr(const char *str);
void    vtty_set_color(uint8_t color);
void    vtty_clear(void);
void    vtty_scroll_up(size_t lines);
void    vtty_scroll_down(size_t lines);

#endif /* VTTY_H */
