/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   stack.h - Stack Inspection and Debug Interface                           */
/*                                                                            */
/*   Provides functions to inspect and print the kernel stack.                */
/*   Required by KFS-2 subject for debugging purposes.                        */
/*                                                                            */
/*   Stack Frame Layout (x86 cdecl calling convention):                       */
/*                                                                            */
/*     Higher addresses                                                       */
/*     +------------------+                                                   */
/*     |   Argument N     |  [EBP + 8 + (N-1)*4]                              */
/*     +------------------+                                                   */
/*     |   Argument 1     |  [EBP + 8]                                        */
/*     +------------------+                                                   */
/*     |  Return Address  |  [EBP + 4]                                        */
/*     +------------------+                                                   */
/*     |   Saved EBP      |  [EBP] <- Current EBP points here                 */
/*     +------------------+                                                   */
/*     |  Local Var 1     |  [EBP - 4]                                        */
/*     +------------------+                                                   */
/*     |  Local Var N     |  [EBP - N*4]                                      */
/*     +------------------+                                                   */
/*     Lower addresses     <- ESP points here                                 */
/*                                                                            */
/* ************************************************************************** */

#ifndef STACK_H
# define STACK_H

# include "types.h"

/* ============================================================================
 * Stack Frame Structure
 * ============================================================================ */

/*
 * Represents a single stack frame in the call stack.
 * Used to walk the stack by following EBP chain.
 */
typedef struct s_stack_frame
{
    struct s_stack_frame    *ebp;           /* Previous frame's EBP */
    uint32_t                eip;            /* Return address (EIP) */
}   t_stack_frame;

/* ============================================================================
 * CPU Register State
 * ============================================================================ */

/*
 * Structure to hold all general-purpose registers.
 * Useful for printing complete CPU state.
 */
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

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/*
 * Print the current kernel stack in a human-friendly format.
 * Shows:
 *   - Current ESP and EBP values
 *   - Stack trace with return addresses
 *   - Raw stack memory dump
 *
 * This is the main function required by KFS-2 subject.
 */
void    stack_print(void);

/*
 * Print a stack trace by walking the EBP chain.
 * Shows each stack frame with its return address.
 *
 * Parameters:
 *   max_frames - Maximum number of frames to print (to avoid infinite loops)
 */
void    stack_trace(uint32_t max_frames);

/*
 * Dump raw stack memory from current ESP.
 *
 * Parameters:
 *   num_words - Number of 32-bit words to dump
 */
void    stack_dump(uint32_t num_words);

/*
 * Print all CPU registers.
 * Shows EAX, EBX, ECX, EDX, ESI, EDI, EBP, ESP, EFLAGS, and segment registers.
 */
void    stack_print_registers(void);

/*
 * Get current stack pointer (ESP).
 *
 * Returns:
 *   Current value of ESP register.
 */
uint32_t    stack_get_esp(void);

/*
 * Get current base pointer (EBP).
 *
 * Returns:
 *   Current value of EBP register.
 */
uint32_t    stack_get_ebp(void);

/*
 * Get current instruction pointer (approximate).
 * Note: This gets the return address, not the exact current EIP.
 *
 * Returns:
 *   Approximate EIP value.
 */
uint32_t    stack_get_eip(void);

/*
 * Get current EFLAGS register.
 *
 * Returns:
 *   Current value of EFLAGS register.
 */
uint32_t    stack_get_eflags(void);

#endif /* STACK_H */
