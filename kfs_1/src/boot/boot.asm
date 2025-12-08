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

    ; Set up the stack
    ; ESP must point to the top of our reserved stack space
    mov esp, stack_top

    ; Clear EBP for stack traces (optional but good practice)
    xor ebp, ebp

    ; Push multiboot info for potential future use
    ; These will be available as parameters to kernel_main if needed
    push ebx                            ; Multiboot info pointer
    push eax                            ; Multiboot magic number

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
