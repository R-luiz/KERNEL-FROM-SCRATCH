#ifndef STACK_H
# define STACK_H

# include "types.h"

typedef struct s_stack_frame
{
    struct s_stack_frame    *ebp;
    uint32_t                eip;
}   t_stack_frame;

typedef struct s_registers
{
    uint32_t    eax;
    uint32_t    ebx;
    uint32_t    ecx;
    uint32_t    edx;
    uint32_t    esi;
    uint32_t    edi;
    uint32_t    ebp;
    uint32_t    esp;
    uint32_t    eip;
    uint32_t    eflags;
    uint16_t    cs;
    uint16_t    ds;
    uint16_t    es;
    uint16_t    fs;
    uint16_t    gs;
    uint16_t    ss;
}   t_registers;

void    stack_print(void);

void    stack_trace(uint32_t max_frames);

void    stack_dump(uint32_t num_words);

void    stack_print_registers(void);

uint32_t    stack_get_esp(void);

uint32_t    stack_get_ebp(void);

uint32_t    stack_get_eip(void);

uint32_t    stack_get_eflags(void);

#endif
