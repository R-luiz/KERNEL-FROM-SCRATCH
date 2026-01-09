#ifndef GDT_H
# define GDT_H

# include "types.h"

# define GDT_ADDRESS            0x00000800

# define GDT_ENTRIES            7

# define GDT_NULL_SELECTOR      0x00
# define GDT_KERNEL_CODE        0x08
# define GDT_KERNEL_DATA        0x10
# define GDT_KERNEL_STACK       0x18
# define GDT_USER_CODE          0x20
# define GDT_USER_DATA          0x28
# define GDT_USER_STACK         0x30

# define GDT_ACCESS_PRESENT     (1 << 7)
# define GDT_ACCESS_RING0       (0 << 5)
# define GDT_ACCESS_RING3       (3 << 5)
# define GDT_ACCESS_DESCRIPTOR  (1 << 4)
# define GDT_ACCESS_EXECUTABLE  (1 << 3)
# define GDT_ACCESS_DC          (1 << 2)
# define GDT_ACCESS_RW          (1 << 1)
# define GDT_ACCESS_ACCESSED    (1 << 0)

# define GDT_KERNEL_CODE_ACCESS 0x9A
# define GDT_KERNEL_DATA_ACCESS 0x92
# define GDT_USER_CODE_ACCESS   0xFA
# define GDT_USER_DATA_ACCESS   0xF2

# define GDT_FLAG_GRANULARITY   (1 << 3)
# define GDT_FLAG_32BIT         (1 << 2)
# define GDT_FLAG_LONG_MODE     (1 << 1)

# define GDT_FLAGS_32BIT        0x0C

typedef struct PACKED s_gdt_entry
{
    uint16_t    limit_low;
    uint16_t    base_low;
    uint8_t     base_middle;
    uint8_t     access;
    uint8_t     flags_limit;
    uint8_t     base_high;
}   t_gdt_entry;

typedef struct PACKED s_gdt_ptr
{
    uint16_t    limit;
    uint32_t    base;
}   t_gdt_ptr;

void    gdt_init(void);

void    gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags);

void    gdt_print(void);

extern void gdt_flush(uint32_t gdt_ptr);

#endif
