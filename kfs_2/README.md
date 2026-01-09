# KFS-2: GDT & Stack

A minimal 32-bit x86 operating system kernel featuring a Global Descriptor Table (GDT) at address 0x800 and comprehensive stack debugging tools. Built for the 42 School KFS (Kernel From Scratch) project series.

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
- [Running](#running)
- [Architecture](#architecture)
- [Concepts Explained](#concepts-explained)
  - [Global Descriptor Table (GDT)](#global-descriptor-table-gdt)
  - [Memory Segmentation](#memory-segmentation)
  - [The Stack](#the-stack)
  - [Interrupt Handling](#interrupt-handling)
  - [Boot Process](#boot-process)
- [Project Structure](#project-structure)
- [Shell Commands](#shell-commands)
- [Technical Details](#technical-details)

---

## Features

### Mandatory (KFS-2 Subject)
- **GDT at 0x800** - Global Descriptor Table with 7 segments
- **Human-readable stack dump** - View stack contents, registers, and call trace
- **Kernel/User segments** - Ring 0 and Ring 3 memory segments

### Bonus
- **Interactive shell** - Command-line interface for debugging
- **Multiple commands** - `stack`, `gdt`, `regs`, `reboot`, `halt`, etc.
- **Virtual terminals** - 4 independent terminals (Alt+F1-F4)
- **Mouse scroll** - Scroll through terminal history
- **PS/2 keyboard & mouse support**

---

## Requirements

### Build Tools (Linux/WSL)
```bash
# Ubuntu/Debian
sudo apt install build-essential gcc-multilib nasm grub-pc-bin grub-common xorriso mtools qemu-system-x86
```

### Compiler Flags (per subject)
- `-fno-builtin` - Don't use built-in functions
- `-fno-exceptions` - No C++ exceptions
- `-fno-stack-protector` - No stack canaries
- `-nostdlib` - No standard library
- `-nodefaultlibs` - No default libraries

---

## Building

```bash
cd kfs_2

# Build kernel binary
make all

# Create bootable ISO (< 10MB as required)
make iso

# Clean and rebuild
make re
```

---

## Running

### Using QEMU

```bash
# Run kernel directly
make run

# Boot from ISO (full GRUB boot)
make run-iso

# With KVM acceleration (Linux)
make run-kvm
```

### Windows with QEMU

```cmd
"C:\Program Files\qemu\qemu-system-i386.exe" -kernel build/kernel.bin -m 32M
```

---

## Architecture

```
Memory Layout
=============

0x00000000 +------------------+
           |     Reserved     |  (BIOS, IVT, etc.)
0x00000800 +------------------+
           |       GDT        |  <- Global Descriptor Table (56 bytes)
           +------------------+
           |     Reserved     |  (VGA memory at 0xB8000)
0x00100000 +------------------+
           |      Kernel      |  <- Kernel loaded here (1MB)
           |    .multiboot    |  <- Multiboot header (first 8KB)
           |      .text       |  <- Code
           |     .rodata      |  <- Read-only data
           |      .data       |  <- Initialized data
           |       .bss       |  <- Uninitialized data (includes stack)
           +------------------+
           |      Stack       |  <- 16KB kernel stack (grows down)
           +------------------+
```

---

## Concepts Explained

### Global Descriptor Table (GDT)

The GDT is a fundamental x86 data structure that defines **memory segments**. In protected mode, all memory accesses go through segment descriptors in the GDT.

#### What is a Segment?

A segment defines a **region of memory** with:
- **Base address** - Where the segment starts (0x00000000 for flat model)
- **Limit** - Size of the segment (0xFFFFF with 4KB granularity = 4GB)
- **Access rights** - Who can access it and how (read/write/execute)
- **Privilege level** - Ring 0 (kernel) or Ring 3 (user)

#### GDT Entry Structure (8 bytes)

```
 63        56 55  52 51  48 47        40 39        32
+------------+------+------+------------+------------+
|  Base 31:24| Flags|Limit |   Access   | Base 23:16 |
|            | G|DB |19:16 |  P|DPL|S|Type          |
+------------+------+------+------------+------------+

 31                      16 15                       0
+-------------------------+-------------------------+
|       Base 15:0         |       Limit 15:0        |
+-------------------------+-------------------------+
```

**Access Byte:**
```
  7   6   5   4   3   2   1   0
+---+---+---+---+---+---+---+---+
| P |  DPL  | S | E |DC |RW | A |
+---+---+---+---+---+---+---+---+
  |     |     |   |   |   |   |
  |     |     |   |   |   |   +-- Accessed (CPU sets this)
  |     |     |   |   |   +------ Read/Write permission
  |     |     |   |   +---------- Direction/Conforming
  |     |     |   +-------------- Executable (1=code, 0=data)
  |     |     +------------------ Descriptor type (1=code/data)
  |     +------------------------ Privilege level (0-3)
  +------------------------------ Present (must be 1)
```

#### Our GDT Layout (at 0x800)

| Index | Selector | Type         | Ring | Access | Description |
|-------|----------|--------------|------|--------|-------------|
| 0     | 0x00     | Null         | -    | 0x00   | Required null descriptor |
| 1     | 0x08     | Kernel Code  | 0    | 0x9A   | Execute/Read |
| 2     | 0x10     | Kernel Data  | 0    | 0x92   | Read/Write |
| 3     | 0x18     | Kernel Stack | 0    | 0x92   | Read/Write |
| 4     | 0x20     | User Code    | 3    | 0xFA   | Execute/Read |
| 5     | 0x28     | User Data    | 3    | 0xF2   | Read/Write |
| 6     | 0x30     | User Stack   | 3    | 0xF2   | Read/Write |

#### Loading the GDT

```asm
; GDT Pointer structure (6 bytes)
gdt_ptr:
    dw gdt_size - 1    ; Limit (size - 1)
    dd 0x00000800      ; Base address

; Load GDT and reload segments
lgdt [gdt_ptr]         ; Load GDT register
jmp 0x08:.reload       ; Far jump to reload CS
.reload:
    mov ax, 0x10       ; Kernel data selector
    mov ds, ax         ; Reload data segments
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
```

---

### Memory Segmentation

#### Flat Memory Model

We use a **flat memory model** where all segments:
- Start at base address **0x00000000**
- Have limit **0xFFFFF** (with 4KB granularity = **4GB**)
- Overlap completely

This effectively gives us a flat 4GB address space while still satisfying x86 segmentation requirements.

#### Why Segmentation?

Intel x86 processors require segmentation in protected mode. Every memory access implicitly uses a segment:
- **CS (Code Segment)** - For instruction fetches
- **DS (Data Segment)** - For data access
- **SS (Stack Segment)** - For stack operations
- **ES, FS, GS** - Extra data segments

#### Segment Selectors

A selector is a 16-bit value loaded into segment registers:

```
 15                  3   2   1   0
+---------------------+---+-------+
|       Index         |TI |  RPL  |
+---------------------+---+-------+
        |              |      |
        |              |      +-- Requested Privilege Level
        |              +--------- Table Indicator (0=GDT, 1=LDT)
        +------------------------ Index into GDT/LDT
```

Example: Selector `0x08` = Index 1, GDT, Ring 0 (Kernel Code)

---

### The Stack

The stack is a **LIFO (Last In, First Out)** data structure used for:
- Function call return addresses
- Local variables
- Saved registers
- Function parameters (cdecl calling convention)

#### x86 Stack Behavior

The x86 stack grows **downward** (from high addresses to low):
- **PUSH** decrements ESP, then writes value
- **POP** reads value, then increments ESP
- **ESP** (Stack Pointer) always points to the top of the stack
- **EBP** (Base Pointer) marks the current stack frame

#### Stack Frame Layout (cdecl)

```
    Higher Addresses
    +------------------+
    |   Argument N     |  [EBP + 8 + (N-1)*4]
    +------------------+
    |   Argument 2     |  [EBP + 12]
    +------------------+
    |   Argument 1     |  [EBP + 8]
    +------------------+
    |  Return Address  |  [EBP + 4]  <- Where to return after call
    +------------------+
    |    Saved EBP     |  [EBP + 0]  <- Previous frame pointer
    +------------------+  <-------- EBP points here
    |   Local Var 1    |  [EBP - 4]
    +------------------+
    |   Local Var 2    |  [EBP - 8]
    +------------------+
    |       ...        |
    +------------------+  <-------- ESP points here
    Lower Addresses
```

#### Stack Trace (Backtrace)

To walk the call stack, we follow the **EBP chain**:

```c
typedef struct s_stack_frame {
    struct s_stack_frame *ebp;  // Previous frame's EBP
    uint32_t              eip;  // Return address
} t_stack_frame;

void stack_trace(void) {
    t_stack_frame *frame = (t_stack_frame *)get_ebp();
    while (frame != NULL) {
        print("EIP: %x, EBP: %x\n", frame->eip, frame);
        frame = frame->ebp;  // Walk to previous frame
    }
}
```

#### Our Stack Configuration

- **Location**: End of BSS section
- **Size**: 16KB (16384 bytes)
- **Segment**: Kernel Stack (selector 0x18)

---

### Interrupt Handling

#### Interrupt Descriptor Table (IDT)

The IDT maps interrupt/exception vectors to handler functions:
- Vectors 0-31: CPU exceptions (divide error, page fault, etc.)
- Vectors 32-47: Hardware IRQs (remapped from 0-15)
- Vector 33 (IRQ1): Keyboard
- Vector 44 (IRQ12): Mouse

#### Programmable Interrupt Controller (PIC)

The 8259 PIC manages hardware interrupts:

```
          +-------+          +-------+
IRQ0-7 -->| Master|--IRQ2--->| Slave |<-- IRQ8-15
          |  PIC  |          |  PIC  |
          +-------+          +-------+
              |
              v
             CPU
```

We remap IRQs to vectors 32-47 to avoid conflicts with CPU exceptions.

---

### Boot Process

1. **BIOS** loads GRUB from disk
2. **GRUB** finds our kernel (multiboot header in first 8KB)
3. **GRUB** loads kernel at 1MB, jumps to `_start`
4. **boot.asm** (`_start`):
   - Loads initial GDT
   - Sets up 16KB stack
   - Calls `kernel_main()`
5. **kernel.c** (`kernel_main`):
   - Initializes VGA driver
   - Sets up GDT at 0x800 via `gdt_init()`
   - Initializes PIC, IDT
   - Initializes keyboard, mouse
   - Starts shell

```
BIOS -> GRUB -> _start (ASM) -> kernel_main (C) -> shell_run
                   |
                   +-> Load GDT
                   +-> Setup Stack
                   +-> Enable Interrupts
```

---

## Project Structure

```
kfs_2/
├── Makefile                 # Build system
├── linker.ld                # Memory layout script
├── iso/boot/grub/grub.cfg   # GRUB configuration
└── src/
    ├── boot/
    │   ├── boot.asm         # Entry point, GDT flush, stack setup
    │   └── interrupts.asm   # ISR/IRQ assembly stubs
    ├── kernel/
    │   ├── kernel.c         # Main entry, printk
    │   ├── gdt.c            # GDT initialization at 0x800
    │   ├── idt.c            # Interrupt Descriptor Table
    │   ├── pic.c            # 8259 PIC driver
    │   ├── isr.c            # Interrupt handlers
    │   ├── stack.c          # Stack inspection tools
    │   ├── shell.c          # Interactive shell (bonus)
    │   └── vtty.c           # Virtual terminal system
    ├── drivers/
    │   ├── vga.c            # VGA text mode driver
    │   ├── keyboard.c       # PS/2 keyboard driver
    │   └── mouse.c          # PS/2 mouse driver
    ├── lib/
    │   └── string.c         # String utilities (k_memset, etc.)
    └── include/
        ├── types.h          # uint8_t, uint32_t, etc.
        ├── gdt.h            # GDT structures and constants
        ├── idt.h            # IDT structures
        ├── pic.h            # PIC constants
        ├── stack.h          # Stack frame structures
        ├── shell.h          # Shell interface
        ├── keyboard.h       # Keyboard interface
        ├── mouse.h          # Mouse interface
        └── vtty.h           # Virtual terminal interface
```

---

## Shell Commands

| Command  | Description |
|----------|-------------|
| `help`   | Display available commands |
| `stack`  | Print kernel stack dump (registers, trace, memory) |
| `gdt`    | Display GDT entries at 0x800 |
| `regs`   | Display CPU registers |
| `clear`  | Clear the screen |
| `info`   | Display kernel information |
| `reboot` | Reboot the system |
| `halt`   | Halt the CPU |

### Keyboard Shortcuts

| Keys | Action |
|------|--------|
| Alt+F1-F4 | Switch virtual terminals |
| Mouse Scroll | Scroll terminal history |

---

## Technical Details

### Compiler Flags

```makefile
KERNEL_FLAGS := -m32 -ffreestanding -fno-builtin -fno-exceptions \
                -fno-stack-protector -nostdlib -nodefaultlibs \
                -fno-pic -fno-pie -mno-red-zone

NASA_FLAGS := -Wall -Wextra -Werror -pedantic -Wshadow \
              -Wpointer-arith -Wcast-align -Wmissing-prototypes \
              -Wstrict-prototypes -Wconversion
```

### Memory Map

| Address | Size | Content |
|---------|------|---------|
| 0x00000800 | 56B | GDT (7 entries x 8 bytes) |
| 0x000B8000 | 4000B | VGA text buffer |
| 0x00100000 | ~20KB | Kernel code and data |
| Stack top | 16KB | Kernel stack |

### ISO Size

As required by the subject, the ISO is under 10MB:
- Kernel binary: ~20KB
- ISO image: ~1.5MB

---

## References

- [OSDev Wiki - GDT](https://wiki.osdev.org/GDT)
- [OSDev Wiki - Segmentation](https://wiki.osdev.org/Segmentation)
- [Intel Software Developer Manual](https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html)
- [Multiboot Specification](https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)

---

## License

This project is part of the 42 School curriculum.
