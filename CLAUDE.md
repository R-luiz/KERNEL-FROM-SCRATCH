# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KFS_1 (Kernel From Scratch) is a minimal 32-bit x86 kernel that boots via GRUB and displays output to VGA text mode. The kernel is written to comply with NASA/JPL C Coding Standards, emphasizing safety and reliability through strict compilation flags and coding practices.

## Build Commands

All commands are run from the `kfs_1/` directory using the provided Makefile:

```bash
cd kfs_1

# Build kernel binary
make all

# Clean object files
make clean

# Remove all generated files (including ISO)
make fclean

# Rebuild from scratch
make re

# Create bootable ISO image
make iso
```

## Running and Testing

```bash
# Run kernel directly in QEMU (software emulation)
make run

# Run with KVM acceleration
make run-kvm

# Boot from ISO image
make run-iso
make run-iso-kvm

# Debug with GDB (starts QEMU with GDB server on port 1234)
make debug
# Then connect: gdb -ex 'target remote :1234' build/kernel.bin

# Analyze kernel binary
make check

# Verify ISO size (must be < 10MB)
make check-iso
```

## Code Architecture

### Boot Sequence

1. **boot.asm** (`src/boot/boot.asm`): Multiboot header and entry point
   - Contains the Multiboot header (must be in first 8KB)
   - Sets up 16KB stack
   - Calls `kernel_main()` from C code
   - Passes Multiboot magic number and info structure to kernel

2. **linker.ld**: Memory layout script
   - Loads kernel at 1MB (0x100000) physical address
   - Orders sections: `.multiboot`, `.text`, `.rodata`, `.data`, `.bss`
   - Entry point: `_start` symbol from boot.asm

3. **kernel.c** (`src/kernel/kernel.c`): Main kernel entry point
   - Initializes VGA terminal
   - Displays kernel banner and "42" (mandatory requirement)
   - Demonstrates `printk()` formatting
   - Halts CPU in infinite loop

### Core Modules

**VGA Driver** (`src/drivers/vga.c`, `vga.h`)
- Text mode output at memory address 0xB8000
- 80x25 character display
- 16 foreground and 8 background colors
- Hardware cursor control via I/O ports 0x3D4/0x3D5
- Automatic scrolling when screen is full
- Special character handling: `\n`, `\r`, `\t`, `\b`

**String Library** (`src/lib/string.c`, `string.h`)
- Memory functions: `k_memset()`, `k_memcpy()`, `k_memmove()`, `k_memcmp()`
- String functions: `k_strlen()`, `k_strcmp()`, `k_strncmp()`, `k_strcpy()`, `k_strncpy()`
- Number conversion: `k_itoa()` (signed), `k_utoa()` (unsigned)
- All functions prefixed with `k_` to avoid conflicts with standard library

**Types** (`src/include/types.h`)
- Fixed-width types: `uint8_t`, `uint16_t`, `uint32_t`, `int32_t`, etc.
- Size types: `size_t`, `ssize_t`
- Boolean type: `bool_t` with `TRUE`/`FALSE`
- Compiler attributes: `PACKED`, `ALIGNED()`, `NORETURN`, `UNUSED`, `ALWAYS_INLINE`
- Compile-time assertions via `STATIC_ASSERT()` macro

### Kernel Utilities

**Printk** (in `kernel.c`)
- Printf-like formatting for kernel debugging
- Supported format specifiers: `%s`, `%c`, `%d`, `%i`, `%u`, `%x`, `%X`, `%p`, `%%`
- Uses built-in variadic argument support (`__builtin_va_list`)

**Kernel Panic** (in `kernel.c`)
- Error display with white-on-red color scheme
- Shows file, line number, and error message
- Halts CPU permanently
- Use via `KERNEL_PANIC(msg)` macro or `KERNEL_ASSERT(condition, msg)`

## NASA/JPL C Coding Standards Compliance

The codebase follows these strict rules:

1. **No recursion**: All functions use iteration
2. **Bounded loops**: Every loop has a computable upper bound
3. **Functions ≤ 60 lines**: Keep functions short and focused
4. **Narrow scope**: Declare variables at narrowest possible scope
5. **Assertions**: Use `KERNEL_ASSERT()` for runtime checks
6. **Return value checking**: Most functions return void; pointer returns checked for NULL

### Compiler Flags

Strict NASA-compliant warning flags are enabled:
- `-Wall -Wextra -Werror -pedantic`: Basic strictness
- `-Wshadow -Wpointer-arith -Wcast-align`: Prevent common bugs
- `-Wmissing-prototypes -Wmissing-declarations`: Enforce declarations
- `-Wconversion -Wundef`: Catch type and macro issues

Freestanding kernel flags:
- `-m32 -ffreestanding -nostdlib -nodefaultlibs`: 32-bit, no standard library
- `-fno-builtin -fno-stack-protector -fno-pic -fno-pie`: Minimal dependencies

## Development Constraints

- **No standard library**: Cannot use `<stdio.h>`, `<stdlib.h>`, `<string.h>`, etc.
- **Freestanding environment**: Only compiler-provided features available
- **32-bit x86**: Code targets i386 architecture
- **No dynamic memory**: No malloc/free (not yet implemented)
- **No floating point**: Kernel runs in protected mode without FPU setup
- **ISO size limit**: Final ISO must be under 10MB

## Adding New Features

When adding functionality:

1. Follow NASA rules (no recursion, bounded loops, ≤60 lines per function)
2. Use `k_*` prefix for library functions to avoid conflicts
3. Include detailed comments explaining purpose and constraints
4. Update relevant header files with prototypes
5. Add new source files to `Makefile` in `C_SRCS` or `ASM_SRCS`
6. Test with both `make run` and `make debug`

## Directory Structure

```
kfs_1/
├── Makefile              # Build system
├── linker.ld            # Kernel linker script
├── src/
│   ├── boot/
│   │   └── boot.asm     # Assembly entry point + Multiboot header
│   ├── kernel/
│   │   ├── kernel.c     # Main kernel logic
│   │   └── kernel.h     # Kernel interface
│   ├── drivers/
│   │   ├── vga.c        # VGA text mode driver
│   │   └── vga.h        # VGA interface
│   ├── lib/
│   │   ├── string.c     # String/memory utilities
│   │   └── string.h     # String interface
│   └── include/
│       └── types.h      # Type definitions
├── iso/
│   └── boot/grub/
│       └── grub.cfg     # GRUB bootloader config
└── build/               # Generated files (object files, kernel.bin)
```

## Toolchain Requirements

- **gcc**: Cross-compiler or native with `-m32` support
- **nasm**: Netwide Assembler for boot.asm
- **ld**: GNU linker
- **grub-mkrescue**: For creating bootable ISOs
- **qemu-system-i386**: For testing (optional: KVM for acceleration)
- **gdb**: For debugging (optional)

## Memory Map

- `0x00000000 - 0x000FFFFF`: Reserved (BIOS, VGA memory at 0xB8000)
- `0x00100000 - onwards`: Kernel code and data (1MB mark)
- Stack: 16KB allocated in BSS section, grows downward
