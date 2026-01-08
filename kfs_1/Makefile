# **************************************************************************** #
#                                                                              #
#   KFS_1 - Kernel From Scratch                                                #
#                                                                              #
#   Makefile - Build System                                                    #
#                                                                              #
#   NASA/JPL C Coding Standards: Strict compilation flags                      #
#                                                                              #
# **************************************************************************** #

# =============================================================================
# Project Configuration
# =============================================================================

NAME            := kernel.bin
ISO             := kfs_1.iso

# Directories
SRC_DIR         := src
BUILD_DIR       := build
ISO_DIR         := iso

# =============================================================================
# Toolchain Configuration
# =============================================================================

# Compilers
CC              := gcc
AS              := nasm
LD              := ld

# Target architecture
ARCH            := i386

# =============================================================================
# Compiler Flags
# =============================================================================

# Common flags for freestanding kernel (required by subject)
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

# NASA/JPL Strict Warning Flags
NASA_FLAGS      := -Wall \
                   -Wextra \
                   -Werror \
                   -pedantic \
                   -Wshadow \
                   -Wpointer-arith \
                   -Wcast-align \
                   -Wwrite-strings \
                   -Wmissing-prototypes \
                   -Wmissing-declarations \
                   -Wredundant-decls \
                   -Wnested-externs \
                   -Winline \
                   -Wno-long-long \
                   -Wstrict-prototypes \
                   -Wconversion \
                   -Wundef

# Include paths
INCLUDES        := -I$(SRC_DIR)/include \
                   -I$(SRC_DIR)/kernel \
                   -I$(SRC_DIR)/drivers \
                   -I$(SRC_DIR)/lib

# Combined C flags (no debug symbols for smaller binary)
CFLAGS          := $(KERNEL_FLAGS) $(NASA_FLAGS) $(INCLUDES) -std=c99 -Os

# Assembler flags
ASFLAGS         := -f elf32

# Linker flags (strip all symbols and sections)
LDFLAGS         := -m elf_i386 -T linker.ld -nostdlib -s

# =============================================================================
# Source Files
# =============================================================================

# Assembly sources
ASM_SRCS        := $(SRC_DIR)/boot/boot.asm \
                   $(SRC_DIR)/boot/interrupts.asm

# C sources
C_SRCS          := $(SRC_DIR)/kernel/kernel.c \
                   $(SRC_DIR)/kernel/idt.c \
                   $(SRC_DIR)/kernel/pic.c \
                   $(SRC_DIR)/kernel/isr.c \
                   $(SRC_DIR)/kernel/vtty.c \
                   $(SRC_DIR)/drivers/vga.c \
                   $(SRC_DIR)/drivers/keyboard.c \
                   $(SRC_DIR)/drivers/mouse.c \
                   $(SRC_DIR)/lib/string.c

# Object files
ASM_OBJS        := $(patsubst $(SRC_DIR)/%.asm,$(BUILD_DIR)/%.o,$(ASM_SRCS))
C_OBJS          := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(C_SRCS))
OBJS            := $(ASM_OBJS) $(C_OBJS)

# =============================================================================
# Build Rules
# =============================================================================

.PHONY: all clean fclean re iso run run-kvm debug check

# Default target
all: $(BUILD_DIR)/$(NAME)

# Link the kernel
$(BUILD_DIR)/$(NAME): $(OBJS) linker.ld
	@echo "  LD      $@"
	@$(LD) $(LDFLAGS) -o $@ $(OBJS)
	@echo "  SIZE    $@"
	@size $@

# Compile assembly files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	@echo "  AS      $<"
	@$(AS) $(ASFLAGS) -o $@ $<

# Compile C files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	@echo "  CC      $<"
	@$(CC) $(CFLAGS) -c -o $@ $<

# =============================================================================
# ISO Image Creation
# =============================================================================

iso: $(ISO)

$(ISO): $(BUILD_DIR)/$(NAME)
	@echo "  ISO     $@"
	@mkdir -p $(ISO_DIR)/boot/grub
	@cp $(BUILD_DIR)/$(NAME) $(ISO_DIR)/boot/
	@if command -v grub-mkrescue >/dev/null 2>&1; then \
		grub-mkrescue --directory=/usr/lib/grub/i386-pc --compress=xz --modules="multiboot" --install-modules="multiboot normal biosdisk part_msdos iso9660" -o $@ $(ISO_DIR) 2>&1 | grep -v "^xorriso" || true; \
		if [ ! -f $@ ]; then \
			echo "  WARN    grub-mkrescue failed. Ensure grub-pc-bin and mtools are installed."; \
			exit 1; \
		fi; \
	else \
		echo "  ERROR   grub-mkrescue not found. Install grub-pc-bin or use 'make run' instead."; \
		exit 1; \
	fi
	@echo "  SIZE    $@"
	@du -h $@

# =============================================================================
# Run Targets
# =============================================================================

# Run with QEMU (software emulation)
run: $(BUILD_DIR)/$(NAME)
	@echo "  QEMU    Running kernel..."
	@qemu-system-i386 -kernel $(BUILD_DIR)/$(NAME) -m 32M

# Run with QEMU + KVM (hardware acceleration)
run-kvm: $(BUILD_DIR)/$(NAME)
	@echo "  KVM     Running kernel..."
	@qemu-system-i386 -enable-kvm -kernel $(BUILD_DIR)/$(NAME) -m 32M

# Run from ISO (full GRUB boot)
run-iso: $(ISO)
	@echo "  QEMU    Booting ISO..."
	@qemu-system-i386 -cdrom $(ISO) -m 32M

# Run from ISO with KVM
run-iso-kvm: $(ISO)
	@echo "  KVM     Booting ISO..."
	@qemu-system-i386 -enable-kvm -cdrom $(ISO) -m 32M

# Debug with GDB
debug: $(BUILD_DIR)/$(NAME)
	@echo "  DEBUG   Starting QEMU with GDB server..."
	@echo "  INFO    Connect with: gdb -ex 'target remote :1234' $(BUILD_DIR)/$(NAME)"
	@qemu-system-i386 -kernel $(BUILD_DIR)/$(NAME) -m 32M -s -S

# Debug with KVM
debug-kvm: $(BUILD_DIR)/$(NAME)
	@echo "  DEBUG   Starting QEMU+KVM with GDB server..."
	@qemu-system-i386 -enable-kvm -kernel $(BUILD_DIR)/$(NAME) -m 32M -s -S

# =============================================================================
# Verification Targets
# =============================================================================

# Check kernel binary
check: $(BUILD_DIR)/$(NAME)
	@echo "=== Kernel Binary Analysis ==="
	@echo ""
	@echo "File info:"
	@file $(BUILD_DIR)/$(NAME)
	@echo ""
	@echo "Size:"
	@ls -lh $(BUILD_DIR)/$(NAME)
	@echo ""
	@echo "Sections:"
	@objdump -h $(BUILD_DIR)/$(NAME)
	@echo ""
	@echo "Multiboot header check:"
	@if objdump -t $(BUILD_DIR)/$(NAME) | grep -q "multiboot"; then \
		echo "  [OK] Multiboot section found"; \
	else \
		echo "  [FAIL] Multiboot section not found!"; \
	fi
	@echo ""
	@echo "Entry point:"
	@objdump -f $(BUILD_DIR)/$(NAME) | grep "start address"

# Verify ISO size (must be < 10MB)
check-iso: $(ISO)
	@echo "=== ISO Size Check ==="
	@SIZE=$$(du -b $(ISO) | cut -f1); \
	MAX=10485760; \
	echo "  ISO size: $$SIZE bytes"; \
	echo "  Maximum:  $$MAX bytes (10MB)"; \
	if [ $$SIZE -lt $$MAX ]; then \
		echo "  [OK] ISO is under 10MB"; \
	else \
		echo "  [FAIL] ISO exceeds 10MB limit!"; \
		exit 1; \
	fi

# =============================================================================
# Clean Targets
# =============================================================================

clean:
	@echo "  CLEAN   Removing object files..."
	@rm -rf $(BUILD_DIR)

fclean: clean
	@echo "  FCLEAN  Removing kernel and ISO..."
	@rm -f $(ISO)
	@rm -rf $(ISO_DIR)/boot/kernel.bin

re: fclean all

# =============================================================================
# Help
# =============================================================================

help:
	@echo "KFS_1 - Kernel From Scratch"
	@echo ""
	@echo "Build targets:"
	@echo "  all          - Build the kernel binary"
	@echo "  iso          - Create bootable ISO image"
	@echo "  clean        - Remove object files"
	@echo "  fclean       - Remove all generated files"
	@echo "  re           - Rebuild everything"
	@echo ""
	@echo "Run targets:"
	@echo "  run          - Run kernel in QEMU"
	@echo "  run-kvm      - Run kernel in QEMU with KVM"
	@echo "  run-iso      - Boot ISO in QEMU"
	@echo "  run-iso-kvm  - Boot ISO in QEMU with KVM"
	@echo ""
	@echo "Debug targets:"
	@echo "  debug        - Run with GDB server (port 1234)"
	@echo "  debug-kvm    - Run with GDB server + KVM"
	@echo ""
	@echo "Verification:"
	@echo "  check        - Analyze kernel binary"
	@echo "  check-iso    - Verify ISO size"
