# KFS-1 - First Kernel Project

This is the first project in the Kernel From Scratch series. It implements a minimal bootable kernel that can display text on the screen.

## Features

- Multiboot-compliant bootloader support
- VGA text mode display
- Basic terminal output functions
- Minimal C kernel implementation

## Building

To build the kernel, you need:
- GCC (or compatible C compiler)
- GNU Assembler (as)
- GNU Linker (ld)
- GNU Make

Build the kernel with:
```bash
make
```

This will produce `kfs-1.bin`, a bootable kernel binary.

## Running

You can run the kernel using QEMU:
```bash
qemu-system-i386 -kernel kfs-1.bin
```

Or you can use it with GRUB on a bootable USB/CD.

## Files

- `boot.s` - Bootstrap assembly code and Multiboot header
- `kernel.c` - Main kernel code with VGA text output
- `linker.ld` - Linker script for kernel memory layout
- `Makefile` - Build system

## Cleaning

To remove build artifacts:
```bash
make clean
```
