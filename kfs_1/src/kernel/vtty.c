/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   vtty.c - Virtual Terminal (TTY) Implementation                           */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant:                                   */
/*   - No recursion                                                           */
/*   - All loops bounded                                                      */
/*   - Functions <= 60 lines                                                  */
/*                                                                            */
/* ************************************************************************** */

#include "../include/vtty.h"
#include "../lib/string.h"

/*
** ==========================================================================
** Global Terminal State
** ==========================================================================
*/

static t_vtty           g_terminals[VTTY_COUNT];
static uint8_t          g_current_terminal;
static volatile uint16_t *g_vga_buffer = (volatile uint16_t *)VGA_MEMORY_ADDRESS;

/*
** ==========================================================================
** Helper: Create VGA Entry
** ==========================================================================
*/

static inline uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)c | ((uint16_t)color << 8));
}

/*
** ==========================================================================
** Helper: Calculate Buffer Index
** ==========================================================================
*/

static inline size_t vga_index(size_t x, size_t y)
{
    return (y * VGA_WIDTH + x);
}

/*
** ==========================================================================
** Save Current Terminal to Buffer
** ==========================================================================
*/

static void vtty_save_current(void)
{
    size_t  i;
    t_vtty  *term;

    term = &g_terminals[g_current_terminal];

    i = 0;
    while (i < VTTY_BUFFER_SIZE)
    {
        term->buffer[i] = g_vga_buffer[i];
        i++;
    }
}

/*
** ==========================================================================
** Restore Terminal from Buffer
** ==========================================================================
*/

static void vtty_restore(uint8_t terminal)
{
    size_t  i;
    t_vtty  *term;

    if (terminal >= VTTY_COUNT)
    {
        return;
    }

    term = &g_terminals[terminal];

    i = 0;
    while (i < VTTY_BUFFER_SIZE)
    {
        g_vga_buffer[i] = term->buffer[i];
        i++;
    }

    vga_set_cursor(term->cursor_col, term->cursor_row);
}

/*
** ==========================================================================
** Initialize Virtual Terminals
** ==========================================================================
*/

void vtty_init(void)
{
    uint8_t     i;
    size_t      j;
    uint16_t    blank;

    g_current_terminal = 0;

    blank = vga_entry(' ', vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK));

    i = 0;
    while (i < VTTY_COUNT)
    {
        g_terminals[i].cursor_row = 0;
        g_terminals[i].cursor_col = 0;
        g_terminals[i].color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

        j = 0;
        while (j < VTTY_BUFFER_SIZE)
        {
            g_terminals[i].buffer[j] = blank;
            j++;
        }

        i++;
    }

    vtty_restore(0);
}

/*
** ==========================================================================
** Switch to Different Terminal
** ==========================================================================
*/

void vtty_switch(uint8_t terminal)
{
    if (terminal >= VTTY_COUNT || terminal == g_current_terminal)
    {
        return;
    }

    vtty_save_current();
    g_current_terminal = terminal;
    vtty_restore(terminal);
}

/*
** ==========================================================================
** Get Current Terminal Number
** ==========================================================================
*/

uint8_t vtty_get_current(void)
{
    return (g_current_terminal);
}

/*
** ==========================================================================
** Scroll Terminal
** ==========================================================================
*/

static void vtty_scroll(void)
{
    size_t      i;
    uint16_t    blank;
    size_t      last_row_start;
    t_vtty      *term;

    term = &g_terminals[g_current_terminal];
    blank = vga_entry(' ', term->color);

    i = 0;
    while (i < (VGA_HEIGHT - 1) * VGA_WIDTH)
    {
        term->buffer[i] = term->buffer[i + VGA_WIDTH];
        i++;
    }

    last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
    i = 0;
    while (i < VGA_WIDTH)
    {
        term->buffer[last_row_start + i] = blank;
        i++;
    }
}

/*
** ==========================================================================
** Put Character to Current Terminal
** ==========================================================================
*/

void vtty_putchar(char c)
{
    t_vtty  *term;
    size_t  index;

    term = &g_terminals[g_current_terminal];

    if (c == '\n')
    {
        term->cursor_col = 0;
        term->cursor_row++;
    }
    else if (c == '\r')
    {
        term->cursor_col = 0;
    }
    else if (c == '\t')
    {
        term->cursor_col = (term->cursor_col + 4U) & ~3U;
    }
    else if (c == '\b')
    {
        if (term->cursor_col > 0)
        {
            term->cursor_col--;
            index = vga_index(term->cursor_col, term->cursor_row);
            term->buffer[index] = vga_entry(' ', term->color);
        }
    }
    else
    {
        index = vga_index(term->cursor_col, term->cursor_row);
        term->buffer[index] = vga_entry(c, term->color);
        term->cursor_col++;
    }

    if (term->cursor_col >= VGA_WIDTH)
    {
        term->cursor_col = 0;
        term->cursor_row++;
    }

    if (term->cursor_row >= VGA_HEIGHT)
    {
        vtty_scroll();
        term->cursor_row = VGA_HEIGHT - 1;
    }

    vtty_restore(g_current_terminal);
}

/*
** ==========================================================================
** Put String to Current Terminal
** ==========================================================================
*/

void vtty_putstr(const char *str)
{
    size_t i;

    if (str == NULL)
    {
        return;
    }

    i = 0;
    while (str[i] != '\0')
    {
        vtty_putchar(str[i]);
        i++;
    }
}

/*
** ==========================================================================
** Set Terminal Color
** ==========================================================================
*/

void vtty_set_color(uint8_t color)
{
    g_terminals[g_current_terminal].color = color;
}

/*
** ==========================================================================
** Clear Current Terminal
** ==========================================================================
*/

void vtty_clear(void)
{
    size_t      i;
    uint16_t    blank;
    t_vtty      *term;

    term = &g_terminals[g_current_terminal];
    blank = vga_entry(' ', term->color);

    i = 0;
    while (i < VTTY_BUFFER_SIZE)
    {
        term->buffer[i] = blank;
        i++;
    }

    term->cursor_row = 0;
    term->cursor_col = 0;

    vtty_restore(g_current_terminal);
}
