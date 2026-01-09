#include "../include/vtty.h"
#include "../lib/string.h"

static t_vtty           g_terminals[VTTY_COUNT];
static uint8_t          g_current_terminal;
static volatile uint16_t *g_vga_buffer = (volatile uint16_t *)VGA_MEMORY_ADDRESS;

static inline uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)c | ((uint16_t)color << 8));
}

static inline size_t buffer_index(size_t x, size_t y)
{
    return (y * VGA_WIDTH + x);
}

static void vtty_refresh_display(void)
{
    t_vtty  *term;
    size_t  display_start;
    size_t  src_idx;
    size_t  dst_idx;
    size_t  i;

    term = &g_terminals[g_current_terminal];


    if (term->cursor_row < VGA_HEIGHT)
    {
        display_start = 0;
    }
    else
    {
        display_start = term->cursor_row - VGA_HEIGHT + 1;
    }


    if (display_start >= term->scroll_offset)
    {
        display_start = display_start - term->scroll_offset;
    }
    else
    {
        display_start = 0;
    }


    dst_idx = 0;
    i = 0;
    while (i < VGA_HEIGHT)
    {
        src_idx = buffer_index(0, display_start + i);
        k_memcpy((void *)&g_vga_buffer[dst_idx], &term->buffer[src_idx],
                 VGA_WIDTH * sizeof(uint16_t));
        dst_idx += VGA_WIDTH;
        i++;
    }


    if (term->scroll_offset == 0)
    {
        size_t visible_row;
        if (term->cursor_row >= display_start)
        {
            visible_row = term->cursor_row - display_start;
        }
        else
        {
            visible_row = 0;
        }
        vga_set_cursor(term->cursor_col, visible_row);
    }
    else
    {

        vga_set_cursor(VGA_WIDTH, VGA_HEIGHT);
    }
}

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
        g_terminals[i].scroll_offset = 0;
        g_terminals[i].total_lines = 0;
        g_terminals[i].color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);

        j = 0;
        while (j < VTTY_BUFFER_SIZE)
        {
            g_terminals[i].buffer[j] = blank;
            j++;
        }

        i++;
    }

    vtty_refresh_display();
}

void vtty_switch(uint8_t terminal)
{
    if (terminal >= VTTY_COUNT || terminal == g_current_terminal)
    {
        return;
    }

    g_current_terminal = terminal;
    vtty_refresh_display();
}

uint8_t vtty_get_current(void)
{
    return (g_current_terminal);
}

static void vtty_scroll_content(void)
{
    size_t      i;
    uint16_t    blank;
    size_t      last_row_start;
    t_vtty      *term;

    term = &g_terminals[g_current_terminal];


    if (term->cursor_row < VTTY_SCROLLBACK_LINES - 1)
    {
        return;
    }


    blank = vga_entry(' ', term->color);

    i = 0;
    while (i < (VTTY_SCROLLBACK_LINES - 1) * VGA_WIDTH)
    {
        term->buffer[i] = term->buffer[i + VGA_WIDTH];
        i++;
    }


    last_row_start = (VTTY_SCROLLBACK_LINES - 1) * VGA_WIDTH;
    i = 0;
    while (i < VGA_WIDTH)
    {
        term->buffer[last_row_start + i] = blank;
        i++;
    }


    term->cursor_row = VTTY_SCROLLBACK_LINES - 1;
}

void vtty_putchar(char c)
{
    t_vtty  *term;
    size_t  index;

    term = &g_terminals[g_current_terminal];


    term->scroll_offset = 0;

    if (c == '\n')
    {
        term->cursor_col = 0;
        term->cursor_row++;
        if (term->cursor_row > term->total_lines)
        {
            term->total_lines = term->cursor_row;
        }
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
            index = buffer_index(term->cursor_col, term->cursor_row);
            term->buffer[index] = vga_entry(' ', term->color);
        }
    }
    else
    {
        index = buffer_index(term->cursor_col, term->cursor_row);
        term->buffer[index] = vga_entry(c, term->color);
        term->cursor_col++;
    }

    if (term->cursor_col >= VGA_WIDTH)
    {
        term->cursor_col = 0;
        term->cursor_row++;
        if (term->cursor_row > term->total_lines)
        {
            term->total_lines = term->cursor_row;
        }
    }

    if (term->cursor_row >= VTTY_SCROLLBACK_LINES)
    {
        vtty_scroll_content();
    }

    vtty_refresh_display();
}

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

void vtty_set_color(uint8_t color)
{
    g_terminals[g_current_terminal].color = color;
}

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
    term->scroll_offset = 0;
    term->total_lines = 0;

    vtty_refresh_display();
}

void vtty_scroll_up(size_t lines)
{
    t_vtty  *term;
    size_t  max_offset;

    term = &g_terminals[g_current_terminal];


    if (term->cursor_row >= VGA_HEIGHT)
    {
        max_offset = term->cursor_row - VGA_HEIGHT + 1;
    }
    else
    {
        max_offset = 0;
    }

    term->scroll_offset += lines;
    if (term->scroll_offset > max_offset)
    {
        term->scroll_offset = max_offset;
    }

    vtty_refresh_display();
}

void vtty_scroll_down(size_t lines)
{
    t_vtty  *term;

    term = &g_terminals[g_current_terminal];

    if (term->scroll_offset >= lines)
    {
        term->scroll_offset -= lines;
    }
    else
    {
        term->scroll_offset = 0;
    }

    vtty_refresh_display();
}
