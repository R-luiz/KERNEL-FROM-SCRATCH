; **************************************************************************** ;
;                                                                              ;
;   KFS_1 - Kernel From Scratch                                                ;
;                                                                              ;
;   boot.asm - Multiboot Header and Entry Point                                ;
;                                                                              ;
;   This file contains:                                                        ;
;   - Multiboot header (for GRUB compatibility)                                ;
;   - Stack initialization                                                     ;
;   - Jump to kernel_main (C code)                                             ;
;                                                                              ;
; **************************************************************************** ;

; =============================================================================
; Multiboot Header Constants
; =============================================================================
; See: https://www.gnu.org/software/grub/manual/multiboot/multiboot.html

MBOOT_PAGE_ALIGN    equ 1 << 0          ; Align loaded modules on page boundaries
MBOOT_MEM_INFO      equ 1 << 1          ; Provide memory map
MBOOT_HEADER_MAGIC  equ 0x1BADB002      ; Magic number for bootloader
MBOOT_HEADER_FLAGS  equ MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO
MBOOT_CHECKSUM      equ -(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; =============================================================================
; Stack Configuration
; =============================================================================

STACK_SIZE          equ 16384           ; 16 KB stack size

; =============================================================================
; Multiboot Header Section
; =============================================================================
; This MUST be in the first 8KB of the kernel binary
; GRUB scans for this signature to identify valid kernels

section .multiboot
align 4
    dd MBOOT_HEADER_MAGIC               ; Magic number
    dd MBOOT_HEADER_FLAGS               ; Flags
    dd MBOOT_CHECKSUM                   ; Checksum (magic + flags + checksum = 0)

; =============================================================================
; BSS Section - Uninitialized Data
; =============================================================================
; Stack grows downward, so stack_top is at the end of reserved space

section .bss
align 16
stack_bottom:
    resb STACK_SIZE                     ; Reserve 16KB for stack
stack_top:

; =============================================================================
; GDT - Global Descriptor Table
; =============================================================================
; We define our own GDT to ensure consistent segment selectors
; regardless of boot method (GRUB or direct QEMU)
;
; Layout:
;   0x00: Null descriptor (required)
;   0x08: Code segment (ring 0, execute/read, 4GB flat)
;   0x10: Data segment (ring 0, read/write, 4GB flat)

section .rodata
align 16
gdt_start:
    ; Null descriptor (entry 0)
    dq 0x0000000000000000

    ; Code segment descriptor (entry 1, selector 0x08)
    ; Base=0, Limit=0xFFFFF, Access=0x9A (present, ring0, code, exec/read)
    ; Flags=0xC (4KB granularity, 32-bit)
    dw 0xFFFF       ; Limit bits 0-15
    dw 0x0000       ; Base bits 0-15
    db 0x00         ; Base bits 16-23
    db 0x9A         ; Access: present, ring0, code segment, exec/read
    db 0xCF         ; Flags (4KB gran, 32-bit) + Limit bits 16-19
    db 0x00         ; Base bits 24-31

    ; Data segment descriptor (entry 2, selector 0x10)
    ; Base=0, Limit=0xFFFFF, Access=0x92 (present, ring0, data, read/write)
    ; Flags=0xC (4KB granularity, 32-bit)
    dw 0xFFFF       ; Limit bits 0-15
    dw 0x0000       ; Base bits 0-15
    db 0x00         ; Base bits 16-23
    db 0x92         ; Access: present, ring0, data segment, read/write
    db 0xCF         ; Flags (4KB gran, 32-bit) + Limit bits 16-19
    db 0x00         ; Base bits 24-31
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1  ; GDT size (limit)
    dd gdt_start                ; GDT base address

; Segment selector constants
CODE_SEG equ 0x08
DATA_SEG equ 0x10

; =============================================================================
; Text Section - Executable Code
; =============================================================================

section .text
global _start                           ; Export entry point for linker
extern kernel_main                      ; Import C function

; -----------------------------------------------------------------------------
; _start - Kernel Entry Point
; -----------------------------------------------------------------------------
; Called by GRUB after loading the kernel
; At this point:
;   - CPU is in 32-bit protected mode
;   - Interrupts are disabled
;   - Paging is disabled
;   - EAX contains multiboot magic number (0x2BADB002)
;   - EBX contains pointer to multiboot information structure
; -----------------------------------------------------------------------------

_start:
    ; Disable interrupts (should already be disabled, but be safe)
    cli

    ; Save multiboot parameters before we clobber registers
    mov edi, eax                        ; Save magic number
    mov esi, ebx                        ; Save multiboot info pointer

    ; Load our own GDT to ensure consistent segment selectors
    lgdt [gdt_descriptor]

    ; Far jump to reload CS with our code segment selector
    jmp CODE_SEG:.reload_segments

.reload_segments:
    ; Reload data segment registers with our data segment selector
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set up the stack
    ; ESP must point to the top of our reserved stack space
    mov esp, stack_top

    ; Clear EBP for stack traces (optional but good practice)
    xor ebp, ebp

    ; Push multiboot info for potential future use
    ; These will be available as parameters to kernel_main if needed
    push esi                            ; Multiboot info pointer
    push edi                            ; Multiboot magic number

    ; Call the kernel main function
    call kernel_main

    ; If kernel_main returns (it shouldn't), halt the CPU
    ; This is a safety measure
.hang:
    cli                                 ; Disable interrupts
    hlt                                 ; Halt the CPU
    jmp .hang                           ; If NMI wakes us, halt again

; =============================================================================
; End of boot.asm
; =============================================================================
