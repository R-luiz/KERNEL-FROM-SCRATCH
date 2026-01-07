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

#define VTTY_COUNT          4
#define VTTY_BUFFER_SIZE    (VGA_WIDTH * VGA_HEIGHT)

/*
** ==========================================================================
** Virtual Terminal Structure
** ==========================================================================
*/

typedef struct s_vtty
{
    uint16_t    buffer[VTTY_BUFFER_SIZE];   /* Screen buffer */
    size_t      cursor_row;                  /* Cursor row */
    size_t      cursor_col;                  /* Cursor column */
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

#endif /* VTTY_H */
