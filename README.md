# KFS - Kernel From Scratch

A minimal 32-bit x86 operating system kernel written in C and Assembly, designed to boot via GRUB and run on both real hardware and virtual machines. Built following NASA/JPL C Coding Standards.

## Features

### Core System
- **Multiboot Compliant**: Boots via GRUB bootloader
- **Protected Mode**: 32-bit protected mode with custom GDT
- **VGA Text Mode**: 80x25 display, 16 colors
- **Interrupt System**: Full IDT with CPU exceptions and hardware IRQs
- **PIC Management**: Remapped IRQs (0-15 to vectors 32-47)

### Input/Output
- **PS/2 Keyboard**: Full scancode handling, modifier keys (Shift, Ctrl, Alt, Caps Lock)
- **PS/2 Mouse**: Scroll wheel support with IntelliMouse detection
- **VGA Driver**: Hardware cursor, color attributes, special characters

### Terminal System
- **4 Virtual Terminals**: Independent TTYs (Alt+F1-F4 to switch)
- **200-line Scrollback**: Per-terminal history buffer
- **Mouse Scroll Navigation**: Scroll wheel to view history

## Project Structure

```
kfs_1/
├── Makefile                    # Build system (NASA-compliant flags)
├── linker.ld                   # Memory layout (kernel at 1MB)
├── iso/boot/grub/grub.cfg      # GRUB configuration
└── src/
    ├── boot/
    │   ├── boot.asm            # Entry point, GDT, stack setup
    │   └── interrupts.asm      # ISR/IRQ assembly stubs
    ├── kernel/
    │   ├── kernel.c            # Main entry, printk, panic
    │   ├── kernel.h            # Kernel constants
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
        ├── idt.h               # IDT structures
        ├── pic.h               # PIC constants
        ├── keyboard.h          # Keyboard interface
        ├── mouse.h             # Mouse interface
        └── vtty.h              # Terminal interface
```

## Building

### Prerequisites
- GCC with 32-bit support (`gcc -m32`)
- NASM assembler
- GNU LD linker
- grub-mkrescue (requires `grub-pc-bin`, `xorriso`, `mtools`)
- QEMU (`qemu-system-i386`)

### Commands
```bash
cd kfs_1
make all        # Build kernel
make iso        # Create bootable ISO
make clean      # Remove objects
make fclean     # Remove all generated files
make re         # Rebuild
```

## Running

```bash
make run            # QEMU direct boot
make run-kvm        # QEMU with KVM
make run-iso        # Boot ISO in QEMU
make run-iso-kvm    # Boot ISO with KVM
make debug          # GDB server on port 1234
```

### VirtualBox
1. Create VM (Type: Other, Version: Other/Unknown)
2. Attach `kfs_1.iso` as CD/DVD
3. Boot

## Controls

### Keyboard
| Key | Action |
|-----|--------|
| Printable keys | Echo to screen |
| Enter | New line |
| Backspace | Delete character |
| Tab | Insert 4 spaces |
| Alt+F1-F4 | Switch terminal |

### Mouse
| Action | Effect |
|--------|--------|
| Scroll Up | View older content |
| Scroll Down | View newer content |

## Technical Details

### Memory Map
| Address | Content |
|---------|---------|
| 0x00000-0xFFFFF | Reserved (BIOS, VGA at 0xB8000) |
| 0x100000+ | Kernel code and data |

### GDT (Custom)
| Selector | Segment |
|----------|---------|
| 0x00 | Null |
| 0x08 | Code (ring 0, 4GB) |
| 0x10 | Data (ring 0, 4GB) |

### IDT
| Vector | Handler |
|--------|---------|
| 0-31 | CPU exceptions |
| 32 | Timer (IRQ0) |
| 33 | Keyboard (IRQ1) |
| 44 | Mouse (IRQ12) |
| 32-47 | Hardware IRQs |
| 48-255 | Default handler |

### Compiler Flags
```
-Wall -Wextra -Werror -pedantic
-Wshadow -Wpointer-arith -Wcast-align
-Wmissing-prototypes -Wmissing-declarations
-Wconversion -Wstrict-prototypes
-m32 -ffreestanding -nostdlib
-fno-builtin -fno-stack-protector -fno-pic
```

## Coding Standards

NASA/JPL C Coding Standards:
1. No recursion
2. Bounded loops
3. Functions ≤ 60 lines
4. Narrow variable scope
5. All returns checked
6. No dynamic memory

## Author

**rluiz** - 42 Student
