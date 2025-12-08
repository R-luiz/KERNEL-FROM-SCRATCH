# KERNEL-FROM-SCRATCH

A repository for learning kernel development from scratch, containing multiple sub-projects that progressively build kernel functionality.

## Overview

This repository is organized into multiple sub-projects, each focusing on different aspects of kernel development. All projects are written in C and use Makefiles for building with GCC.

## Projects

### KFS-1 - Basic Bootable Kernel
The first project implements a minimal bootable kernel with:
- Multiboot support
- VGA text mode output
- Basic terminal functions

See [kfs-1/README.md](kfs-1/README.md) for details.

### Future Projects
- **kfs-2** - Memory management
- **kfs-3** - Interrupt handling
- **kfs-4** - Process management
- And more...

## Prerequisites

To build and run these kernel projects, you'll need:
- GCC (GNU Compiler Collection)
- GNU Assembler (as)
- GNU Linker (ld)
- GNU Make
- QEMU (for testing kernels)

On Ubuntu/Debian:
```bash
sudo apt-get install build-essential qemu-system-x86
```

## Building

Each sub-project has its own Makefile. Navigate to the project directory and run:
```bash
cd kfs-1
make
```

## Running

You can test kernels using QEMU:
```bash
cd kfs-1
qemu-system-i386 -kernel kfs-1.bin
```

## Contributing

This is a learning project. Feel free to explore the code and experiment with kernel development concepts.

## License

This project is for educational purposes.