# KFS_1 - Kernel From Scratch: Complete Documentation

This document provides a comprehensive line-by-line explanation of every file in the KFS_1 kernel project. The kernel is a minimal 32-bit x86 operating system kernel that boots via GRUB and displays output to VGA text mode.

---

## Table of Contents

1. [Project Overview](#project-overview)
2. [Directory Structure](#directory-structure)
3. [Boot Process Flow](#boot-process-flow)
4. [File Documentation](#file-documentation)
   - [boot.asm - Assembly Entry Point](#bootasm---assembly-entry-point)
   - [linker.ld - Linker Script](#linkerld---linker-script)
   - [types.h - Type Definitions](#typesh---type-definitions)
   - [kernel.h - Kernel Interface](#kernelh---kernel-interface)
   - [kernel.c - Kernel Implementation](#kernelc---kernel-implementation)
   - [vga.h - VGA Driver Interface](#vgah---vga-driver-interface)
   - [vga.c - VGA Driver Implementation](#vgac---vga-driver-implementation)
   - [string.h - String Library Interface](#stringh---string-library-interface)
   - [string.c - String Library Implementation](#stringc---string-library-implementation)
   - [Makefile - Build System](#makefile---build-system)
   - [grub.cfg - GRUB Configuration](#grubcfg---grub-configuration)
5. [NASA/JPL Coding Standards](#nasajpl-coding-standards)
6. [Memory Map](#memory-map)

---

## Project Overview

KFS_1 is a **freestanding kernel** - it runs without any operating system support. This means:
- No standard C library (`stdio.h`, `stdlib.h`, `string.h` are unavailable)
- No dynamic memory allocation (no `malloc`/`free`)
- Direct hardware access required
- Must implement all functionality from scratch

The kernel boots via the **Multiboot specification**, which allows GRUB (or any Multiboot-compliant bootloader) to load it.

---

## Directory Structure

```
kfs_1/
├── Makefile              # Build system configuration
├── linker.ld             # Memory layout for linking
├── iso/
│   └── boot/
│       └── grub/
│           └── grub.cfg  # GRUB bootloader configuration
└── src/
    ├── boot/
    │   └── boot.asm      # Assembly entry point + Multiboot header
    ├── kernel/
    │   ├── kernel.c      # Main kernel logic
    │   └── kernel.h      # Kernel interface declarations
    ├── drivers/
    │   ├── vga.c         # VGA text mode driver
    │   └── vga.h         # VGA interface declarations
    ├── lib/
    │   ├── string.c      # String/memory utilities
    │   └── string.h      # String interface declarations
    └── include/
        └── types.h       # Type definitions (uint8_t, etc.)
```

---

## Boot Process Flow

```
1. BIOS loads GRUB from disk
           ↓
2. GRUB finds kernel.bin on ISO/disk
           ↓
3. GRUB scans for Multiboot header (magic number 0x1BADB002)
           ↓
4. GRUB loads kernel to memory at 1MB (0x100000)
           ↓
5. GRUB jumps to _start (boot.asm)
   - CPU is in 32-bit protected mode
   - Interrupts disabled
   - EAX = Multiboot magic (0x2BADB002)
   - EBX = Multiboot info structure pointer
           ↓
6. boot.asm sets up stack, calls kernel_main()
           ↓
7. kernel_main() initializes VGA, displays output
           ↓
8. CPU halts in infinite loop
```

---

## File Documentation

---

### boot.asm - Assembly Entry Point

**File:** `src/boot/boot.asm`
**Purpose:** Contains the Multiboot header and CPU initialization
**Language:** NASM Assembly (32-bit x86)

```nasm
; Lines 1-12: File header comment block
; Standard project header identifying the file

; =============================================================================
; Lines 14-23: Multiboot Header Constants
; =============================================================================

MBOOT_PAGE_ALIGN    equ 1 << 0          ; Bit 0: Align modules on 4KB pages
```
**Explanation:** `equ` defines a constant. `1 << 0` is bit-shifting: `1` shifted left by `0` positions = `1` (binary: `0001`). This flag tells GRUB to align loaded modules on page boundaries.

```nasm
MBOOT_MEM_INFO      equ 1 << 1          ; Bit 1: Provide memory map
```
**Explanation:** `1 << 1` = `2` (binary: `0010`). This flag requests GRUB to provide memory information to the kernel.

```nasm
MBOOT_HEADER_MAGIC  equ 0x1BADB002      ; Multiboot magic number
```
**Explanation:** This specific hexadecimal value is defined by the Multiboot specification. GRUB scans for this exact number to identify valid kernels.

```nasm
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
```
**Explanation:** The `|` operator combines flags using bitwise OR. Result: `1 | 2 = 3` (binary: `0011`).

```nasm
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)
```
**Explanation:** The checksum ensures header integrity. The sum `MAGIC + FLAGS + CHECKSUM` must equal zero. This is a two's complement negation.

```nasm
; Line 29: Stack configuration
STACK_SIZE          equ 16384           ; 16 KB = 16 * 1024 bytes
```

```nasm
; =============================================================================
; Lines 37-41: Multiboot Header Section
; =============================================================================

section .multiboot
```
**Explanation:** Declares a new section named `.multiboot`. The linker script places this at the very beginning of the kernel binary. GRUB requires the Multiboot header to be within the first 8KB.

```nasm
align 4
```
**Explanation:** Aligns the following data to a 4-byte boundary. The Multiboot header must be 32-bit aligned.

```nasm
    dd MBOOT_HEADER_MAGIC               ; 4 bytes: Magic number
    dd MBOOT_HEADER_FLAGS               ; 4 bytes: Flags
    dd MBOOT_CHECKSUM                   ; 4 bytes: Checksum
```
**Explanation:** `dd` means "define doubleword" (4 bytes). These three 32-bit values form the complete Multiboot header (12 bytes total).

```nasm
; =============================================================================
; Lines 48-52: BSS Section - Stack Space
; =============================================================================

section .bss
```
**Explanation:** The BSS (Block Started by Symbol) section contains uninitialized data. It doesn't take space in the binary file but is allocated in memory at runtime.

```nasm
align 16
```
**Explanation:** Align stack to 16-byte boundary for performance and x86 ABI compliance.

```nasm
stack_bottom:
    resb STACK_SIZE                     ; Reserve 16384 bytes
stack_top:
```
**Explanation:**
- `stack_bottom:` is a label marking the start of stack memory
- `resb` means "reserve bytes" - allocates uninitialized space
- `stack_top:` marks the end (stack grows downward, so ESP points here initially)

```nasm
; =============================================================================
; Lines 58-60: Text Section - Executable Code
; =============================================================================

section .text
global _start                           ; Make _start visible to linker
extern kernel_main                      ; Import C function
```
**Explanation:**
- `.text` section contains executable code
- `global` exports a symbol for other files to reference
- `extern` declares a symbol defined elsewhere (in `kernel.c`)

```nasm
; =============================================================================
; Lines 74-98: _start - Kernel Entry Point
; =============================================================================

_start:
    cli                                 ; Clear Interrupt Flag
```
**Explanation:** `cli` disables hardware interrupts. This prevents interrupt handlers (which aren't set up yet) from being called.

```nasm
    mov esp, stack_top                  ; Set Stack Pointer
```
**Explanation:** `mov` copies `stack_top` address into ESP (Extended Stack Pointer). The stack is now ready for use.

```nasm
    xor ebp, ebp                        ; Clear Base Pointer
```
**Explanation:** `xor reg, reg` is an efficient way to set a register to zero. EBP=0 indicates the bottom of the call stack for debuggers.

```nasm
    push ebx                            ; Push Multiboot info pointer
    push eax                            ; Push Multiboot magic
```
**Explanation:** Push parameters onto the stack for `kernel_main()`. In cdecl calling convention, parameters are pushed right-to-left.

```nasm
    call kernel_main                    ; Call C kernel
```
**Explanation:** Transfers control to the C function `kernel_main()`. The return address is pushed onto the stack.

```nasm
.hang:
    cli                                 ; Disable interrupts
    hlt                                 ; Halt CPU until interrupt
    jmp .hang                           ; Loop (NMI can wake CPU)
```
**Explanation:**
- `.hang` is a local label (the `.` prefix makes it local)
- `hlt` puts the CPU into a low-power state
- The loop handles Non-Maskable Interrupts (NMI) that can wake the CPU

---

### linker.ld - Linker Script

**File:** `linker.ld`
**Purpose:** Defines how the linker combines object files into the final kernel binary
**Language:** GNU Linker Script

```ld
ENTRY(_start)
```
**Explanation:** Sets the entry point symbol. The ELF header will mark `_start` as where execution begins.

```ld
SECTIONS
{
    . = 1M;
```
**Explanation:**
- `SECTIONS` block defines memory layout
- `.` is the "location counter" (current address)
- `1M` = 1 megabyte = 0x100000. The kernel loads at the 1MB mark because:
  - Below 1MB is reserved for BIOS, VGA memory, etc.
  - GRUB's Multiboot protocol expects kernels at 1MB+

```ld
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)       /* Multiboot header first! */
        *(.text)            /* All other code */
    }
```
**Explanation:**
- `.text` section contains executable code
- `BLOCK(4K)` sets 4KB block granularity
- `ALIGN(4K)` aligns section start to 4KB boundary
- `*(.multiboot)` includes all `.multiboot` sections from all input files
- `*(.text)` includes all `.text` sections
- Order matters: Multiboot header must be first (within 8KB)

```ld
    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
        *(.rodata.*)
    }
```
**Explanation:** Read-only data (string literals, constants). The `*` suffix matches variants like `.rodata.str1.4`.

```ld
    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
        *(.data.*)
    }
```
**Explanation:** Initialized read-write data (global variables with initial values).

```ld
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }
```
**Explanation:**
- Uninitialized data (zero-initialized at runtime)
- `COMMON` includes uninitialized global variables
- BSS doesn't occupy space in the binary file

```ld
    _kernel_end = .;
```
**Explanation:** Defines a symbol at the current address. Can be used by the kernel to know where it ends in memory.

```ld
    /DISCARD/ :
    {
        *(.comment)
        *(.note)
        *(.note.*)
        *(.eh_frame)
        *(.eh_frame_hdr)
    }
```
**Explanation:** The `/DISCARD/` section throws away unwanted sections:
- `.comment` - compiler version info
- `.note` - debugging notes
- `.eh_frame` - C++ exception handling (not used in kernel)

---

### types.h - Type Definitions

**File:** `src/include/types.h`
**Purpose:** Provides fixed-width integer types since standard headers are unavailable
**Language:** C

```c
#ifndef TYPES_H
#define TYPES_H
```
**Explanation:** Include guards prevent multiple inclusion. If `TYPES_H` isn't defined, define it and include the file contents.

```c
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;
```
**Explanation:** `typedef` creates type aliases. These guarantee exact sizes:
| Type | Size | Range |
|------|------|-------|
| `uint8_t` | 1 byte | 0 to 255 |
| `uint16_t` | 2 bytes | 0 to 65,535 |
| `uint32_t` | 4 bytes | 0 to 4,294,967,295 |
| `uint64_t` | 8 bytes | 0 to 18,446,744,073,709,551,615 |

```c
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;
```
**Explanation:** Signed versions allow negative numbers using two's complement.

```c
typedef uint32_t            size_t;
typedef int32_t             ssize_t;
typedef uint32_t            uintptr_t;
typedef int32_t             intptr_t;
```
**Explanation:**
- `size_t` - type for sizes/counts (always unsigned)
- `ssize_t` - signed size (can indicate errors with -1)
- `uintptr_t` - unsigned integer that can hold a pointer value
- `intptr_t` - signed integer that can hold a pointer value

```c
typedef uint8_t             bool_t;

#define TRUE                ((bool_t)1)
#define FALSE               ((bool_t)0)
```
**Explanation:** Custom boolean type. Standard `bool` requires `<stdbool.h>` which is unavailable.

```c
#define NULL                ((void *)0)
```
**Explanation:** Null pointer constant. Cast to `void *` for type safety.

```c
#define PACKED              __attribute__((packed))
```
**Explanation:** GCC attribute that removes padding between struct members. Critical for hardware structures.

```c
#define ALIGNED(x)          __attribute__((aligned(x)))
```
**Explanation:** Forces alignment to `x` bytes. E.g., `ALIGNED(4096)` aligns to page boundary.

```c
#define NORETURN            __attribute__((noreturn))
```
**Explanation:** Tells compiler the function never returns (like `kernel_panic`). Enables optimizations.

```c
#define UNUSED              __attribute__((unused))
```
**Explanation:** Suppresses "unused variable/parameter" warnings.

```c
#define ALWAYS_INLINE       __attribute__((always_inline)) inline
```
**Explanation:** Forces the compiler to inline the function (no function call overhead).

```c
#define STATIC_ASSERT(cond, msg) \
    typedef char static_assertion_##msg[(cond) ? 1 : -1]
```
**Explanation:** Compile-time assertion. If `cond` is false, it creates a negative-sized array, causing a compiler error. The `##` operator concatenates tokens.

```c
STATIC_ASSERT(sizeof(uint8_t) == 1, uint8_t_must_be_1_byte);
STATIC_ASSERT(sizeof(uint16_t) == 2, uint16_t_must_be_2_bytes);
STATIC_ASSERT(sizeof(uint32_t) == 4, uint32_t_must_be_4_bytes);
STATIC_ASSERT(sizeof(uint64_t) == 8, uint64_t_must_be_8_bytes);
```
**Explanation:** Verifies type sizes at compile time. If the compiler's type sizes don't match expectations, compilation fails.

---

### kernel.h - Kernel Interface

**File:** `src/kernel/kernel.h`
**Purpose:** Declares kernel functions and macros
**Language:** C

```c
#include "../include/types.h"
#include "../drivers/vga.h"
#include "../lib/string.h"
```
**Explanation:** Includes dependencies. Relative paths navigate from `kernel/` directory.

```c
#define KERNEL_NAME         "KFS_1"
#define KERNEL_VERSION      "1.0.0"
#define KERNEL_AUTHOR       "rluiz"
```
**Explanation:** String constants for kernel identification.

```c
#define KERNEL_PANIC(msg) kernel_panic(__FILE__, __LINE__, msg)
```
**Explanation:** Macro wrapper for `kernel_panic()`. `__FILE__` and `__LINE__` are predefined macros that expand to the current source file and line number.

```c
void NORETURN kernel_panic(const char *file, int line, const char *msg);
```
**Explanation:** Function declaration. `NORETURN` indicates it never returns. Takes file name, line number, and error message.

```c
#define KERNEL_ASSERT(condition, msg) \
    do { \
        if (!(condition)) { \
            KERNEL_PANIC(msg); \
        } \
    } while (0)
```
**Explanation:**
- Runtime assertion macro
- `do { ... } while (0)` is a common C idiom for multi-statement macros
- If `condition` is false, triggers kernel panic

```c
void printk(const char *format, ...);
```
**Explanation:** Printf-like function declaration. `...` indicates variadic arguments (variable number of parameters).

```c
void kernel_main(void);
```
**Explanation:** Kernel entry point declaration. Called from `boot.asm`.

---

### kernel.c - Kernel Implementation

**File:** `src/kernel/kernel.c`
**Purpose:** Main kernel logic including panic handler and printk
**Language:** C

```c
#include "kernel.h"
```
**Explanation:** Includes the kernel header (which transitively includes all other headers).

```c
typedef __builtin_va_list   va_list;
#define va_start(ap, last)  __builtin_va_start(ap, last)
#define va_arg(ap, type)    __builtin_va_arg(ap, type)
#define va_end(ap)          __builtin_va_end(ap)
```
**Explanation:** Variadic argument handling using GCC built-ins (since `<stdarg.h>` is unavailable):
- `va_list` - type to hold argument list state
- `va_start(ap, last)` - initializes `ap`, `last` is the last named parameter
- `va_arg(ap, type)` - retrieves next argument of specified type
- `va_end(ap)` - cleanup

```c
void NORETURN kernel_panic(const char *file, int line, const char *msg)
{
    char line_str[12];
```
**Explanation:** Buffer for line number conversion (12 chars holds any 32-bit int).

```c
    vga_set_color(vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_RED));
    vga_clear();
```
**Explanation:** Sets white text on red background (error colors) and clears screen.

```c
    vga_putstr("\n\n");
    vga_putstr("  =============================================\n");
    vga_putstr("              KERNEL PANIC\n");
    // ... more display code ...
```
**Explanation:** Outputs panic banner to screen.

```c
    k_itoa(line, line_str, 10);
    vga_putstr(line_str);
```
**Explanation:** Converts integer line number to string, then displays it.

```c
    while (1)
    {
        __asm__ volatile ("cli");
        __asm__ volatile ("hlt");
    }
```
**Explanation:**
- `__asm__ volatile` embeds inline assembly
- `"cli"` disables interrupts
- `"hlt"` halts CPU
- `volatile` prevents compiler from optimizing away
- Infinite loop ensures CPU stays halted

```c
static void printk_putnum(uint32_t num, int base, int is_signed, int uppercase)
{
    char buffer[33];
```
**Explanation:** Helper function (static = file-local). Buffer holds up to 32 binary digits + null terminator.

```c
    if (is_signed)
    {
        k_itoa((int32_t)num, buffer, base);
    }
    else
    {
        k_utoa(num, buffer, base);
    }
```
**Explanation:** Uses appropriate conversion function based on signedness.

```c
    if (!uppercase && base == 16)
    {
        size_t i = 0;
        while (buffer[i] != '\0')
        {
            if (buffer[i] >= 'A' && buffer[i] <= 'F')
            {
                buffer[i] = buffer[i] + ('a' - 'A');
            }
            i++;
        }
    }
```
**Explanation:** Converts uppercase hex to lowercase. `'a' - 'A'` = 32 (ASCII difference).

```c
void printk(const char *format, ...)
{
    va_list args;
    size_t  i;
    char    c;

    if (format == NULL)
    {
        return;
    }
    va_start(args, format);
```
**Explanation:** Initializes variadic argument processing. `format` is the last named parameter.

```c
    i = 0;
    while (format[i] != '\0')
    {
        if (format[i] == '%' && format[i + 1] != '\0')
        {
            i++;
            c = format[i];
```
**Explanation:** Scans format string. When `%` is found, look at next character for format specifier.

```c
            if (c == 's')
            {
                const char *s = va_arg(args, const char *);
                vga_putstr(s != NULL ? s : "(null)");
            }
```
**Explanation:** `%s` handles strings. `va_arg` retrieves next argument as `const char *`.

```c
            else if (c == 'c')
            {
                char ch = (char)va_arg(args, int);
                vga_putchar(ch);
            }
```
**Explanation:** `%c` handles characters. Note: `char` is promoted to `int` in variadic calls.

```c
            else if (c == 'd' || c == 'i')
            {
                int32_t num = va_arg(args, int32_t);
                printk_putnum((uint32_t)num, 10, 1, 0);
            }
```
**Explanation:** `%d` and `%i` handle signed decimal integers.

```c
            else if (c == 'p')
            {
                uint32_t ptr = (uint32_t)va_arg(args, void *);
                vga_putstr("0x");
                printk_putnum(ptr, 16, 0, 0);
            }
```
**Explanation:** `%p` handles pointers. Prefixes with "0x" and prints in hexadecimal.

```c
    va_end(args);
}
```
**Explanation:** Cleanup for variadic argument processing.

```c
static void display_42_banner(void)
{
    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK));
    vga_putstr("\n");
    vga_putstr("        ##   #####  \n");
    // ... ASCII art ...
}
```
**Explanation:** Displays ASCII art "42" in cyan (project requirement).

```c
void kernel_main(void)
{
    vga_init();
```
**Explanation:** Entry point called from `boot.asm`. First initializes VGA driver.

```c
    vga_set_color(vga_make_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK));
    vga_putstr("===========================================\n");
    vga_putstr("  ");
    vga_putstr(KERNEL_NAME);
    // ... header display ...
```
**Explanation:** Displays kernel header in green.

```c
    display_42_banner();
```
**Explanation:** Shows the "42" requirement.

```c
    printk("printk test: string=%s, char=%c, int=%d\n", "hello", 'X', -42);
    printk("printk test: uint=%u, hex=%x, ptr=%p\n", 12345, 0xDEAD, (void *)0xB8000);
```
**Explanation:** Tests printk functionality with various format specifiers.

```c
    while (1)
    {
        __asm__ volatile ("hlt");
    }
}
```
**Explanation:** Halts CPU forever. The kernel's job is done.

---

### vga.h - VGA Driver Interface

**File:** `src/drivers/vga.h`
**Purpose:** Declares VGA text mode constants, types, and functions
**Language:** C

```c
#define VGA_MEMORY_ADDRESS  0xB8000
#define VGA_WIDTH           80
#define VGA_HEIGHT          25
#define VGA_SIZE            (VGA_WIDTH * VGA_HEIGHT)
```
**Explanation:**
- `0xB8000` - physical memory address where VGA text buffer is mapped
- 80 columns × 25 rows = 2000 character cells
- Each cell is 2 bytes (character + attribute)

**VGA Memory Layout:**
```
0xB8000: [char0][attr0][char1][attr1]...[char79][attr79]  <- Row 0
0xB80A0: [char80][attr80]...                              <- Row 1
...
```

```c
typedef enum e_vga_color
{
    VGA_COLOR_BLACK         = 0,
    VGA_COLOR_BLUE          = 1,
    VGA_COLOR_GREEN         = 2,
    VGA_COLOR_CYAN          = 3,
    VGA_COLOR_RED           = 4,
    VGA_COLOR_MAGENTA       = 5,
    VGA_COLOR_BROWN         = 6,
    VGA_COLOR_LIGHT_GREY    = 7,
    VGA_COLOR_DARK_GREY     = 8,
    VGA_COLOR_LIGHT_BLUE    = 9,
    VGA_COLOR_LIGHT_GREEN   = 10,
    VGA_COLOR_LIGHT_CYAN    = 11,
    VGA_COLOR_LIGHT_RED     = 12,
    VGA_COLOR_LIGHT_MAGENTA = 13,
    VGA_COLOR_YELLOW        = 14,
    VGA_COLOR_WHITE         = 15
}   t_vga_color;
```
**Explanation:** Standard VGA 16-color palette. Colors 0-7 are dark, 8-15 are bright versions.

**Attribute Byte Format:**
```
Bit:  7    6  5  4    3  2  1  0
      |    |__|__|    |__|__|__|
      |       |           |
   Blink  Background   Foreground
          (0-7)        (0-15)
```

```c
typedef struct s_vga_terminal
{
    size_t          cursor_row;
    size_t          cursor_col;
    uint8_t         current_color;
    volatile uint16_t   *buffer;
}   t_vga_terminal;
```
**Explanation:**
- `cursor_row`, `cursor_col` - current cursor position (0-indexed)
- `current_color` - attribute byte for new characters
- `buffer` - pointer to VGA memory (volatile prevents optimization)

---

### vga.c - VGA Driver Implementation

**File:** `src/drivers/vga.c`
**Purpose:** Implements VGA text mode output
**Language:** C

```c
static t_vga_terminal g_terminal;
```
**Explanation:** Global terminal state. `static` limits scope to this file.

```c
static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}
```
**Explanation:**
- `outb` writes a byte to an I/O port
- `__asm__ volatile` - inline assembly that won't be optimized away
- `"outb %0, %1"` - the instruction (output byte)
- `: :` - no outputs, two inputs
- `"a"(value)` - put `value` in AL register (constraint `a`)
- `"Nd"(port)` - put `port` in DX register (constraint `Nd`)

```c
static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;

    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return (ret);
}
```
**Explanation:**
- `inb` reads a byte from an I/O port
- `"=a"(ret)` - output constraint, result goes to `ret` via AL register

```c
static inline uint16_t vga_entry(char c, uint8_t color)
{
    return ((uint16_t)c | ((uint16_t)color << 8));
}
```
**Explanation:** Creates a 16-bit VGA entry: low byte is character, high byte is attribute.
```
Entry: [attribute byte][character byte]
       bits 15-8        bits 7-0
```

```c
static inline size_t vga_index(size_t x, size_t y)
{
    return (y * VGA_WIDTH + x);
}
```
**Explanation:** Converts 2D coordinates to linear buffer index.

```c
void vga_init(void)
{
    g_terminal.cursor_row = 0;
    g_terminal.cursor_col = 0;
    g_terminal.current_color = vga_make_color(VGA_COLOR_WHITE, VGA_COLOR_BLACK);
    g_terminal.buffer = (volatile uint16_t *)VGA_MEMORY_ADDRESS;
    vga_clear();
    vga_enable_cursor(14, 15);
}
```
**Explanation:** Initializes terminal state, clears screen, enables blinking cursor.

```c
uint8_t vga_make_color(t_vga_color fg, t_vga_color bg)
{
    return ((uint8_t)(fg | (bg << 4)));
}
```
**Explanation:** Combines foreground (bits 0-3) and background (bits 4-7) into attribute byte.

```c
void vga_clear(void)
{
    size_t      i;
    uint16_t    blank;

    blank = vga_entry(' ', g_terminal.current_color);
    i = 0;
    while (i < VGA_SIZE)
    {
        g_terminal.buffer[i] = blank;
        i++;
    }
    g_terminal.cursor_row = 0;
    g_terminal.cursor_col = 0;
    vga_update_cursor();
}
```
**Explanation:** Fills entire screen with spaces, resets cursor to top-left.

```c
void vga_scroll(void)
{
    size_t      i;
    uint16_t    blank;
    size_t      last_row_start;

    i = 0;
    while (i < (VGA_HEIGHT - 1) * VGA_WIDTH)
    {
        g_terminal.buffer[i] = g_terminal.buffer[i + VGA_WIDTH];
        i++;
    }
```
**Explanation:** Copies each row up by one (row 1→row 0, row 2→row 1, etc.).

```c
    blank = vga_entry(' ', g_terminal.current_color);
    last_row_start = (VGA_HEIGHT - 1) * VGA_WIDTH;
    i = 0;
    while (i < VGA_WIDTH)
    {
        g_terminal.buffer[last_row_start + i] = blank;
        i++;
    }
}
```
**Explanation:** Clears the last row with spaces.

```c
void vga_putchar(char c)
{
    if (c == '\n')
    {
        g_terminal.cursor_col = 0;
        g_terminal.cursor_row++;
    }
```
**Explanation:** Newline: move to start of next line.

```c
    else if (c == '\r')
    {
        g_terminal.cursor_col = 0;
    }
```
**Explanation:** Carriage return: move to start of current line.

```c
    else if (c == '\t')
    {
        g_terminal.cursor_col = (g_terminal.cursor_col + 4U) & ~3U;
    }
```
**Explanation:** Tab: advance to next 4-column boundary. `& ~3U` clears the lowest 2 bits, aligning to multiple of 4.

```c
    else if (c == '\b')
    {
        if (g_terminal.cursor_col > 0)
        {
            g_terminal.cursor_col--;
            vga_putchar_at(' ', g_terminal.current_color,
                g_terminal.cursor_col, g_terminal.cursor_row);
        }
    }
```
**Explanation:** Backspace: move back one column and erase character.

```c
    else
    {
        vga_putchar_at(c, g_terminal.current_color,
            g_terminal.cursor_col, g_terminal.cursor_row);
        g_terminal.cursor_col++;
    }
```
**Explanation:** Normal character: write at cursor position, advance cursor.

```c
    if (g_terminal.cursor_col >= VGA_WIDTH)
    {
        g_terminal.cursor_col = 0;
        g_terminal.cursor_row++;
    }
```
**Explanation:** Line wrap: if past column 79, move to next line.

```c
    if (g_terminal.cursor_row >= VGA_HEIGHT)
    {
        vga_scroll();
        g_terminal.cursor_row = VGA_HEIGHT - 1;
    }
    vga_update_cursor();
}
```
**Explanation:** Screen full: scroll up, keep cursor on last line.

```c
#define VGA_CTRL_REGISTER   0x3D4
#define VGA_DATA_REGISTER   0x3D5
```
**Explanation:** VGA CRTC (CRT Controller) I/O ports for hardware cursor control.

```c
void vga_enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xC0) | cursor_start);
    outb(VGA_CTRL_REGISTER, 0x0B);
    outb(VGA_DATA_REGISTER, (inb(VGA_DATA_REGISTER) & 0xE0) | cursor_end);
}
```
**Explanation:**
- Register `0x0A` controls cursor start scanline
- Register `0x0B` controls cursor end scanline
- `cursor_start=14, cursor_end=15` creates a thin underline cursor
- Masks preserve other bits while setting scanline values

```c
void vga_disable_cursor(void)
{
    outb(VGA_CTRL_REGISTER, 0x0A);
    outb(VGA_DATA_REGISTER, 0x20);
}
```
**Explanation:** Bit 5 of register `0x0A` disables the cursor when set.

```c
void vga_update_cursor(void)
{
    uint16_t pos;

    pos = (uint16_t)(g_terminal.cursor_row * VGA_WIDTH + g_terminal.cursor_col);
    outb(VGA_CTRL_REGISTER, 0x0F);
    outb(VGA_DATA_REGISTER, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REGISTER, 0x0E);
    outb(VGA_DATA_REGISTER, (uint8_t)((pos >> 8) & 0xFF));
}
```
**Explanation:**
- Register `0x0E` holds high byte of cursor position
- Register `0x0F` holds low byte of cursor position
- Position is linear index (0-1999)

---

### string.h - String Library Interface

**File:** `src/lib/string.h`
**Purpose:** Declares string/memory manipulation functions
**Language:** C

All functions use `k_` prefix to avoid conflicts with any compiler built-ins.

```c
void    *k_memset(void *dest, int c, size_t n);
```
**Explanation:** Fills `n` bytes at `dest` with byte value `c`.

```c
void    *k_memcpy(void *dest, const void *src, size_t n);
```
**Explanation:** Copies `n` bytes from `src` to `dest`. Buffers must not overlap.

```c
void    *k_memmove(void *dest, const void *src, size_t n);
```
**Explanation:** Like `memcpy` but handles overlapping buffers safely.

```c
int     k_memcmp(const void *s1, const void *s2, size_t n);
```
**Explanation:** Compares `n` bytes. Returns 0 if equal, <0 if s1<s2, >0 if s1>s2.

```c
size_t  k_strlen(const char *str);
```
**Explanation:** Returns length of null-terminated string (not including null).

```c
void    k_itoa(int32_t value, char *buffer, int base);
void    k_utoa(uint32_t value, char *buffer, int base);
```
**Explanation:** Convert integer to string. Base can be 2-16.

---

### string.c - String Library Implementation

**File:** `src/lib/string.c`
**Purpose:** Implements string/memory manipulation functions
**Language:** C

```c
void *k_memset(void *dest, int c, size_t n)
{
    uint8_t *ptr;
    size_t  i;

    if (dest == NULL)
    {
        return (NULL);
    }
    ptr = (uint8_t *)dest;
    i = 0;
    while (i < n)
    {
        ptr[i] = (uint8_t)c;
        i++;
    }
    return (dest);
}
```
**Explanation:**
- Cast `dest` to byte pointer for byte-by-byte access
- NULL check follows NASA defensive coding
- Returns original pointer for chaining

```c
void *k_memmove(void *dest, const void *src, size_t n)
{
    // ...
    if (d < s)
    {
        i = 0;
        while (i < n)
        {
            d[i] = s[i];
            i++;
        }
    }
    else if (d > s)
    {
        i = n;
        while (i > 0)
        {
            i--;
            d[i] = s[i];
        }
    }
    return (dest);
}
```
**Explanation:** Handles overlapping regions:
- If `dest < src`: copy forward (start to end)
- If `dest > src`: copy backward (end to start)
- This prevents overwriting source data before it's copied

```c
void k_itoa(int32_t value, char *buffer, int base)
{
    static const char   digits[] = "0123456789ABCDEF";
    char                temp[33];
    int                 i;
    int                 j;
    int                 is_negative;
    uint32_t            uvalue;
```
**Explanation:**
- `digits` array for number-to-character conversion
- `temp` holds digits in reverse order
- 33 bytes: 32 binary digits + null

```c
    is_negative = 0;
    if (value < 0 && base == 10)
    {
        is_negative = 1;
        uvalue = (uint32_t)(-(value + 1)) + 1;
    }
```
**Explanation:** Safe handling of `INT_MIN`. Direct `-value` would overflow for `-2147483648`. Instead: `-(value + 1) + 1` avoids overflow.

```c
    while (uvalue > 0 && i < 32)
    {
        temp[i] = digits[uvalue % (uint32_t)base];
        uvalue = uvalue / (uint32_t)base;
        i++;
    }
```
**Explanation:** Extract digits by repeated division:
- `uvalue % base` gets rightmost digit
- `uvalue / base` removes rightmost digit
- Loop bound `i < 32` is NASA compliance

```c
    j = 0;
    if (is_negative)
    {
        buffer[j] = '-';
        j++;
    }
    while (i > 0)
    {
        i--;
        buffer[j] = temp[i];
        j++;
    }
    buffer[j] = '\0';
```
**Explanation:** Reverse digits into output buffer, add minus sign if needed.

---

### Makefile - Build System

**File:** `Makefile`
**Purpose:** Automates compilation and linking
**Language:** GNU Make

```makefile
NAME            := kernel.bin
ISO             := kfs_1.iso
```
**Explanation:** `:=` is immediate assignment (expanded once when defined).

```makefile
CC              := gcc
AS              := nasm
LD              := ld
```
**Explanation:** Toolchain programs:
- `gcc` - C compiler
- `nasm` - Netwide Assembler
- `ld` - GNU linker

```makefile
KERNEL_FLAGS    := -m32 \
                   -ffreestanding \
                   -fno-builtin \
                   -fno-exceptions \
                   -fno-stack-protector \
                   -nostdlib \
                   -nodefaultlibs \
                   -fno-pic \
                   -fno-pie \
                   -mno-red-zone
```
**Explanation:**
| Flag | Purpose |
|------|---------|
| `-m32` | Generate 32-bit code |
| `-ffreestanding` | No standard library assumptions |
| `-fno-builtin` | Don't use built-in function optimizations |
| `-fno-exceptions` | Disable C++ exceptions |
| `-fno-stack-protector` | Disable stack canaries |
| `-nostdlib` | Don't link standard library |
| `-nodefaultlibs` | Don't link default libraries |
| `-fno-pic` | No position-independent code |
| `-fno-pie` | No position-independent executable |
| `-mno-red-zone` | Disable red zone (required for kernels) |

```makefile
NASA_FLAGS      := -Wall \
                   -Wextra \
                   -Werror \
                   -pedantic \
                   ...
```
**Explanation:**
| Flag | Purpose |
|------|---------|
| `-Wall` | Enable common warnings |
| `-Wextra` | Enable extra warnings |
| `-Werror` | Treat warnings as errors |
| `-pedantic` | Strict ISO C compliance |
| `-Wshadow` | Warn on variable shadowing |
| `-Wconversion` | Warn on implicit type conversions |

```makefile
ASFLAGS         := -f elf32 -g
```
**Explanation:**
- `-f elf32` - Output 32-bit ELF format
- `-g` - Include debug symbols

```makefile
LDFLAGS         := -m elf_i386 -T linker.ld -nostdlib
```
**Explanation:**
- `-m elf_i386` - Produce 32-bit ELF executable
- `-T linker.ld` - Use custom linker script
- `-nostdlib` - Don't link standard startup files

```makefile
ASM_OBJS        := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))
```
**Explanation:** `patsubst` pattern substitution: converts `src/boot/boot.asm` to `build/boot/boot.o`.

```makefile
$(BUILD_DIR)/$(NAME): $(OBJS) linker.ld
	@echo "  LD      $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
```
**Explanation:**
- Target depends on all object files and linker script
- `@` prefix suppresses command echo
- `$@` is the target name

```makefile
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	@$(AS) $(ASFLAGS) -o $@ $<
```
**Explanation:**
- Pattern rule for `.asm` files
- `mkdir -p` creates directory if needed
- `$<` is the first prerequisite (source file)

```makefile
run: $(BUILD_DIR)/$(NAME)
	@qemu-system-i386 -kernel $(BUILD_DIR)/$(NAME) -m 32M
```
**Explanation:**
- Runs kernel in QEMU emulator
- `-kernel` loads multiboot kernel directly
- `-m 32M` allocates 32MB RAM

```makefile
debug: $(BUILD_DIR)/$(NAME)
	@qemu-system-i386 -kernel $(BUILD_DIR)/$(NAME) -m 32M -s -S
```
**Explanation:**
- `-s` starts GDB server on port 1234
- `-S` pauses execution at startup

---

### grub.cfg - GRUB Configuration

**File:** `iso/boot/grub/grub.cfg`
**Purpose:** Configures GRUB bootloader menu
**Language:** GRUB Script

```
set timeout=5
set default=0
```
**Explanation:**
- Wait 5 seconds before auto-booting
- Default to first menu entry (index 0)

```
menuentry "KFS_1 - Kernel From Scratch" {
    multiboot /boot/kernel.bin
    boot
}
```
**Explanation:**
- Creates menu entry with title
- `multiboot` loads kernel using Multiboot protocol
- Path is relative to ISO filesystem root
- `boot` starts the loaded kernel

```
menuentry "Reboot" {
    reboot
}

menuentry "Shutdown" {
    halt
}
```
**Explanation:** Additional menu options for system control.

---

## NASA/JPL Coding Standards

The codebase follows NASA/JPL's "Power of Ten" rules:

1. **No Recursion**: All functions use iteration only
2. **Bounded Loops**: Every loop has a computable maximum iteration count
3. **No Dynamic Memory**: No heap allocation (no malloc/free)
4. **Functions ≤ 60 Lines**: Small, focused functions
5. **Assertions**: Runtime checks via `KERNEL_ASSERT()`
6. **Minimum Variable Scope**: Declare at narrowest scope needed
7. **Check Return Values**: All return values must be used or explicitly ignored
8. **Limited Pointer Use**: Minimize pointer arithmetic
9. **Compile Cleanly**: Zero warnings with strict flags
10. **Test Coverage**: All code paths should be testable

---

## Memory Map

```
Address Range          | Description
-----------------------|------------------------------------------
0x00000000 - 0x000003FF| Real Mode IVT (Interrupt Vector Table)
0x00000400 - 0x000004FF| BIOS Data Area
0x00000500 - 0x00007BFF| Conventional Memory (usable)
0x00007C00 - 0x00007DFF| Boot Sector (loaded by BIOS)
0x00007E00 - 0x0007FFFF| Conventional Memory (usable)
0x00080000 - 0x0009FFFF| Extended BIOS Data Area
0x000A0000 - 0x000BFFFF| Video Memory (VGA at 0xB8000)
0x000C0000 - 0x000FFFFF| BIOS ROM
0x00100000 - onwards   | Extended Memory (KERNEL LOADS HERE)
```

The kernel loads at `0x00100000` (1MB) because:
- Below 1MB is reserved for legacy hardware
- GRUB's Multiboot protocol expects kernels at 1MB+
- This avoids conflicts with BIOS and VGA memory

---

## Quick Reference

### Building
```bash
cd kfs_1
make all        # Build kernel
make iso        # Create bootable ISO
make clean      # Remove object files
make fclean     # Remove all generated files
make re         # Full rebuild
```

### Running
```bash
make run        # Run in QEMU
make run-kvm    # Run with KVM acceleration
make run-iso    # Boot ISO in QEMU
make debug      # Debug with GDB (port 1234)
```

### Debugging
```bash
# Terminal 1
make debug

# Terminal 2
gdb build/kernel.bin
(gdb) target remote :1234
(gdb) break kernel_main
(gdb) continue
```

---

*Generated for KFS_1 v1.0.0*
