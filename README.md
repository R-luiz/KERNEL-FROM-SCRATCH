# KFS - Kernel From Scratch

A minimal 32-bit x86 operating system kernel written in C and Assembly, designed to boot via GRUB and run on both real hardware and virtual machines. Built following NASA/JPL C Coding Standards.

## Projects

### KFS-1: Basic Kernel
First iteration with VGA output, keyboard/mouse input, and virtual terminals.

### KFS-2: GDT & Shell (Current)
Extended with proper GDT implementation, interactive shell, stack inspection, and CPU control commands.

## Features

### Core System
- **Multiboot Compliant**: Boots via GRUB bootloader
- **Protected Mode**: 32-bit protected mode with custom GDT
- **GDT at 0x800**: Properly structured Global Descriptor Table with null, code, data, and stack segments
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
- **Interactive Shell**: Command interpreter with built-in commands

### Shell Commands (KFS-2)
| Command | Description |
|---------|-------------|
| `help` | Show available commands |
| `stack` | Print kernel stack dump |
| `gdt` | Display GDT at 0x800 |
| `regs` | Display CPU registers |
| `clear` | Clear the screen |
| `info` | Show kernel info |
| `reboot` | Reboot system |
| `halt` | Halt CPU |

## Project Structure

```
kfs_1/                          # Basic kernel (Phase 1)
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

kfs_2/                          # GDT & Shell (Phase 2)
├── Makefile
├── linker.ld
├── iso/boot/grub/grub.cfg
└── src/
    ├── boot/
    │   ├── boot.asm            # GDT at 0x800
    │   └── interrupts.asm
    ├── kernel/
    │   ├── kernel.c            # Main entry
    │   ├── gdt.c               # GDT display utilities
    │   ├── shell.c             # Interactive shell
    │   ├── stack.c             # Stack inspection
    │   ├── idt.c, pic.c, isr.c, vtty.c
    ├── drivers/
    │   ├── vga.c, keyboard.c, mouse.c
    ├── lib/
    │   └── string.c
    └── include/
        └── *.h (gdt.h, shell.h, stack.h, ...)
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
cd kfs_2  # or kfs_1
make all        # Build kernel
make iso        # Create bootable ISO
make clean      # Remove objects
make fclean     # Remove all generated files
make re         # Rebuild
```

## Running

### Option 1: Using WSL (Recommended for Windows)

```powershell
# Build the kernel
wsl -d Ubuntu -e bash -c "cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2' && make all"

# Run in QEMU (Windows QEMU)
& "C:\Program Files\qemu\qemu-system-i386.exe" -kernel "C:\Users\luizr\Documents\Nouveau dossier\KERNEL-FROM-SCRATCH\kfs_2\build\kernel.bin" -m 32M

# Or create and boot from ISO
wsl -d Ubuntu -e bash -c "cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2' && make iso"
& "C:\Program Files\qemu\qemu-system-i386.exe" -cdrom "C:\Users\luizr\Documents\Nouveau dossier\KERNEL-FROM-SCRATCH\kfs_2\kfs_2.iso" -m 32M
```

### Option 2: Fully Inside WSL Ubuntu

```bash
# Open WSL Ubuntu terminal
wsl -d Ubuntu

# Navigate to project
cd '/mnt/c/Users/luizr/Documents/Nouveau dossier/KERNEL-FROM-SCRATCH/kfs_2'

# Build
make clean && make all

# Run directly (requires X display - WSLg)
export DISPLAY=:0
make run

# Or boot from ISO
make iso
make run-iso
```

### Option 3: Native Linux

```bash
make run            # QEMU direct boot
make run-kvm        # QEMU with KVM
make run-iso        # Boot ISO in QEMU
make run-iso-kvm    # Boot ISO with KVM
make debug          # GDB server on port 1234
```

### VirtualBox
1. Create VM (Type: Other, Version: Other/Unknown)
2. Attach `kfs_2.iso` as CD/DVD
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

### GDT (at 0x800)
| Selector | Segment |
|----------|---------|
| 0x00 | Null |
| 0x08 | Kernel Code (ring 0, 4GB) |
| 0x10 | Kernel Data (ring 0, 4GB) |
| 0x18 | Kernel Stack (ring 0, 4GB) |
| 0x20 | User Code (ring 3, 4GB) |
| 0x28 | User Data (ring 3, 4GB) |
| 0x30 | User Stack (ring 3, 4GB) |

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
