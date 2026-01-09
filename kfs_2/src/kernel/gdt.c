/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   gdt.c - Global Descriptor Table Implementation                           */
/*                                                                            */
/*   This module creates and initializes the GDT at address 0x00000800        */
/*   with the following segments:                                             */
/*     0x00: Null descriptor                                                  */
/*     0x08: Kernel Code (ring 0)                                             */
/*     0x10: Kernel Data (ring 0)                                             */
/*     0x18: Kernel Stack (ring 0)                                            */
/*     0x20: User Code (ring 3)                                               */
/*     0x28: User Data (ring 3)                                               */
/*     0x30: User Stack (ring 3)                                              */
/*                                                                            */
/* ************************************************************************** */

#include "gdt.h"
#include "types.h"

/* External printk for debug output */
extern void printk(const char *fmt, ...);

/* ============================================================================
 * Static Variables
 * ============================================================================ */

/*
 * Pointer to GDT at address 0x800
 * We cast the fixed address to our structure pointer.
 * Note: This file is compiled with -Wno-array-bounds because the compiler
 * cannot statically verify bounds for hardware-mapped memory.
 */
static t_gdt_entry  *g_gdt = (t_gdt_entry *)GDT_ADDRESS;

/* GDT pointer structure for LGDT instruction */
static t_gdt_ptr    g_gdt_ptr;

/* ============================================================================
 * GDT Entry Setup
 * ============================================================================ */

/*
 * Set a single GDT entry.
 *
 * The entry is written directly to address 0x800 + (index * 8).
 *
 * Flat memory model: base=0, limit=0xFFFFF (with 4KB granularity = 4GB)
 *
 * Entry structure (8 bytes):
 *   [0-1] limit_low:   Lower 16 bits of limit
 *   [2-3] base_low:    Lower 16 bits of base
 *   [4]   base_middle: Bits 16-23 of base
 *   [5]   access:      Access byte (P, DPL, S, E, DC, RW, A)
 *   [6]   flags_limit: Flags (4 bits) | Limit bits 16-19 (4 bits)
 *   [7]   base_high:   Bits 24-31 of base
 */
void    gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags)
{
    t_gdt_entry *entry;

    /* Bounds check */
    if (index >= GDT_ENTRIES)
    {
        return;
    }

    entry = &g_gdt[index];

    /* Set the base address (split across 3 fields) */
    entry->base_low = (uint16_t)(base & 0xFFFF);
    entry->base_middle = (uint8_t)((base >> 16) & 0xFF);
    entry->base_high = (uint8_t)((base >> 24) & 0xFF);

    /* Set the limit (split across 2 fields) */
    entry->limit_low = (uint16_t)(limit & 0xFFFF);
    entry->flags_limit = (uint8_t)((flags << 4) | ((limit >> 16) & 0x0F));

    /* Set the access byte */
    entry->access = access;
}

/* ============================================================================
 * GDT Initialization
 * ============================================================================ */

/*
 * Initialize the Global Descriptor Table.
 *
 * Creates a flat memory model where all segments span 0x00000000 to 0xFFFFFFFF.
 * The 4KB granularity flag means limit of 0xFFFFF = 4GB address space.
 *
 * Segment layout:
 *   Index  Selector  Type          Ring  Access  Description
 *   0      0x00      Null          -     0x00    Required null descriptor
 *   1      0x08      Code          0     0x9A    Kernel code (execute/read)
 *   2      0x10      Data          0     0x92    Kernel data (read/write)
 *   3      0x18      Stack         0     0x92    Kernel stack (read/write)
 *   4      0x20      Code          3     0xFA    User code (execute/read)
 *   5      0x28      Data          3     0xF2    User data (read/write)
 *   6      0x30      Stack         3     0xF2    User stack (read/write)
 */
void    gdt_init(void)
{
    /* Set up the GDT pointer for LGDT instruction */
    g_gdt_ptr.limit = (uint16_t)((sizeof(t_gdt_entry) * GDT_ENTRIES) - 1);
    g_gdt_ptr.base = GDT_ADDRESS;

    /*
     * Entry 0: Null Descriptor (required by x86)
     * The first entry must be all zeros. The CPU will triple fault
     * if any segment register is loaded with selector 0x00.
     */
    gdt_set_entry(0, 0, 0, 0, 0);

    /*
     * Entry 1: Kernel Code Segment (selector 0x08)
     * - Base: 0x00000000 (start of memory)
     * - Limit: 0xFFFFF (with 4KB granularity = 4GB)
     * - Access: 0x9A = Present, Ring 0, Code, Execute/Read
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(1, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_CODE_ACCESS, GDT_FLAGS_32BIT);

    /*
     * Entry 2: Kernel Data Segment (selector 0x10)
     * - Base: 0x00000000
     * - Limit: 0xFFFFF (4GB)
     * - Access: 0x92 = Present, Ring 0, Data, Read/Write
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(2, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_DATA_ACCESS, GDT_FLAGS_32BIT);

    /*
     * Entry 3: Kernel Stack Segment (selector 0x18)
     * - Same as kernel data, used explicitly for stack operations
     * - Base: 0x00000000
     * - Limit: 0xFFFFF (4GB)
     * - Access: 0x92 = Present, Ring 0, Data, Read/Write
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(3, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_DATA_ACCESS, GDT_FLAGS_32BIT);

    /*
     * Entry 4: User Code Segment (selector 0x20)
     * - Base: 0x00000000
     * - Limit: 0xFFFFF (4GB)
     * - Access: 0xFA = Present, Ring 3, Code, Execute/Read
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(4, 0x00000000, 0xFFFFF,
                  GDT_USER_CODE_ACCESS, GDT_FLAGS_32BIT);

    /*
     * Entry 5: User Data Segment (selector 0x28)
     * - Base: 0x00000000
     * - Limit: 0xFFFFF (4GB)
     * - Access: 0xF2 = Present, Ring 3, Data, Read/Write
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(5, 0x00000000, 0xFFFFF,
                  GDT_USER_DATA_ACCESS, GDT_FLAGS_32BIT);

    /*
     * Entry 6: User Stack Segment (selector 0x30)
     * - Same as user data, used explicitly for user stack
     * - Base: 0x00000000
     * - Limit: 0xFFFFF (4GB)
     * - Access: 0xF2 = Present, Ring 3, Data, Read/Write
     * - Flags: 0x0C = 4KB granularity, 32-bit
     */
    gdt_set_entry(6, 0x00000000, 0xFFFFF,
                  GDT_USER_DATA_ACCESS, GDT_FLAGS_32BIT);

    /* Load the GDT and reload segment registers */
    gdt_flush((uint32_t)&g_gdt_ptr);
}

/* ============================================================================
 * GDT Debug Output
 * ============================================================================ */

/*
 * Print GDT entries for debugging.
 * Shows the configuration of each segment descriptor.
 */
void    gdt_print(void)
{
    static const char *names[GDT_ENTRIES] = {
        "Null",
        "Kernel Code",
        "Kernel Data",
        "Kernel Stack",
        "User Code",
        "User Data",
        "User Stack"
    };
    uint32_t    i;
    uint32_t    base;
    uint32_t    limit;
    t_gdt_entry *entry;

    printk("\n=== Global Descriptor Table (0x%x) ===\n", GDT_ADDRESS);
    printk("GDT Pointer: base=0x%x, limit=0x%x\n\n",
           g_gdt_ptr.base, g_gdt_ptr.limit);

    for (i = 0; i < GDT_ENTRIES; i++)
    {
        entry = &g_gdt[i];

        /* Reconstruct base address from split fields */
        base = (uint32_t)entry->base_low |
               ((uint32_t)entry->base_middle << 16) |
               ((uint32_t)entry->base_high << 24);

        /* Reconstruct limit from split fields */
        limit = (uint32_t)entry->limit_low |
                (((uint32_t)entry->flags_limit & 0x0F) << 16);

        printk("Entry %d [0x%x]: %s\n", i, i * 8, names[i]);
        printk("  Base:   0x%x\n", base);
        printk("  Limit:  0x%x", limit);

        /* Show actual limit if granularity is 4KB */
        if (entry->flags_limit & (GDT_FLAG_GRANULARITY << 4))
        {
            printk(" (4KB pages = %d MB)\n",
                   (uint32_t)(((limit + 1) * 4096) / (1024 * 1024)));
        }
        else
        {
            printk(" (bytes)\n");
        }

        printk("  Access: 0x%x", entry->access);

        /* Decode access byte */
        if (entry->access & GDT_ACCESS_PRESENT)
        {
            printk(" [Present");
            printk(" Ring%d", (entry->access >> 5) & 0x03);
            if (entry->access & GDT_ACCESS_EXECUTABLE)
            {
                printk(" Code");
            }
            else
            {
                printk(" Data");
            }
            if (entry->access & GDT_ACCESS_RW)
            {
                printk(" R/W");
            }
            printk("]");
        }
        printk("\n");

        printk("  Flags:  0x%x", (entry->flags_limit >> 4) & 0x0F);
        if (entry->flags_limit & (GDT_FLAG_GRANULARITY << 4))
        {
            printk(" [4KB");
        }
        else
        {
            printk(" [1B");
        }
        if (entry->flags_limit & (GDT_FLAG_32BIT << 4))
        {
            printk(" 32-bit]");
        }
        else
        {
            printk(" 16-bit]");
        }
        printk("\n\n");
    }
}
