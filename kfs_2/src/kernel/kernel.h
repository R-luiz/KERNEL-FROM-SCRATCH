#ifndef KERNEL_H
#define KERNEL_H

#include "../include/types.h"
#include "../drivers/vga.h"
#include "../lib/string.h"

#define KERNEL_NAME         "KFS_2"
#define KERNEL_VERSION      "2.0.0"
#define KERNEL_AUTHOR       "rluiz"

#define KERNEL_PANIC(msg) kernel_panic(__FILE__, __LINE__, msg)

void NORETURN kernel_panic(const char *file, int line, const char *msg);

#define KERNEL_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            KERNEL_PANIC(msg); \
        } \
    } while (0)

void printk(const char *format, ...);

void kernel_main(void);

#endif
