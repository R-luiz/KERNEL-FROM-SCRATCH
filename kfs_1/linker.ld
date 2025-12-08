/* ************************************************************************** */
/*                                                                            */
/*   KFS_1 - Kernel From Scratch                                              */
/*                                                                            */
/*   linker.ld - Kernel Linker Script                                         */
/*                                                                            */
/*   This script tells the linker how to arrange the kernel in memory:        */
/*   - Where to load the kernel (1MB = 0x100000)                              */
/*   - How to order the sections                                              */
/*   - What the entry point is                                                */
/*                                                                            */
/* ************************************************************************** */

/*
** Entry point: the _start symbol from boot.asm
** This is where GRUB will jump after loading the kernel
*/
ENTRY(_start)

/*
** Memory layout:
** 0x00000000 - 0x000FFFFF : Reserved (BIOS, VGA, etc.)
** 0x00100000 - onwards    : Kernel
*/

SECTIONS
{
    /*
    ** Start at 1MB (0x100000)
    ** The first 1MB of RAM is reserved for BIOS and legacy devices
    */
    . = 1M;

    /*
    ** .text section - Executable code
    ** The multiboot header MUST come first (within first 8KB)
    */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)       /* Multiboot header first! */
        *(.text)            /* All other code */
    }

    /*
    ** .rodata section - Read-only data
    ** Constants, string literals, etc.
    */
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
    }

    /*
    ** .data section - Initialized read-write data
    ** Global/static variables with initial values
    */
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }

    /*
    ** .bss section - Uninitialized data
    ** Global/static variables without initial values (zeroed at startup)
    ** Note: BSS doesn't take space in the kernel binary, only in memory
    */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

    /*
    ** Symbols for kernel use
    ** These can be used to know where sections begin/end
    */
    _kernel_end = .;

    /*
    ** Discard unnecessary sections
    ** These are not needed in a freestanding kernel
    */
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
}
