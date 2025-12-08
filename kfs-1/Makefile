# Makefile for KFS-1 Kernel

# Compiler and tools
CC = gcc
AS = as
LD = ld

# Flags
CFLAGS = -std=gnu99 -ffreestanding -O2 -Wall -Wextra -m32
LDFLAGS = -T linker.ld -nostdlib -melf_i386
ASFLAGS = --32

# Target architecture
TARGET = i686-elf

# Output
KERNEL = kfs-1.bin

# Source files
ASM_SOURCES = boot.s
C_SOURCES = kernel.c

# Object files
ASM_OBJECTS = $(ASM_SOURCES:.s=.o)
C_OBJECTS = $(C_SOURCES:.c=.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Default target
all: $(KERNEL)

# Link kernel
$(KERNEL): $(OBJECTS)
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

# Compile C sources
%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS)

# Assemble assembly sources
%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

# Clean build artifacts
clean:
	rm -f $(OBJECTS) $(KERNEL)

# Phony targets
.PHONY: all clean

# Help target
help:
	@echo "KFS-1 Kernel Build System"
	@echo "========================="
	@echo "Targets:"
	@echo "  all     - Build the kernel (default)"
	@echo "  clean   - Remove build artifacts"
	@echo "  help    - Show this help message"
