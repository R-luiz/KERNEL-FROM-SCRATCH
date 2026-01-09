#include "gdt.h"
#include "types.h"

extern void printk(const char *fmt, ...);

static t_gdt_entry  *g_gdt = (t_gdt_entry *)GDT_ADDRESS;

static t_gdt_ptr    g_gdt_ptr;

void    gdt_set_entry(uint32_t index, uint32_t base, uint32_t limit,
                      uint8_t access, uint8_t flags)
{
    t_gdt_entry *entry;


    if (index >= GDT_ENTRIES)
    {
        return;
    }

    entry = &g_gdt[index];


    entry->base_low = (uint16_t)(base & 0xFFFF);
    entry->base_middle = (uint8_t)((base >> 16) & 0xFF);
    entry->base_high = (uint8_t)((base >> 24) & 0xFF);


    entry->limit_low = (uint16_t)(limit & 0xFFFF);
    entry->flags_limit = (uint8_t)((flags << 4) | ((limit >> 16) & 0x0F));


    entry->access = access;
}

void    gdt_init(void)
{

    g_gdt_ptr.limit = (uint16_t)((sizeof(t_gdt_entry) * GDT_ENTRIES) - 1);
    g_gdt_ptr.base = GDT_ADDRESS;


    gdt_set_entry(0, 0, 0, 0, 0);


    gdt_set_entry(1, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_CODE_ACCESS, GDT_FLAGS_32BIT);


    gdt_set_entry(2, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_DATA_ACCESS, GDT_FLAGS_32BIT);


    gdt_set_entry(3, 0x00000000, 0xFFFFF,
                  GDT_KERNEL_DATA_ACCESS, GDT_FLAGS_32BIT);


    gdt_set_entry(4, 0x00000000, 0xFFFFF,
                  GDT_USER_CODE_ACCESS, GDT_FLAGS_32BIT);


    gdt_set_entry(5, 0x00000000, 0xFFFFF,
                  GDT_USER_DATA_ACCESS, GDT_FLAGS_32BIT);


    gdt_set_entry(6, 0x00000000, 0xFFFFF,
                  GDT_USER_DATA_ACCESS, GDT_FLAGS_32BIT);


    gdt_flush((uint32_t)&g_gdt_ptr);
}

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


        base = (uint32_t)entry->base_low |
               ((uint32_t)entry->base_middle << 16) |
               ((uint32_t)entry->base_high << 24);


        limit = (uint32_t)entry->limit_low |
                (((uint32_t)entry->flags_limit & 0x0F) << 16);

        printk("Entry %d [0x%x]: %s\n", i, i * 8, names[i]);
        printk("  Base:   0x%x\n", base);
        printk("  Limit:  0x%x", limit);


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
