/* ************************************************************************** */
/*                                                                            */
/*   KFS_2 - Kernel From Scratch                                              */
/*                                                                            */
/*   stack.c - Stack Inspection and Debug Implementation                      */
/*                                                                            */
/*   Provides functions to inspect and print the kernel stack.                */
/*   Required by KFS-2 subject for debugging purposes.                        */
/*                                                                            */
/* ************************************************************************** */

#include "stack.h"
#include "types.h"

/* External printk for output */
extern void printk(const char *fmt, ...);

/* ============================================================================
 * Register Access Functions (inline assembly)
 * ============================================================================ */

/*
 * Get current stack pointer.
 * Uses inline assembly to read ESP register.
 */
uint32_t    stack_get_esp(void)
{
    uint32_t    esp;

    __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
    return esp;
}

/*
 * Get current base pointer.
 * Uses inline assembly to read EBP register.
 */
uint32_t    stack_get_ebp(void)
{
    uint32_t    ebp;

    __asm__ __volatile__("mov %%ebp, %0" : "=r"(ebp));
    return ebp;
}

/*
 * Get approximate instruction pointer.
 * Returns the return address of the caller (since we can't directly read EIP).
 */
uint32_t    stack_get_eip(void)
{
    uint32_t    eip;

    /* Get return address from stack - this is the caller's EIP */
    __asm__ __volatile__(
        "mov 4(%%ebp), %0"
        : "=r"(eip)
    );
    return eip;
}

/*
 * Get current EFLAGS register.
 * Uses PUSHFD to push flags onto stack, then pops into variable.
 */
uint32_t    stack_get_eflags(void)
{
    uint32_t    eflags;

    __asm__ __volatile__(
        "pushfl\n\t"
        "pop %0"
        : "=r"(eflags)
    );
    return eflags;
}

/* ============================================================================
 * Static Helper Functions
 * ============================================================================ */

/*
 * Get segment register values.
 */
static void get_segment_registers(uint16_t *cs, uint16_t *ds, uint16_t *es,
                                   uint16_t *fs, uint16_t *gs, uint16_t *ss)
{
    __asm__ __volatile__("mov %%cs, %0" : "=r"(*cs));
    __asm__ __volatile__("mov %%ds, %0" : "=r"(*ds));
    __asm__ __volatile__("mov %%es, %0" : "=r"(*es));
    __asm__ __volatile__("mov %%fs, %0" : "=r"(*fs));
    __asm__ __volatile__("mov %%gs, %0" : "=r"(*gs));
    __asm__ __volatile__("mov %%ss, %0" : "=r"(*ss));
}

/*
 * Print EFLAGS register in human-readable format.
 */
static void print_eflags(uint32_t eflags)
{
    printk("EFLAGS: 0x%x [", eflags);

    /* Carry Flag */
    if (eflags & (1 << 0))
        printk("CF ");
    /* Parity Flag */
    if (eflags & (1 << 2))
        printk("PF ");
    /* Auxiliary Carry Flag */
    if (eflags & (1 << 4))
        printk("AF ");
    /* Zero Flag */
    if (eflags & (1 << 6))
        printk("ZF ");
    /* Sign Flag */
    if (eflags & (1 << 7))
        printk("SF ");
    /* Trap Flag */
    if (eflags & (1 << 8))
        printk("TF ");
    /* Interrupt Enable Flag */
    if (eflags & (1 << 9))
        printk("IF ");
    /* Direction Flag */
    if (eflags & (1 << 10))
        printk("DF ");
    /* Overflow Flag */
    if (eflags & (1 << 11))
        printk("OF ");
    /* I/O Privilege Level */
    printk("IOPL=%d ", (int)((eflags >> 12) & 0x03));
    /* Nested Task Flag */
    if (eflags & (1 << 14))
        printk("NT ");
    /* Resume Flag */
    if (eflags & (1 << 16))
        printk("RF ");
    /* Virtual 8086 Mode */
    if (eflags & (1 << 17))
        printk("VM ");

    printk("]\n");
}

/* ============================================================================
 * Public Functions
 * ============================================================================ */

/*
 * Print all CPU registers in a formatted display.
 */
void    stack_print_registers(void)
{
    uint32_t    eax, ebx, ecx, edx;
    uint32_t    esi, edi, ebp, esp;
    uint32_t    eflags;
    uint16_t    cs, ds, es, fs, gs, ss;

    /* Get general purpose registers */
    __asm__ __volatile__(
        "mov %%eax, %0\n\t"
        "mov %%ebx, %1\n\t"
        "mov %%ecx, %2\n\t"
        "mov %%edx, %3"
        : "=m"(eax), "=m"(ebx), "=m"(ecx), "=m"(edx)
    );
    __asm__ __volatile__(
        "mov %%esi, %0\n\t"
        "mov %%edi, %1"
        : "=m"(esi), "=m"(edi)
    );

    ebp = stack_get_ebp();
    esp = stack_get_esp();
    eflags = stack_get_eflags();
    get_segment_registers(&cs, &ds, &es, &fs, &gs, &ss);

    printk("\n=== CPU Registers ===\n");
    printk("EAX: 0x%x    EBX: 0x%x\n", eax, ebx);
    printk("ECX: 0x%x    EDX: 0x%x\n", ecx, edx);
    printk("ESI: 0x%x    EDI: 0x%x\n", esi, edi);
    printk("EBP: 0x%x    ESP: 0x%x\n", ebp, esp);
    printk("\n");
    print_eflags(eflags);
    printk("\n");
    printk("Segment Registers:\n");
    printk("  CS: 0x%x  DS: 0x%x  ES: 0x%x\n", cs, ds, es);
    printk("  FS: 0x%x  GS: 0x%x  SS: 0x%x\n", fs, gs, ss);
}

/*
 * Walk the stack and print a backtrace.
 * Follows the EBP chain to find return addresses.
 */
void    stack_trace(uint32_t max_frames)
{
    t_stack_frame   *frame;
    uint32_t        frame_num;

    printk("\n=== Stack Trace ===\n");

    /* Get current frame (caller's frame) */
    frame = (t_stack_frame *)stack_get_ebp();

    frame_num = 0;
    while (frame != NULL && frame_num < max_frames)
    {
        /* Sanity check: EBP should be within reasonable memory range */
        if ((uint32_t)frame < 0x1000 || (uint32_t)frame > 0xFFFFFF00)
        {
            printk("  #%d: [Invalid frame pointer: 0x%x]\n",
                   frame_num, (uint32_t)frame);
            break;
        }

        printk("  #%d: EIP=0x%x  EBP=0x%x\n",
               frame_num, frame->eip, (uint32_t)frame);

        /* Move to previous frame */
        frame = frame->ebp;
        frame_num++;
    }

    if (frame_num == 0)
    {
        printk("  (no frames to display)\n");
    }
    else if (frame_num >= max_frames)
    {
        printk("  ... (truncated at %d frames)\n", max_frames);
    }
}

/*
 * Dump raw stack memory.
 * Shows memory contents from current ESP upward.
 */
void    stack_dump(uint32_t num_words)
{
    uint32_t    *stack_ptr;
    uint32_t    esp;
    uint32_t    ebp;
    uint32_t    i;

    esp = stack_get_esp();
    ebp = stack_get_ebp();
    stack_ptr = (uint32_t *)esp;

    printk("\n=== Stack Dump ===\n");
    printk("ESP: 0x%x  EBP: 0x%x\n\n", esp, ebp);
    printk("Address      Value       Info\n");
    printk("------------ ----------- ----\n");

    for (i = 0; i < num_words; i++)
    {
        uint32_t addr = esp + (i * 4);
        uint32_t value = stack_ptr[i];

        printk("0x%x: 0x%x", addr, value);

        /* Mark special locations */
        if (addr == ebp)
        {
            printk("  <- EBP");
        }
        if (addr == esp)
        {
            printk("  <- ESP");
        }
        /* Check if value looks like a return address (in kernel space) */
        if (value >= 0x100000 && value < 0x200000)
        {
            printk("  (possible EIP)");
        }

        printk("\n");
    }
}

/*
 * Main stack print function - combines all debug info.
 * This is the primary function required by KFS-2 subject.
 */
void    stack_print(void)
{
    printk("\n");
    printk("========================================\n");
    printk("         KERNEL STACK DUMP             \n");
    printk("========================================\n");

    /* Print CPU registers */
    stack_print_registers();

    /* Print stack trace (backtrace) */
    stack_trace(10);

    /* Print raw stack dump (32 words = 128 bytes) */
    stack_dump(32);

    printk("\n========================================\n");
    printk("         END OF STACK DUMP             \n");
    printk("========================================\n\n");
}
