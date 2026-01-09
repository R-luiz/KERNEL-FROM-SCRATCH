# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

KFS (Kernel From Scratch) is a minimal 32-bit x86 operating system kernel that boots via GRUB and runs on both real hardware and virtual machines. Features include PS/2 keyboard and mouse support, 4 virtual terminals with scrollback, and full interrupt handling. Written to comply with NASA/JPL C Coding Standards.

### KFS-1 (Phase 1)
Basic kernel with VGA output, keyboard/mouse input, and virtual terminals.

### KFS-2 (Phase 2 - Current)
Extended with:
- GDT at 0x800 with 7 segments (null, kernel code/data/stack, user code/data/stack)
- Interactive shell with built-in commands
- Stack inspection and dump utilities
- CPU register display
- System control (reboot, halt)

## Build Commands

### Using WSL on Windows (Recommended)

```powershell
# Build the kernel
wsl -d Ubuntu -e bash -c "cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2' && make all"

# Create bootable ISO
wsl -d Ubuntu -e bash -c "cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2' && make iso"

# Run in QEMU (Windows)
& "C:\Program Files\qemu\qemu-system-i386.exe" -cdrom "C:\Users\luizr\Documents\Nouveau dossier\KERNEL-FROM-SCRATCH\kfs_2\kfs_2.iso" -m 32M
```

### Inside WSL Ubuntu

```bash
cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2'

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

### Windows with WSL + QEMU

```powershell
# Direct kernel boot
& "C:\Program Files\qemu\qemu-system-i386.exe" -kernel "C:\Users\luizr\Documents\Nouveau dossier\KERNEL-FROM-SCRATCH\kfs_2\build\kernel.bin" -m 32M

# Boot from ISO
& "C:\Program Files\qemu\qemu-system-i386.exe" -cdrom "C:\Users\luizr\Documents\Nouveau dossier\KERNEL-FROM-SCRATCH\kfs_2\kfs_2.iso" -m 32M
```

### Inside WSL Ubuntu

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

## Shell Commands (KFS-2)

| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `stack` | Print kernel stack dump (shows EBP chain and return addresses) |
| `gdt` | Display GDT entries at 0x800 (base, limit, flags) |
| `regs` | Display CPU registers (EAX, EBX, ECX, EDX, ESP, EBP, ESI, EDI, EFLAGS) |
| `clear` | Clear the screen |
| `info` | Show kernel info |
| `reboot` | Reboot system (via keyboard controller) |
| `halt` | Halt CPU (cli + hlt) |

## Keyboard Shortcuts

- **Alt+F1-F4**: Switch between virtual terminals
- **Mouse scroll wheel**: Scroll through terminal history

## Code Architecture

### Boot Sequence

1. **boot.asm** (`src/boot/boot.asm`): Multiboot header and entry point
   - Contains the Multiboot header (must be in first 8KB)
   - Sets up custom GDT with code (0x08) and data (0x10) selectors
   - Sets up 16KB stack
   - Calls `kernel_main()` from C code

2. **linker.ld**: Memory layout script
   - Loads kernel at 1MB (0x100000) physical address
   - Orders sections: `.multiboot`, `.text`, `.rodata`, `.data`, `.bss`
   - Entry point: `_start` symbol from boot.asm

3. **kernel.c** (`src/kernel/kernel.c`): Main kernel entry point
   - Initializes VGA, PIC, IDT, keyboard, mouse, virtual terminals
   - Displays kernel banner and "42" (mandatory requirement)
   - Enables interrupts and enters main loop

### Core Modules

**VGA Driver** (`src/drivers/vga.c`, `vga.h`)
- Text mode output at memory address 0xB8000
- 80x25 character display, 16 colors
- Hardware cursor control via I/O ports 0x3D4/0x3D5
- Special character handling: `\n`, `\r`, `\t`, `\b`

**Keyboard Driver** (`src/drivers/keyboard.c`, `include/keyboard.h`)
- PS/2 keyboard handling via port 0x60
- Full scancode-to-ASCII translation
- Modifier keys: Shift, Ctrl, Alt, Caps Lock
- Ring buffer for key event queue

**Mouse Driver** (`src/drivers/mouse.c`, `include/mouse.h`)
- PS/2 mouse with IntelliMouse scroll wheel detection
- 3-byte (standard) or 4-byte (scroll wheel) packet handling
- Ring buffer for mouse event queue
- IRQ12 on slave PIC

**Virtual Terminal System** (`src/kernel/vtty.c`, `include/vtty.h`)
- 4 independent terminals (Alt+F1-F4 to switch)
- 200-line scrollback buffer per terminal
- Mouse scroll wheel navigation through history
- Independent cursor, color, and content per terminal

### Interrupt System

**IDT** (`src/kernel/idt.c`, `include/idt.h`)
- 256-entry Interrupt Descriptor Table
- CPU exceptions (vectors 0-31)
- Hardware IRQs (vectors 32-47)
- Uses code selector 0x08 for all handlers

**PIC** (`src/kernel/pic.c`, `include/pic.h`)
- 8259 PIC initialization and remapping
- IRQs 0-7 mapped to vectors 32-39 (master)
- IRQs 8-15 mapped to vectors 40-47 (slave)
- Cascade enabled on IRQ2 for mouse support

**ISR** (`src/kernel/isr.c`)
- CPU exception handler (halts on exception)
- IRQ dispatcher for keyboard (IRQ1) and mouse (IRQ12)
- Alt+F1-F4 terminal switching logic

**Assembly Stubs** (`src/boot/interrupts.asm`)
- ISR stubs for all 256 vectors
- Save/restore CPU state
- Call C handlers with proper stack frame

### Support Libraries

**String Library** (`src/lib/string.c`, `string.h`)
- Memory: `k_memset()`, `k_memcpy()`, `k_memmove()`, `k_memcmp()`
- String: `k_strlen()`, `k_strcmp()`, `k_strncmp()`, `k_strcpy()`, `k_strncpy()`
- Number conversion: `k_itoa()`, `k_utoa()`

**Types** (`src/include/types.h`)
- Fixed-width types: `uint8_t`, `uint16_t`, `uint32_t`, `int8_t`, etc.
- Size types: `size_t`, `ssize_t`
- Boolean: `bool_t` with `TRUE`/`FALSE`
- Attributes: `PACKED`, `ALIGNED()`, `NORETURN`, `UNUSED`

### Kernel Utilities

**Printk** (in `kernel.c`)
- Printf-like formatting: `%s`, `%c`, `%d`, `%i`, `%u`, `%x`, `%X`, `%p`, `%%`
- Uses `__builtin_va_list` for variadic arguments

**Kernel Panic** (in `kernel.c`)
- White-on-red error display
- Shows file, line, message
- Use via `KERNEL_PANIC(msg)` or `KERNEL_ASSERT(cond, msg)`

## Directory Structure

```
kfs_1/                          # Phase 1: Basic Kernel
├── Makefile
├── linker.ld
├── iso/boot/grub/grub.cfg
└── src/
    ├── boot/
    │   ├── boot.asm
    │   └── interrupts.asm
    ├── kernel/
    │   ├── kernel.c, idt.c, pic.c, isr.c, vtty.c
    ├── drivers/
    │   ├── vga.c, keyboard.c, mouse.c
    ├── lib/
    │   └── string.c
    └── include/
        └── *.h

kfs_2/                          # Phase 2: GDT & Shell
├── Makefile
├── linker.ld
├── iso/boot/grub/grub.cfg
└── src/
    ├── boot/
    │   ├── boot.asm            # GDT at 0x800 with 7 segments
    │   └── interrupts.asm
    ├── kernel/
    │   ├── kernel.c            # Main entry, printk, panic
    │   ├── kernel.h            # Kernel constants
    │   ├── gdt.c               # GDT display utilities
    │   ├── shell.c             # Interactive shell commands
    │   ├── stack.c             # Stack inspection utilities
    │   ├── idt.c               # Interrupt Descriptor Table
    │   ├── pic.c               # PIC initialization
    │   ├── isr.c               # Interrupt handlers
    │   └── vtty.c              # Virtual terminal system
    ├── drivers/
    │   ├── vga.c / vga.h       # VGA text mode driver
    │   ├── keyboard.c          # PS/2 keyboard driver
    │   └── mouse.c             # PS/2 mouse driver
    ├── lib/
    │   ├── string.c / string.h # Memory/string utilities
    └── include/
        ├── types.h             # Fixed-width types, attributes
        ├── gdt.h               # GDT structures and interface
        ├── shell.h             # Shell command interface
        ├── stack.h             # Stack inspection interface
        ├── idt.h               # IDT structures
        ├── pic.h               # PIC constants
        ├── keyboard.h          # Keyboard interface
        ├── mouse.h             # Mouse interface
        └── vtty.h              # Terminal interface
```

## Technical Details

### GDT (at 0x800 in boot.asm)
| Selector | Segment | Description |
|----------|---------|-------------|
| 0x00 | Null | Required null descriptor |
| 0x08 | Kernel Code | Ring 0, 4GB, executable |
| 0x10 | Kernel Data | Ring 0, 4GB, read/write |
| 0x18 | Kernel Stack | Ring 0, 4GB, read/write |
| 0x20 | User Code | Ring 3, 4GB, executable |
| 0x28 | User Data | Ring 3, 4GB, read/write |
| 0x30 | User Stack | Ring 3, 4GB, read/write |

### IDT Vectors
| Vector | Handler |
|--------|---------|
| 0-31 | CPU exceptions |
| 32 | Timer (IRQ0) |
| 33 | Keyboard (IRQ1) |
| 44 | Mouse (IRQ12) |

### Memory Map
| Address | Content |
|---------|---------|
| 0x00000-0xFFFFF | Reserved (BIOS, VGA at 0xB8000) |
| 0x100000+ | Kernel code and data |

## NASA/JPL C Coding Standards

1. **No recursion**: All functions use iteration
2. **Bounded loops**: Every loop has computable upper bound
3. **Functions ≤ 60 lines**: Keep functions short and focused
4. **Narrow scope**: Declare variables at narrowest possible scope
5. **All returns checked**: Pointer returns checked for NULL
6. **No dynamic memory**: No malloc/free

### Compiler Flags

```
-Wall -Wextra -Werror -pedantic
-Wshadow -Wpointer-arith -Wcast-align
-Wmissing-prototypes -Wmissing-declarations
-Wconversion -Wstrict-prototypes
-m32 -ffreestanding -nostdlib
-fno-builtin -fno-stack-protector -fno-pic
```

## Development Constraints

- **No standard library**: Cannot use `<stdio.h>`, `<stdlib.h>`, `<string.h>`, etc.
- **Freestanding environment**: Only compiler-provided features available
- **32-bit x86**: Code targets i386 architecture
- **No dynamic memory**: No malloc/free (not yet implemented)
- **ISO size limit**: Final ISO must be under 10MB

## Adding New Features

1. Follow NASA rules (no recursion, bounded loops, ≤60 lines)
2. Use `k_*` prefix for library functions
3. Include detailed comments
4. Update relevant header files
5. Add source files to `Makefile` in `C_SRCS` or `ASM_SRCS`
6. Test with `make run` and `make run-iso`

## Toolchain Requirements

- **gcc**: Cross-compiler or native with `-m32` support
- **nasm**: Netwide Assembler for boot.asm
- **ld**: GNU linker
- **grub-mkrescue**: For creating bootable ISOs (requires `grub-pc-bin`, `xorriso`, `mtools`)
- **qemu-system-i386**: For testing (optional: KVM for acceleration)
