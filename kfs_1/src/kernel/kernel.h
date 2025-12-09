/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   kernel.h - Kernel Main Interface                                         */
/*                                                                            */
/*   NASA/JPL C Coding Standards Compliant                                    */
/*                                                                            */
/* ************************************************************************** */

#ifndef KERNEL_H
#define KERNEL_H

#include "../include/types.h"
#include "../drivers/vga.h"
#include "../lib/string.h"

/*
** ==========================================================================
** Kernel Version Information
** ==========================================================================
*/

#define KERNEL_NAME         "KFS_1"
#define KERNEL_VERSION      "1.0.0"
#define KERNEL_AUTHOR       "rluiz"

/*
** ==========================================================================
** Kernel Panic (NASA Rule #5: Assertions)
** ==========================================================================
*/

#define KERNEL_PANIC(msg) kernel_panic(__FILE__, __LINE__, msg)

void NORETURN kernel_panic(const char *file, int line, const char *msg);

/*
** ==========================================================================
** Kernel Assert Macro
** ==========================================================================
*/

#define KERNEL_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            KERNEL_PANIC(msg); \
        } \
    } while (0)

/*
** ==========================================================================
** Simple printk for debugging (Bonus)
** ==========================================================================
*/

void printk(const char *format, ...);

/*
** ==========================================================================
** Kernel Entry Point
** ==========================================================================
** Called by boot.asm after Multiboot initialization
*/

void kernel_main(void);

#endif /* KERNEL_H */
