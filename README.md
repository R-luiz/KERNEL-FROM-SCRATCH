# KFS - Kernel From Scratch

A minimal 32-bit x86 operating system kernel written in C and Assembly, designed to boot via GRUB and run on both real hardware and virtual machines.

## Features

### Core Features
- **Multiboot Compliant**: Boots via GRUB bootloader
- **Protected Mode**: Runs in 32-bit protected mode with custom GDT
- **VGA Text Mode**: 80x25 character display with 16 foreground and 8 background colors
- **Interrupt Handling**: Full IDT setup with CPU exception and hardware IRQ handlers
- **PIC Management**: Programmable Interrupt Controller initialization and EOI handling

### Bonus Features
- **PS/2 Keyboard Driver**: Full keyboard input with modifier key support (Shift, Ctrl, Alt, Caps Lock)
- **Virtual Terminals**: 4 independent TTYs switchable with Alt+F1 through Alt+F4
- **printk()**: Printf-like formatted output for kernel debugging
- **Hardware Cursor**: Blinking cursor with proper positioning
- **Scrolling**: Automatic screen scrolling when buffer is full

## Project Structure

```
kfs_1/
├── Makefile                 # Build system with NASA-compliant flags
├── linker.ld               # Kernel memory layout (loads at 1MB)
├── iso/
│   └── boot/grub/
│       └── grub.cfg        # GRUB bootloader configuration
└── src/
    ├── boot/
    │   ├── boot.asm        # Entry point, GDT setup, stack initialization
    │   └── interrupts.asm  # ISR/IRQ assembly stubs
    ├── kernel/
    │   ├── kernel.c        # Main entry, printk, kernel panic
    │   ├── kernel.h        # Kernel constants and prototypes
    │   ├── idt.c           # Interrupt Descriptor Table setup
    │   ├── pic.c           # PIC initialization
    │   ├── isr.c           # Interrupt service routines
    │   └── vtty.c          # Virtual terminal implementation
    ├── drivers/
    │   ├── vga.c           # VGA text mode driver
    │   ├── vga.h           # VGA interface
    │   └── keyboard.c      # PS/2 keyboard driver
    ├── lib/
    │   ├── string.c        # Memory/string utilities (k_memset, k_strlen, etc.)
    │   └── string.h        # String library interface
    └── include/
        ├── types.h         # Fixed-width types, compiler attributes
        ├── idt.h           # IDT structures and prototypes
        ├── pic.h           # PIC constants and prototypes
        ├── keyboard.h      # Keyboard interface
        └── vtty.h          # Virtual terminal interface
```

## Building

### Prerequisites

- **GCC**: With 32-bit support (`gcc -m32`)
- **NASM**: Netwide Assembler
- **GNU LD**: Linker with ELF support
- **grub-mkrescue**: For creating bootable ISOs (requires `grub-pc-bin`, `xorriso`, `mtools`)
- **QEMU**: For testing (`qemu-system-i386`)

### Build Commands

```bash
cd kfs_1

# Build kernel binary
make all

# Create bootable ISO
make iso

# Clean build artifacts
make clean

# Full clean (including ISO)
make fclean

# Rebuild from scratch
make re
```

## Running

### Direct Boot (QEMU)
```bash
make run          # Software emulation
make run-kvm      # With KVM acceleration
```

### ISO Boot (GRUB)
```bash
make run-iso      # Boot ISO in QEMU
make run-iso-kvm  # Boot ISO with KVM
```

### VirtualBox
1. Create a new VM (Type: Other, Version: Other/Unknown)
2. Attach the generated `kfs_1.iso` as a CD/DVD
3. Boot the VM

### Debugging
```bash
make debug        # Start QEMU with GDB server on port 1234

# In another terminal:
gdb -ex 'target remote :1234' build/kernel.bin
```

## Usage

Once booted, you'll see the kernel banner and a "42" ASCII art display.

### Keyboard Controls
| Key | Action |
|-----|--------|
| Any printable key | Echo character to screen |
| Enter | New line |
| Backspace | Delete previous character |
| Tab | Insert tab (4 spaces) |
| Alt+F1 | Switch to Terminal 1 |
| Alt+F2 | Switch to Terminal 2 |
| Alt+F3 | Switch to Terminal 3 |
| Alt+F4 | Switch to Terminal 4 |

### Virtual Terminals
The kernel provides 4 independent virtual terminals, each with:
- Separate screen buffer (80x25 characters)
- Independent cursor position
- Individual color settings

## Technical Details

### Memory Layout
| Address | Content |
|---------|---------|
| 0x00000000 - 0x000FFFFF | Reserved (BIOS, VGA at 0xB8000) |
| 0x00100000 onwards | Kernel code and data |

### GDT Layout
| Selector | Segment |
|----------|---------|
| 0x00 | Null descriptor |
| 0x08 | Code segment (ring 0, 4GB flat) |
| 0x10 | Data segment (ring 0, 4GB flat) |

### IDT Configuration
| Vector | Handler |
|--------|---------|
| 0-31 | CPU exceptions (divide error, page fault, etc.) |
| 32-47 | Hardware IRQs (remapped from PIC) |
| 48-255 | Default handler |

### Compiler Flags
The project uses strict NASA/JPL C Coding Standards flags:
- `-Wall -Wextra -Werror -pedantic`
- `-Wshadow -Wpointer-arith -Wcast-align`
- `-Wmissing-prototypes -Wmissing-declarations`
- `-Wconversion -Wstrict-prototypes`

Freestanding kernel flags:
- `-m32 -ffreestanding -nostdlib`
- `-fno-builtin -fno-stack-protector -fno-pic`

## Coding Standards

This project follows NASA/JPL C Coding Standards:

1. **No recursion**: All functions use iteration
2. **Bounded loops**: Every loop has a computable upper bound
3. **Function length**: Maximum 60 lines per function
4. **Narrow scope**: Variables declared at narrowest possible scope
5. **Return value checking**: All return values checked or explicitly ignored
6. **No dynamic memory**: No malloc/free (not implemented)

## Verification

```bash
# Analyze kernel binary
make check

# Verify ISO size (must be < 10MB)
make check-iso
```

## License

This project was created as part of the 42 school curriculum.

## Author

**rluiz** - 42 Student
