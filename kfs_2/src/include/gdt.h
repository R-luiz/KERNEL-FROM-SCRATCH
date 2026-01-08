/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   gdt.h - Global Descriptor Table Interface                                */
/*                                                                            */
/*   The GDT defines memory segments with base address, size, and access      */
/*   privileges. Required segments for KFS-2:                                 */
/*     - Kernel Code, Data, Stack (ring 0)                                    */
/*     - User Code, Data, Stack (ring 3)                                      */
/*                                                                            */
/*   GDT must be located at address 0x00000800 per subject requirements.      */
/*                                                                            */
/* ************************************************************************** */

#ifndef GDT_H
# define GDT_H

# include "types.h"

/* ============================================================================
 * GDT Constants
 * ============================================================================ */

/* GDT must be at this address per KFS-2 subject */
# define GDT_ADDRESS            0x00000800

/* Number of GDT entries (including null descriptor) */
# define GDT_ENTRIES            7

/* Segment selectors (index * 8 because each entry is 8 bytes) */
# define GDT_NULL_SELECTOR      0x00    /* Null descriptor (required) */
# define GDT_KERNEL_CODE        0x08    /* Kernel code segment */
# define GDT_KERNEL_DATA        0x10    /* Kernel data segment */
# define GDT_KERNEL_STACK       0x18    /* Kernel stack segment */
# define GDT_USER_CODE          0x20    /* User code segment (ring 3) */
# define GDT_USER_DATA          0x28    /* User data segment (ring 3) */
# define GDT_USER_STACK         0x30    /* User stack segment (ring 3) */

/* ============================================================================
 * GDT Access Byte Flags
 * ============================================================================
 *
 * Access byte structure (8 bits):
 *   Bit 7: Present (P)        - Must be 1 for valid segments
 *   Bit 6-5: DPL              - Descriptor Privilege Level (0=kernel, 3=user)
 *   Bit 4: Descriptor Type(S) - 1 for code/data segments
 *   Bit 3: Executable (E)     - 1 for code, 0 for data
 *   Bit 2: DC                 - Direction/Conforming bit
 *   Bit 1: RW                 - Readable (code) / Writable (data)
 *   Bit 0: Accessed (A)       - CPU sets this when segment is accessed
 */

# define GDT_ACCESS_PRESENT     (1 << 7)    /* Segment is present */
# define GDT_ACCESS_RING0       (0 << 5)    /* Ring 0 (kernel) */
# define GDT_ACCESS_RING3       (3 << 5)    /* Ring 3 (user) */
# define GDT_ACCESS_DESCRIPTOR  (1 << 4)    /* Code/data descriptor */
# define GDT_ACCESS_EXECUTABLE  (1 << 3)    /* Code segment */
# define GDT_ACCESS_DC          (1 << 2)    /* Direction/Conforming */
# define GDT_ACCESS_RW          (1 << 1)    /* Readable/Writable */
# define GDT_ACCESS_ACCESSED    (1 << 0)    /* Accessed bit */

/* Pre-computed access bytes for common segment types */
# define GDT_KERNEL_CODE_ACCESS 0x9A    /* P=1, DPL=0, S=1, E=1, RW=1 */
# define GDT_KERNEL_DATA_ACCESS 0x92    /* P=1, DPL=0, S=1, E=0, RW=1 */
# define GDT_USER_CODE_ACCESS   0xFA    /* P=1, DPL=3, S=1, E=1, RW=1 */
# define GDT_USER_DATA_ACCESS   0xF2    /* P=1, DPL=3, S=1, E=0, RW=1 */

/* ============================================================================
 * GDT Flags (upper 4 bits of flags/limit byte)
 * ============================================================================
 *
 * Flags structure (4 bits):
 *   Bit 3: Granularity (G)  - 0=byte, 1=4KB pages
 *   Bit 2: Size (D/B)       - 0=16-bit, 1=32-bit
 *   Bit 1: Long mode (L)    - 1 for 64-bit code (0 for 32-bit)
 *   Bit 0: Available (AVL)  - Available for system use
 */

# define GDT_FLAG_GRANULARITY   (1 << 3)    /* 4KB granularity */
# define GDT_FLAG_32BIT         (1 << 2)    /* 32-bit protected mode */
# define GDT_FLAG_LONG_MODE     (1 << 1)    /* 64-bit mode (not used) */

/* Standard flags: 4KB granularity + 32-bit = 0xC */
# define GDT_FLAGS_32BIT        0x0C

/* ============================================================================
 * GDT Structures
 * ============================================================================ */

/*
 * GDT Entry structure (8 bytes)
 *
 * Memory layout:
 *   Bytes 0-1: Limit bits 0-15
 *   Bytes 2-3: Base bits 0-15
 *   Byte 4:    Base bits 16-23
 *   Byte 5:    Access byte
 *   Byte 6:    Flags (4 bits) + Limit bits 16-19 (4 bits)
 *   Byte 7:    Base bits 24-31
 */
typedef struct PACKED s_gdt_entry
{
    uint16_t    limit_low;      /* Limit bits 0-15 */
    uint16_t    base_low;       /* Base bits 0-15 */
    uint8_t     base_middle;    /* Base bits 16-23 */
    uint8_t     access;         /* Access byte */
    uint8_t     flags_limit;    /* Flags (4 bits) + Limit bits 16-19 */
    uint8_t     base_high;      /* Base bits 24-31 */
}   t_gdt_entry;

/*
 * GDT Pointer structure (6 bytes)
 * Used with LGDT instruction to load the GDT
 */
typedef struct PACKED s_gdt_ptr
{
    uint16_t    limit;          /* GDT size - 1 */
    uint32_t    base;           /* GDT base address */
}   t_gdt_ptr;

/* ============================================================================
 * Function Prototypes
 * ============================================================================ */

/*
 * Initialize the GDT at address 0x800 with all required segments.
 * This function:
 *   1. Creates entries for null, kernel, and user segments
 *   2. Copies the GDT to address 0x800
 *   3. Loads the GDT using LGDT instruction
 *   4. Reloads segment registers
 */
void    gdt_init(void);

/*
 * Set a GDT entry with the specified parameters.
 *
 * Parameters:
 *   index  - GDT entry index (0-6)
 *   base   - Segment base address
 *   limit  - Segment limit (in bytes or 4KB pages depending on granularity)
 *   access - Access byte defining segment properties
 *   flags  - Flags (granularity, size)
 */
void    gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags);

/*
 * Print GDT information for debugging.
 * Shows all segment entries with their base, limit, and access rights.
 */
void    gdt_print(void);

/* External function in assembly to flush/reload segment registers */
extern void gdt_flush(uint32_t gdt_ptr);

#endif /* GDT_H */
